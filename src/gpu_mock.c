/**
 * @file gpu_mock.c
 * @brief Deterministic offline GPU simulation backend implementation.
 *
 * Implements a simulated NVIDIA GPU hardware environment for testing, CI/CD,
 * and environments without physical GPUs. Complies with BARR-C:2018.
 */

#include "gpu_mock.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/** Total number of simulated devices in the mock backend. */
#define MOCK_DEVICE_COUNT (2U)

/** Internal mock state flag. */
static bool s_mock_initialized = false;

/** Simulated hardware device table. */
static const gpu_device_info_t s_mock_devices[MOCK_DEVICE_COUNT] = {
    {
        .device_index = 0U,
        .name = "NVIDIA GeForce RTX 4090 [SIMULATED]",
        .pcie_bus_id = "00000000:01:00.0",
        .vram_total_bytes = 25769803776ULL, /* 24.00 GiB */
        .vram_used_bytes = 2147483648ULL,   /*  2.00 GiB */
        .vram_free_bytes = 23622320128ULL,  /* 22.00 GiB */
        .gpu_utilization_pct = 12U,
        .mem_utilization_pct = 8U,
        .temperature_celsius = 42U,
        .sm_clock_mhz = 2520U,
        .mem_clock_mhz = 10501U,
        .backend = GPU_BACKEND_MOCK
    },
    {
        .device_index = 1U,
        .name = "NVIDIA H100 80GB HBM3 [SIMULATED]",
        .pcie_bus_id = "00000000:41:00.0",
        .vram_total_bytes = 85899345920ULL, /* 80.00 GiB */
        .vram_used_bytes = 17179869184ULL,  /* 16.00 GiB */
        .vram_free_bytes = 68719476736ULL,  /* 64.00 GiB */
        .gpu_utilization_pct = 45U,
        .mem_utilization_pct = 32U,
        .temperature_celsius = 58U,
        .sm_clock_mhz = 1755U,
        .mem_clock_mhz = 2619U,
        .backend = GPU_BACKEND_MOCK
    }
};

gpu_status_t gpu_mock_init(void)
{
    if (s_mock_initialized)
    {
        return GPU_STATUS_ERROR_ALREADY_INITIALIZED;
    }

    s_mock_initialized = true;
    return GPU_STATUS_OK;
}

gpu_status_t gpu_mock_shutdown(void)
{
    if (!s_mock_initialized)
    {
        return GPU_STATUS_ERROR_NOT_INITIALIZED;
    }

    s_mock_initialized = false;
    return GPU_STATUS_OK;
}

gpu_status_t gpu_mock_get_device_count(uint32_t * const p_count)
{
    if (p_count == NULL)
    {
        return GPU_STATUS_ERROR_NULL_POINTER;
    }

    if (!s_mock_initialized)
    {
        return GPU_STATUS_ERROR_NOT_INITIALIZED;
    }

    *p_count = MOCK_DEVICE_COUNT;
    return GPU_STATUS_OK;
}

gpu_status_t gpu_mock_get_device_info(uint32_t device_index, gpu_device_info_t * const p_info)
{
    if (p_info == NULL)
    {
        return GPU_STATUS_ERROR_NULL_POINTER;
    }

    if (!s_mock_initialized)
    {
        return GPU_STATUS_ERROR_NOT_INITIALIZED;
    }

    if (device_index >= MOCK_DEVICE_COUNT)
    {
        return GPU_STATUS_ERROR_DEVICE_NOT_FOUND;
    }

    /* Copy mock device telemetry */
    *p_info = s_mock_devices[device_index];
    return GPU_STATUS_OK;
}

gpu_status_t gpu_mock_vector_add(
    const float * const p_vec_a,
    const float * const p_vec_b,
    float * const p_vec_out,
    size_t length)
{
    if (!s_mock_initialized)
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

    for (size_t idx = 0U; idx < length; ++idx)
    {
        p_vec_out[idx] = p_vec_a[idx] + p_vec_b[idx];
    }

    return GPU_STATUS_OK;
}
