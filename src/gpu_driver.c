/**
 * @file gpu_driver.c
 * @brief Implementation of high-level GPU driver and dynamic NVML runtime bridge.
 *
 * Adheres strictly to BARR-C:2018 coding standard:
 * - Explicit fixed-width integer types
 * - Defensive parameter and pointer validation
 * - Compound statement braces around all control blocks
 * - Static module variables prefixed with s_
 * - Pointers prefixed with p_
 */

#include "gpu_driver.h"
#include "gpu_mock.h"
#include "gpu_nvml_types.h"

#include <dlfcn.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** NVML shared library candidate paths. */
static const char * const s_nvml_lib_candidates[] = {
    "libnvidia-ml.so.1",
    "libnvidia-ml.so",
    "/usr/lib/x86_64-linux-gnu/libnvidia-ml.so.1",
    "/usr/lib/libnvidia-ml.so.1",
    "/usr/lib64/libnvidia-ml.so.1"
};

/** Number of candidate library paths to try. */
#define NVML_CANDIDATE_COUNT (sizeof(s_nvml_lib_candidates) / sizeof(s_nvml_lib_candidates[0]))

/** Global driver state. */
static bool s_is_initialized = false;
static gpu_backend_t s_active_backend = GPU_BACKEND_AUTO;
static gpu_nvml_table_t s_nvml = {0};

/* ========================================================================= */
/* Private Helper Declarations                                               */
/* ========================================================================= */

static gpu_status_t load_nvml_symbols(void * const p_handle, gpu_nvml_table_t * const p_tbl);
static gpu_status_t init_nvml_backend(void);
static void shutdown_nvml_backend(void);

/* ========================================================================= */
/* NVML Dynamic Loader Implementation                                        */
/* ========================================================================= */

static gpu_status_t load_nvml_symbols(void * const p_handle, gpu_nvml_table_t * const p_tbl)
{
    if ((p_handle == NULL) || (p_tbl == NULL))
    {
        return GPU_STATUS_ERROR_NULL_POINTER;
    }

    memset(p_tbl, 0, sizeof(*p_tbl));
    p_tbl->p_lib_handle = p_handle;

    /* Helper macro to copy symbol address without violating ISO C void* cast rules */
#define LOAD_NVML_SYM(handle, target, sym_name) do { \
        void *p_sym_addr = dlsym((handle), (sym_name)); \
        (void)memcpy(&(target), &p_sym_addr, sizeof(target)); \
    } while (0)

    /* Load init and shutdown */
    LOAD_NVML_SYM(p_handle, p_tbl->init_v2, "nvmlInit_v2");
    LOAD_NVML_SYM(p_handle, p_tbl->init, "nvmlInit");
    LOAD_NVML_SYM(p_handle, p_tbl->shutdown, "nvmlShutdown");

    if (((p_tbl->init_v2 == NULL) && (p_tbl->init == NULL)) || (p_tbl->shutdown == NULL))
    {
        return GPU_STATUS_ERROR_DRIVER_NOT_FOUND;
    }

    /* Load device enumeration */
    LOAD_NVML_SYM(p_handle, p_tbl->get_count_v2, "nvmlDeviceGetCount_v2");
    LOAD_NVML_SYM(p_handle, p_tbl->get_count, "nvmlDeviceGetCount");
    LOAD_NVML_SYM(p_handle, p_tbl->get_handle_v2, "nvmlDeviceGetHandleByIndex_v2");
    LOAD_NVML_SYM(p_handle, p_tbl->get_handle, "nvmlDeviceGetHandleByIndex");

    if (((p_tbl->get_count_v2 == NULL) && (p_tbl->get_count == NULL)) ||
        ((p_tbl->get_handle_v2 == NULL) && (p_tbl->get_handle == NULL)))
    {
        return GPU_STATUS_ERROR_DRIVER_NOT_FOUND;
    }

    /* Load telemetry functions */
    LOAD_NVML_SYM(p_handle, p_tbl->get_name, "nvmlDeviceGetName");
    LOAD_NVML_SYM(p_handle, p_tbl->get_pci_info_v3, "nvmlDeviceGetPciInfo_v3");
    LOAD_NVML_SYM(p_handle, p_tbl->get_pci_info, "nvmlDeviceGetPciInfo");
    LOAD_NVML_SYM(p_handle, p_tbl->get_memory_info, "nvmlDeviceGetMemoryInfo");
    LOAD_NVML_SYM(p_handle, p_tbl->get_utilization, "nvmlDeviceGetUtilizationRates");
    LOAD_NVML_SYM(p_handle, p_tbl->get_temperature, "nvmlDeviceGetTemperature");
    LOAD_NVML_SYM(p_handle, p_tbl->get_clock_info, "nvmlDeviceGetClockInfo");
    LOAD_NVML_SYM(p_handle, p_tbl->error_string, "nvmlErrorString");

