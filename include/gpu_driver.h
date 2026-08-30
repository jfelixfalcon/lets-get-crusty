/**
 * @file gpu_driver.h
 * @brief High-level GPU Hardware Abstraction Layer & Telemetry API.
 *
 * Provides a production-grade, BARR-C:2018 compliant interface for querying
 * NVIDIA GPU telemetry (VRAM, utilization, clock speeds, temperature, PCIe ID)
 * and executing compute operations via hardware (NVML/CUDA) or simulated backends.
 */

#ifndef GPU_DRIVER_H
#define GPU_DRIVER_H

#include "gpu_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the GPU driver subsystem with the specified backend.
 *
 * @param[in] requested_backend Desired backend (AUTO, NVML, or MOCK).
 * @return GPU_STATUS_OK on success, or an error status code.
 * @retval GPU_STATUS_ERROR_ALREADY_INITIALIZED if subsystem was already initialized.
 * @retval GPU_STATUS_ERROR_BACKEND_UNAVAILABLE if requested backend is unavailable.
 * @retval GPU_STATUS_ERROR_INVALID_PARAM if requested_backend value is invalid.
 */
gpu_status_t gpu_driver_init(gpu_backend_t requested_backend);

/**
 * @brief Shuts down the GPU driver subsystem and frees all allocated resources.
 *
 * @return GPU_STATUS_OK on success, or GPU_STATUS_ERROR_NOT_INITIALIZED if not running.
 */
gpu_status_t gpu_driver_shutdown(void);

/**
 * @brief Checks if the GPU driver subsystem is currently initialized.
 *
 * @return true if initialized, false otherwise.
 */
bool gpu_driver_is_initialized(void);

/**
 * @brief Returns the active GPU communication backend currently in use.
 *
 * @return Active gpu_backend_t identifier, or GPU_BACKEND_AUTO if uninitialized.
 */
gpu_backend_t gpu_driver_get_active_backend(void);

/**
 * @brief Converts a backend identifier to a human-readable display string.
 *
 * @param[in] backend Backend identifier enum.
 * @return Pointer to a static, null-terminated string describing the backend.
 */
const char * gpu_driver_get_backend_name(gpu_backend_t backend);

/**
 * @brief Converts a status code to a human-readable descriptive string.
 *
 * @param[in] status Status code to convert.
 * @return Pointer to a static, null-terminated string describing the status.
 */
const char * gpu_driver_status_to_string(gpu_status_t status);

/**
 * @brief Queries the total number of detected GPU devices.
 *
 * @param[out] p_count Pointer to memory where device count will be stored.
 * @return GPU_STATUS_OK on success, or an error code.
 * @retval GPU_STATUS_ERROR_NULL_POINTER if p_count is NULL.
 * @retval GPU_STATUS_ERROR_NOT_INITIALIZED if driver is not initialized.
 */
gpu_status_t gpu_driver_get_device_count(uint32_t * const p_count);

/**
 * @brief Retrieves detailed telemetry and specifications for a specific GPU device.
 *
 * @param[in]  device_index Zero-based index of the GPU device to query.
 * @param[out] p_info       Pointer to gpu_device_info_t structure to populate.
 * @return GPU_STATUS_OK on success, or an error code.
 * @retval GPU_STATUS_ERROR_NULL_POINTER if p_info is NULL.
 * @retval GPU_STATUS_ERROR_NOT_INITIALIZED if driver is not initialized.
 * @retval GPU_STATUS_ERROR_DEVICE_NOT_FOUND if device_index is out of range.
 */
gpu_status_t gpu_driver_get_device_info(uint32_t device_index, gpu_device_info_t * const p_info);

/**
 * @brief Executes element-wise vector addition: p_vec_out[i] = p_vec_a[i] + p_vec_b[i].
 *
 * @param[in]  p_vec_a    Pointer to input vector A array.
 * @param[in]  p_vec_b    Pointer to input vector B array.
 * @param[out] p_vec_out  Pointer to output vector buffer.
 * @param[in]  length     Number of single-precision floating point elements.
 * @return GPU_STATUS_OK on success, or an error code.
 * @retval GPU_STATUS_ERROR_NULL_POINTER if any vector pointer is NULL (when length > 0).
 * @retval GPU_STATUS_ERROR_NOT_INITIALIZED if driver is not initialized.
 */
gpu_status_t gpu_driver_vector_add(
    const float * const p_vec_a,
    const float * const p_vec_b,
    float * const p_vec_out,
    size_t length);

/**
 * @brief Runs an interactive vector addition compute demo, verifying results.
 *
 * @return GPU_STATUS_OK on success, or an error code.
 */
gpu_status_t gpu_driver_run_compute_demo(void);

#ifdef __cplusplus
}
#endif

#endif /* GPU_DRIVER_H */
