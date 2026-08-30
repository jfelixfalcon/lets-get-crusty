/**
 * @file gpu_nvml_types.h
 * @brief NVIDIA Management Library (NVML) dynamic definitions and function pointers.
 *
 * Provides zero-dependency dynamic loader interfaces for communicating with
 * libnvidia-ml.so without requiring NVIDIA proprietary headers at compile time.
 */

#ifndef GPU_NVML_TYPES_H
#define GPU_NVML_TYPES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** NVML Return Status Codes */
typedef enum
{
    NVML_SUCCESS = 0,
    NVML_ERROR_UNINITIALIZED = 1,
    NVML_ERROR_INVALID_ARGUMENT = 2,
    NVML_ERROR_NOT_SUPPORTED = 3,
    NVML_ERROR_NO_PERMISSION = 4,
    NVML_ERROR_ALREADY_INITIALIZED = 5,
    NVML_ERROR_NOT_FOUND = 6,
    NVML_ERROR_INSUFFICIENT_SIZE = 7,
    NVML_ERROR_INSUFFICIENT_POWER = 8,
    NVML_ERROR_DRIVER_NOT_LOADED = 9,
    NVML_ERROR_TIMEOUT = 10,
    NVML_ERROR_IRQ_ISSUE = 11,
    NVML_ERROR_LIBRARY_NOT_FOUND = 12,
    NVML_ERROR_FUNCTION_NOT_FOUND = 13,
    NVML_ERROR_CORRUPTED_INFOROM = 14,
    NVML_ERROR_GPU_IS_LOST = 15,
    NVML_ERROR_RESET_REQUIRED = 16,
    NVML_ERROR_OPERATING_SYSTEM = 17,
    NVML_ERROR_LIB_RM_VERSION_MISMATCH = 18,
    NVML_ERROR_IN_USE = 19,
    NVML_ERROR_MEMORY = 20,
    NVML_ERROR_NO_DATA = 21,
    NVML_ERROR_VGPU_ECC_NOT_SUPPORTED = 22,
    NVML_ERROR_INSUFFICIENT_RESOURCES = 23,
    NVML_ERROR_FREQ_NOT_SUPPORTED = 24,
    NVML_ERROR_UNKNOWN = 999
} nvmlReturn_t;

/** Opaque handle to NVML device. */
typedef struct nvmlDevice_st *nvmlDevice_t;

/** NVML Memory Telemetry Structure. */
typedef struct
{
    uint64_t total; /**< Total physical device memory in bytes. */
    uint64_t free;  /**< Unallocated device memory in bytes. */
    uint64_t used;  /**< Allocated device memory in bytes. */
} nvmlMemory_t;

/** NVML Utilization Rates Structure. */
typedef struct
{
    uint32_t gpu;    /**< Percent of time over past sample period GPU was active [0-100]. */
    uint32_t memory; /**< Percent of time memory controller was reading/writing [0-100]. */
} nvmlUtilization_t;

/** NVML PCI Info Structure. */
typedef struct
{
    char busIdLegacy[16];
    uint32_t domain;
    uint32_t bus;
    uint32_t device;
    uint32_t pciDeviceId;
    uint32_t pciSubSystemId;
    char busId[32]; /**< Full PCIe bus identifier string. */
} nvmlPciInfo_t;

/** NVML Temperature Sensors Enumeration. */
typedef enum
{
    NVML_TEMPERATURE_GPU = 0,
    NVML_TEMPERATURE_COUNT
} nvmlTemperatureSensors_t;

/** NVML Clock Domain Enumeration. */
typedef enum
{
    NVML_CLOCK_GRAPHICS = 0,
    NVML_CLOCK_SM = 1,
    NVML_CLOCK_MEM = 2,
    NVML_CLOCK_VIDEO = 3,
    NVML_CLOCK_COUNT
} nvmlClockType_t;

/* NVML Function Pointer Signatures */
typedef nvmlReturn_t (*pfn_nvmlInit_v2)(void);
typedef nvmlReturn_t (*pfn_nvmlInit)(void);
typedef nvmlReturn_t (*pfn_nvmlShutdown)(void);
typedef nvmlReturn_t (*pfn_nvmlDeviceGetCount_v2)(uint32_t *p_count);
typedef nvmlReturn_t (*pfn_nvmlDeviceGetCount)(uint32_t *p_count);
typedef nvmlReturn_t (*pfn_nvmlDeviceGetHandleByIndex_v2)(uint32_t index, nvmlDevice_t *p_device);
typedef nvmlReturn_t (*pfn_nvmlDeviceGetHandleByIndex)(uint32_t index, nvmlDevice_t *p_device);
typedef nvmlReturn_t (*pfn_nvmlDeviceGetName)(nvmlDevice_t device, char *p_name, uint32_t length);
typedef nvmlReturn_t (*pfn_nvmlDeviceGetPciInfo_v3)(nvmlDevice_t device, nvmlPciInfo_t *p_pci);
typedef nvmlReturn_t (*pfn_nvmlDeviceGetPciInfo)(nvmlDevice_t device, nvmlPciInfo_t *p_pci);
typedef nvmlReturn_t (*pfn_nvmlDeviceGetMemoryInfo)(nvmlDevice_t device, nvmlMemory_t *p_memory);
typedef nvmlReturn_t (*pfn_nvmlDeviceGetUtilizationRates)(nvmlDevice_t device, nvmlUtilization_t *p_utilization);
typedef nvmlReturn_t (*pfn_nvmlDeviceGetTemperature)(nvmlDevice_t device, nvmlTemperatureSensors_t sensor, uint32_t *p_temp);
typedef nvmlReturn_t (*pfn_nvmlDeviceGetClockInfo)(nvmlDevice_t device, nvmlClockType_t type, uint32_t *p_clock);
typedef const char * (*pfn_nvmlErrorString)(nvmlReturn_t result);

/**
 * @brief Dynamic function dispatch table for loaded NVML symbols.
 */
typedef struct
{
    void *p_lib_handle;
    pfn_nvmlInit_v2 init_v2;
    pfn_nvmlInit init;
    pfn_nvmlShutdown shutdown;
    pfn_nvmlDeviceGetCount_v2 get_count_v2;
    pfn_nvmlDeviceGetCount get_count;
    pfn_nvmlDeviceGetHandleByIndex_v2 get_handle_v2;
    pfn_nvmlDeviceGetHandleByIndex get_handle;
    pfn_nvmlDeviceGetName get_name;
    pfn_nvmlDeviceGetPciInfo_v3 get_pci_info_v3;
    pfn_nvmlDeviceGetPciInfo get_pci_info;
    pfn_nvmlDeviceGetMemoryInfo get_memory_info;
    pfn_nvmlDeviceGetUtilizationRates get_utilization;
    pfn_nvmlDeviceGetTemperature get_temperature;
    pfn_nvmlDeviceGetClockInfo get_clock_info;
    pfn_nvmlErrorString error_string;
} gpu_nvml_table_t;

#ifdef __cplusplus
}
#endif

#endif /* GPU_NVML_TYPES_H */