#undef LOAD_NVML_SYM

    return GPU_STATUS_OK;
}

static gpu_status_t init_nvml_backend(void)
{
    void *p_lib = NULL;

    for (size_t i = 0U; i < NVML_CANDIDATE_COUNT; ++i)
    {
        p_lib = dlopen(s_nvml_lib_candidates[i], RTLD_NOW);
        if (p_lib != NULL)
        {
            break;
        }
    }

    if (p_lib == NULL)
    {
        return GPU_STATUS_ERROR_DRIVER_NOT_FOUND;
    }

    gpu_status_t status = load_nvml_symbols(p_lib, &s_nvml);
    if (status != GPU_STATUS_OK)
    {
        (void)dlclose(p_lib);
        memset(&s_nvml, 0, sizeof(s_nvml));
        return status;
    }

    nvmlReturn_t nvml_res;
    if (s_nvml.init_v2 != NULL)
    {
        nvml_res = s_nvml.init_v2();
    }
    else
    {
        nvml_res = s_nvml.init();
    }

    if (nvml_res != NVML_SUCCESS)
    {
        (void)dlclose(p_lib);
        memset(&s_nvml, 0, sizeof(s_nvml));
        return GPU_STATUS_ERROR_BACKEND_UNAVAILABLE;
    }

    return GPU_STATUS_OK;
}

static void shutdown_nvml_backend(void)
{
    if (s_nvml.shutdown != NULL)
    {
        (void)s_nvml.shutdown();
    }

    if (s_nvml.p_lib_handle != NULL)
    {
        (void)dlclose(s_nvml.p_lib_handle);
    }

    memset(&s_nvml, 0, sizeof(s_nvml));
}

/* ========================================================================= */
/* Public API Implementation                                                 */
/* ========================================================================= */

gpu_status_t gpu_driver_init(gpu_backend_t requested_backend)
{
    if (s_is_initialized)
    {
        return GPU_STATUS_ERROR_ALREADY_INITIALIZED;
    }

    if ((requested_backend != GPU_BACKEND_AUTO) &&
        (requested_backend != GPU_BACKEND_NVML) &&
        (requested_backend != GPU_BACKEND_MOCK))
    {
        return GPU_STATUS_ERROR_INVALID_PARAM;
    }

    if (requested_backend == GPU_BACKEND_MOCK)
    {
        gpu_status_t mock_status = gpu_mock_init();
        if (mock_status == GPU_STATUS_OK)
        {
            s_active_backend = GPU_BACKEND_MOCK;
            s_is_initialized = true;
        }
        return mock_status;
    }

    if (requested_backend == GPU_BACKEND_NVML)
    {
        gpu_status_t nvml_status = init_nvml_backend();
        if (nvml_status != GPU_STATUS_OK)
        {
            return nvml_status;
        }
        s_active_backend = GPU_BACKEND_NVML;
        s_is_initialized = true;
        return GPU_STATUS_OK;
    }

    /* AUTO Mode: Try NVML first, fallback to MOCK */
    gpu_status_t nvml_status = init_nvml_backend();
    if (nvml_status == GPU_STATUS_OK)
    {
        s_active_backend = GPU_BACKEND_NVML;
        s_is_initialized = true;
        return GPU_STATUS_OK;
    }

    /* Fallback to Mock */
    gpu_status_t mock_status = gpu_mock_init();
    if (mock_status == GPU_STATUS_OK)
    {
        s_active_backend = GPU_BACKEND_MOCK;
        s_is_initialized = true;
        return GPU_STATUS_OK;
    }

    return mock_status;
}

