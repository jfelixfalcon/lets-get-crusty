/**
 * @file gpu_types.h
 * @brief Public data types, status enumeration, and constants for GPU driver.
 *
 * Adheres to BARR-C:2018 coding standards using explicit fixed-width integers
 * and defensive type definitions.
 */

#ifndef GPU_TYPES_H
#define GPU_TYPES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Maximum length for GPU device name strings including null terminator. */
#define GPU_MAX_NAME_LEN (128U)

/** Maximum length for PCIe Bus ID strings including null terminator. */
#define GPU_MAX_BUS_ID_LEN (32U)

/** Maximum supported number of GPU devices. */
#define GPU_MAX_DEVICES (16U)

/**
 * @brief Status return codes for all GPU driver operations.
 */
typedef enum
{
    GPU_STATUS_OK = 0,                        /**< Operation completed successfully. */
    GPU_STATUS_ERROR_NOT_INITIALIZED = -1,    /**< Driver subsystem is not initialized. */
    GPU_STATUS_ERROR_ALREADY_INITIALIZED = -2,/**< Driver subsystem is already initialized. */
    GPU_STATUS_ERROR_NULL_POINTER = -3,       /**< A required pointer parameter was NULL. */
    GPU_STATUS_ERROR_INVALID_PARAM = -4,      /**< An argument value was outside valid range. */
    GPU_STATUS_ERROR_DRIVER_NOT_FOUND = -5,   /**< GPU driver shared library was not found. */
    GPU_STATUS_ERROR_DEVICE_NOT_FOUND = -6,   /**< Specified GPU device index does not exist. */
    GPU_STATUS_ERROR_BACKEND_UNAVAILABLE = -7,/**< Requested backend is not supported/available. */
    GPU_STATUS_ERROR_COMPUTE_FAILED = -8,     /**< GPU compute operation encountered an error. */
    GPU_STATUS_ERROR_UNKNOWN = -9             /**< Unspecified internal error occurred. */
} gpu_status_t;

/**
 * @brief GPU communication backend selection.
 */
typedef enum
{
    GPU_BACKEND_AUTO = 0,  /**< Automatically select best backend (NVML -> Mock fallback). */
    GPU_BACKEND_NVML = 1,  /**< Real NVIDIA Management Library (libnvidia-ml.so). */
    GPU_BACKEND_MOCK = 2   /**< Deterministic offline software simulation backend. */
} gpu_backend_t;

/**
 * @brief Comprehensive telemetry and specification metrics for a single GPU device.
 */
typedef struct
{
    uint32_t device_index;                 /**< 0-based device index. */
    char name[GPU_MAX_NAME_LEN];          /**< Model name string (e.g. "NVIDIA RTX 4090"). */
    char pcie_bus_id[GPU_MAX_BUS_ID_LEN]; /**< PCIe Bus identifier string. */
    uint64_t vram_total_bytes;             /**< Total installed VRAM in bytes. */
    uint64_t vram_used_bytes;              /**< Currently allocated/used VRAM in bytes. */
    uint64_t vram_free_bytes;              /**< Available free VRAM in bytes. */
    uint32_t gpu_utilization_pct;          /**< Core compute utilization percentage [0-100]. */
    uint32_t mem_utilization_pct;          /**< Memory controller utilization percentage [0-100]. */
    uint32_t temperature_celsius;          /**< Core temperature in degrees Celsius. */
    uint32_t sm_clock_mhz;                 /**< Streaming multiprocessor clock frequency in MHz. */
    uint32_t mem_clock_mhz;                /**< Memory clock frequency in MHz. */
    gpu_backend_t backend;                 /**< Backend providing this telemetry. */
} gpu_device_info_t;

#ifdef __cplusplus
}
#endif

#endif /* GPU_TYPES_H */
