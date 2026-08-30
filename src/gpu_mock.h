/**
 * @file gpu_mock.h
 * @brief Internal interface for the deterministic GPU simulation mock backend.
 */

#ifndef GPU_MOCK_H
#define GPU_MOCK_H

#include "gpu_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the mock simulation GPU backend.
 *
 * @return GPU_STATUS_OK on success.
 */
gpu_status_t gpu_mock_init(void);

/**
 * @brief Shuts down the mock simulation GPU backend.
 *
 * @return GPU_STATUS_OK on success.
 */
gpu_status_t gpu_mock_shutdown(void);

/**
 * @brief Retrieves the number of simulated GPU devices.
 *
 * @param[out] p_count Pointer to uint32_t to store simulated device count.
 * @return GPU_STATUS_OK on success, or GPU_STATUS_ERROR_NULL_POINTER.
 */
gpu_status_t gpu_mock_get_device_count(uint32_t * const p_count);

/**
 * @brief Retrieves simulated telemetry data for the specified mock device index.
 *
 * @param[in]  device_index Zero-based index of simulated device.
 * @param[out] p_info       Pointer to gpu_device_info_t struct to populate.
 * @return GPU_STATUS_OK on success, or error status.
 */
gpu_status_t gpu_mock_get_device_info(uint32_t device_index, gpu_device_info_t * const p_info);

/**
 * @brief Simulates vector addition compute operation.
 *
 * @param[in]  p_vec_a    Input vector A.
 * @param[in]  p_vec_b    Input vector B.
 * @param[out] p_vec_out  Output vector buffer.
 * @param[in]  length     Number of elements.
 * @return GPU_STATUS_OK on success, or error status.
 */
gpu_status_t gpu_mock_vector_add(
    const float * const p_vec_a,
    const float * const p_vec_b,
    float * const p_vec_out,
    size_t length);

#ifdef __cplusplus
}
#endif

#endif /* GPU_MOCK_H */