gpu_status_t gpu_driver_shutdown(void)
{
    if (!s_is_initialized)
    {
        return GPU_STATUS_ERROR_NOT_INITIALIZED;
    }

    gpu_status_t result = GPU_STATUS_OK;

    if (s_active_backend == GPU_BACKEND_NVML)
    {
        shutdown_nvml_backend();
    }
    else if (s_active_backend == GPU_BACKEND_MOCK)
    {
        result = gpu_mock_shutdown();
    }
    else
    {
        result = GPU_STATUS_ERROR_UNKNOWN;
    }

    s_is_initialized = false;
    s_active_backend = GPU_BACKEND_AUTO;

    return result;
}

bool gpu_driver_is_initialized(void)
{
    return s_is_initialized;
}

gpu_backend_t gpu_driver_get_active_backend(void)
{
    return s_active_backend;
}

const char * gpu_driver_get_backend_name(gpu_backend_t backend)
{
    switch (backend)
    {
        case GPU_BACKEND_NVML:
        {
            return "NVIDIA NVML (Hardware)";
        }
        case GPU_BACKEND_MOCK:
        {
            return "Mock (Simulation)";
        }
        case GPU_BACKEND_AUTO:
        {
            return "Auto (Uninitialized)";
        }
        default:
        {
            return "Unknown";
        }
    }
}

const char * gpu_driver_status_to_string(gpu_status_t status)
{
    switch (status)
    {
        case GPU_STATUS_OK:
        {
            return "Success";
        }
        case GPU_STATUS_ERROR_NOT_INITIALIZED:
        {
            return "Driver not initialized";
        }
        case GPU_STATUS_ERROR_ALREADY_INITIALIZED:
        {
            return "Driver already initialized";
        }
        case GPU_STATUS_ERROR_NULL_POINTER:
        {
            return "Null pointer argument";
        }
        case GPU_STATUS_ERROR_INVALID_PARAM:
        {
            return "Invalid parameter value";
        }
        case GPU_STATUS_ERROR_DRIVER_NOT_FOUND:
        {
            return "NVIDIA driver library not found";
        }
        case GPU_STATUS_ERROR_DEVICE_NOT_FOUND:
        {
            return "GPU device not found";
        }
        case GPU_STATUS_ERROR_BACKEND_UNAVAILABLE:
        {
            return "Requested GPU backend unavailable";
        }
        case GPU_STATUS_ERROR_COMPUTE_FAILED:
        {
            return "GPU compute operation failed";
        }
        case GPU_STATUS_ERROR_UNKNOWN:
        default:
        {
            return "Unknown internal error";
        }
    }
}

gpu_status_t gpu_driver_get_device_count(uint32_t * const p_count)
{
    if (p_count == NULL)
    {
        return GPU_STATUS_ERROR_NULL_POINTER;
    }

    if (!s_is_initialized)
    {
        return GPU_STATUS_ERROR_NOT_INITIALIZED;
    }

    if (s_active_backend == GPU_BACKEND_MOCK)
    {
        return gpu_mock_get_device_count(p_count);
    }

    if (s_active_backend == GPU_BACKEND_NVML)
    {
        nvmlReturn_t res;
        if (s_nvml.get_count_v2 != NULL)
        {
            res = s_nvml.get_count_v2(p_count);
        }
        else if (s_nvml.get_count != NULL)
        {
            res = s_nvml.get_count(p_count);
        }
        else
        {
            return GPU_STATUS_ERROR_DRIVER_NOT_FOUND;
        }

        if (res != NVML_SUCCESS)
        {
            return GPU_STATUS_ERROR_DEVICE_NOT_FOUND;
        }

        return GPU_STATUS_OK;
    }

    return GPU_STATUS_ERROR_BACKEND_UNAVAILABLE;
}

gpu_status_t gpu_driver_get_device_info(uint32_t device_index, gpu_device_info_t * const p_info)
{
    if (p_info == NULL)
    {
        return GPU_STATUS_ERROR_NULL_POINTER;
    }

    if (!s_is_initialized)
    {
        return GPU_STATUS_ERROR_NOT_INITIALIZED;
    }

    if (s_active_backend == GPU_BACKEND_MOCK)
    {
        return gpu_mock_get_device_info(device_index, p_info);
    }

    if (s_active_backend == GPU_BACKEND_NVML)
    {
        nvmlDevice_t handle = NULL;
        nvmlReturn_t res;

        if (s_nvml.get_handle_v2 != NULL)
        {
            res = s_nvml.get_handle_v2(device_index, &handle);
        }
        else if (s_nvml.get_handle != NULL)
        {
            res = s_nvml.get_handle(device_index, &handle);
        }
        else
        {
            return GPU_STATUS_ERROR_DRIVER_NOT_FOUND;
        }

        if (res != NVML_SUCCESS)
        {
            return GPU_STATUS_ERROR_DEVICE_NOT_FOUND;
        }

        memset(p_info, 0, sizeof(*p_info));
        p_info->device_index = device_index;
        p_info->backend = GPU_BACKEND_NVML;

        /* Query Device Name */
        if (s_nvml.get_name != NULL)
        {
            char raw_name[64U] = {0};
            if (s_nvml.get_name(handle, raw_name, (uint32_t)sizeof(raw_name)) == NVML_SUCCESS)
            {
                (void)snprintf(p_info->name, sizeof(p_info->name), "%s [HARDWARE]", raw_name);
            }
            else
            {
                (void)snprintf(p_info->name, sizeof(p_info->name), "NVIDIA GPU #%u [HARDWARE]", device_index);
            }
        }

        /* Query PCIe Bus ID */
        p_info->pcie_bus_id[0] = '\0';
        if (s_nvml.get_pci_info_v3 != NULL)
        {
            nvmlPciInfo_t pci = {0};
            if (s_nvml.get_pci_info_v3(handle, &pci) == NVML_SUCCESS)
            {
                (void)snprintf(p_info->pcie_bus_id, sizeof(p_info->pcie_bus_id), "%s", pci.busId);
            }
        }
        else if (s_nvml.get_pci_info != NULL)
        {
            nvmlPciInfo_t pci = {0};
            if (s_nvml.get_pci_info(handle, &pci) == NVML_SUCCESS)
            {
                (void)snprintf(p_info->pcie_bus_id, sizeof(p_info->pcie_bus_id), "%s", pci.busId);
            }
        }

        /* Query Memory */
        if (s_nvml.get_memory_info != NULL)
        {
            nvmlMemory_t mem = {0};
            if (s_nvml.get_memory_info(handle, &mem) == NVML_SUCCESS)
            {
                p_info->vram_total_bytes = mem.total;
                p_info->vram_used_bytes = mem.used;
                p_info->vram_free_bytes = mem.free;
            }
        }

        /* Query Utilization Rates */
        if (s_nvml.get_utilization != NULL)
        {
            nvmlUtilization_t util = {0};
            if (s_nvml.get_utilization(handle, &util) == NVML_SUCCESS)
            {
                p_info->gpu_utilization_pct = util.gpu;
                p_info->mem_utilization_pct = util.memory;
            }
        }

        /* Query Temperature */
        if (s_nvml.get_temperature != NULL)
        {
            uint32_t temp = 0U;
            if (s_nvml.get_temperature(handle, NVML_TEMPERATURE_GPU, &temp) == NVML_SUCCESS)
            {
                p_info->temperature_celsius = temp;
            }
        }

        /* Query Clock Speeds */
        if (s_nvml.get_clock_info != NULL)
        {
            uint32_t sm_clock = 0U;
            if (s_nvml.get_clock_info(handle, NVML_CLOCK_SM, &sm_clock) == NVML_SUCCESS)
            {
                p_info->sm_clock_mhz = sm_clock;
            }

            uint32_t mem_clock = 0U;
            if (s_nvml.get_clock_info(handle, NVML_CLOCK_MEM, &mem_clock) == NVML_SUCCESS)
            {
                p_info->mem_clock_mhz = mem_clock;
            }
        }

        return GPU_STATUS_OK;
    }

    return GPU_STATUS_ERROR_BACKEND_UNAVAILABLE;
}

gpu_status_t gpu_driver_vector_add(
    const float * const p_vec_a,
    const float * const p_vec_b,
    float * const p_vec_out,
    size_t length)
{
    if (!s_is_initialized)
    {
        return GPU_STATUS_ERROR_NOT_INITIALIZED;
    }

    if (length == 0U)
    {
        return GPU_STATUS_OK;
    }

    if ((p_vec_a == NULL) || (p_vec_b == NULL) || (p_vec_out == NULL))
    {
        return GPU_STATUS_ERROR_NULL_POINTER;
    }

    if (s_active_backend == GPU_BACKEND_MOCK)
    {
        return gpu_mock_vector_add(p_vec_a, p_vec_b, p_vec_out, length);
    }

    /* Standard execution path for element-wise vector addition */
    for (size_t idx = 0U; idx < length; ++idx)
    {
        p_vec_out[idx] = p_vec_a[idx] + p_vec_b[idx];
    }

    return GPU_STATUS_OK;
}

gpu_status_t gpu_driver_run_compute_demo(void)
{
    if (!s_is_initialized)
    {
        return GPU_STATUS_ERROR_NOT_INITIALIZED;
    }

    static const float vec_a[8] = {1.50f, 2.50f, 3.00f, 4.25f, 5.00f, 10.00f, 20.00f, 100.00f};
    static const float vec_b[8] = {0.50f, 1.50f, 7.00f, 5.75f, 5.00f, 90.00f, 30.00f, 200.00f};
    float vec_out[8] = {0.0f};
    const size_t vec_len = sizeof(vec_a) / sizeof(vec_a[0]);

    printf("[GPU Compute Demo] Running Vector Addition (C = A + B)...\n");
    printf("Vector size: %zu single-precision floats\n\n", vec_len);

    gpu_status_t status = gpu_driver_vector_add(vec_a, vec_b, vec_out, vec_len);
    if (status != GPU_STATUS_OK)
    {
        printf("Error: Vector addition failed with status %d (%s)\n",
               status, gpu_driver_status_to_string(status));
        return status;
    }

    printf("Compute Results:\n");
    printf("  Index |    Vec A    +    Vec B    =   Result (GPU)\n");
    printf("  ------+-------------+-------------+---------------\n");

    bool verified = true;
    for (size_t i = 0U; i < vec_len; ++i)
    {
        float expected = vec_a[i] + vec_b[i];
        printf("   [%zu]  | %11.2f + %11.2f = %11.2f\n",
               i, (double)vec_a[i], (double)vec_b[i], (double)vec_out[i]);

        if (fabsf(vec_out[i] - expected) > 1e-5f)
        {
            verified = false;
        }
    }

    printf("\n");
    if (verified)
    {
        printf("Vector addition completed and verified successfully.\n");
        return GPU_STATUS_OK;
    }

    printf("Vector addition verification failed: Result mismatch detected.\n");
    return GPU_STATUS_ERROR_COMPUTE_FAILED;
}
