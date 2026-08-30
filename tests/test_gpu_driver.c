/**
 * @file test_gpu_driver.c
 * @brief Comprehensive unit tests for GPU driver and mock simulation backend.
 *
 * Adheres strictly to BARR-C:2018 coding standards and uses the Unity Test Framework.
 */

#include "gpu_driver.h"
#include "unity/unity.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

void setUp(void)
{
    /* Ensure clean uninitialized state before each test */
    if (gpu_driver_is_initialized())
    {
        (void)gpu_driver_shutdown();
    }
}

void tearDown(void)
{
    /* Cleanup any initialized state after each test */
    if (gpu_driver_is_initialized())
    {
        (void)gpu_driver_shutdown();
    }
}

/* ========================================================================= */
/* Lifecycle Tests                                                           */
/* ========================================================================= */

static void test_lifecycle_uninitialized_calls_fail(void)
{
    uint32_t count = 0U;
    gpu_device_info_t info;
    float a[2] = {1.0f, 2.0f};
    float b[2] = {3.0f, 4.0f};
    float out[2] = {0.0f, 0.0f};

    TEST_ASSERT_FALSE(gpu_driver_is_initialized());
    TEST_ASSERT_EQUAL_INT(GPU_BACKEND_AUTO, gpu_driver_get_active_backend());

    TEST_ASSERT_EQUAL_INT(
        GPU_STATUS_ERROR_NOT_INITIALIZED,
        gpu_driver_get_device_count(&count)
    );

    TEST_ASSERT_EQUAL_INT(
        GPU_STATUS_ERROR_NOT_INITIALIZED,
        gpu_driver_get_device_info(0U, &info)
    );

    TEST_ASSERT_EQUAL_INT(
        GPU_STATUS_ERROR_NOT_INITIALIZED,
        gpu_driver_vector_add(a, b, out, 2U)
    );

    TEST_ASSERT_EQUAL_INT(
        GPU_STATUS_ERROR_NOT_INITIALIZED,
        gpu_driver_run_compute_demo()
    );

    TEST_ASSERT_EQUAL_INT(
        GPU_STATUS_ERROR_NOT_INITIALIZED,
        gpu_driver_shutdown()
    );
}

static void test_lifecycle_init_and_shutdown_mock(void)
{
    TEST_ASSERT_FALSE(gpu_driver_is_initialized());

    gpu_status_t status = gpu_driver_init(GPU_BACKEND_MOCK);
    TEST_ASSERT_EQUAL_INT(GPU_STATUS_OK, status);
    TEST_ASSERT_TRUE(gpu_driver_is_initialized());
    TEST_ASSERT_EQUAL_INT(GPU_BACKEND_MOCK, gpu_driver_get_active_backend());

    status = gpu_driver_shutdown();
    TEST_ASSERT_EQUAL_INT(GPU_STATUS_OK, status);
    TEST_ASSERT_FALSE(gpu_driver_is_initialized());
    TEST_ASSERT_EQUAL_INT(GPU_BACKEND_AUTO, gpu_driver_get_active_backend());
}

static void test_lifecycle_double_init_fails(void)
{
    TEST_ASSERT_EQUAL_INT(GPU_STATUS_OK, gpu_driver_init(GPU_BACKEND_MOCK));
    TEST_ASSERT_TRUE(gpu_driver_is_initialized());

    gpu_status_t status = gpu_driver_init(GPU_BACKEND_MOCK);
    TEST_ASSERT_EQUAL_INT(GPU_STATUS_ERROR_ALREADY_INITIALIZED, status);

    (void)gpu_driver_shutdown();
}

static void test_lifecycle_invalid_backend_rejected(void)
{
    gpu_status_t status = gpu_driver_init((gpu_backend_t)999);
    TEST_ASSERT_EQUAL_INT(GPU_STATUS_ERROR_INVALID_PARAM, status);
    TEST_ASSERT_FALSE(gpu_driver_is_initialized());
}

/* ========================================================================= */
/* Defensive Parameter Verification Tests                                    */
/* ========================================================================= */

static void test_defensive_null_pointer_checks(void)
{
    TEST_ASSERT_EQUAL_INT(GPU_STATUS_OK, gpu_driver_init(GPU_BACKEND_MOCK));

    TEST_ASSERT_EQUAL_INT(
        GPU_STATUS_ERROR_NULL_POINTER,
        gpu_driver_get_device_count(NULL)
    );

    TEST_ASSERT_EQUAL_INT(
        GPU_STATUS_ERROR_NULL_POINTER,
        gpu_driver_get_device_info(0U, NULL)
    );

    float buf[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    TEST_ASSERT_EQUAL_INT(
        GPU_STATUS_ERROR_NULL_POINTER,
        gpu_driver_vector_add(NULL, buf, buf, 4U)
    );

    TEST_ASSERT_EQUAL_INT(
        GPU_STATUS_ERROR_NULL_POINTER,
        gpu_driver_vector_add(buf, NULL, buf, 4U)
    );

    TEST_ASSERT_EQUAL_INT(
        GPU_STATUS_ERROR_NULL_POINTER,
        gpu_driver_vector_add(buf, buf, NULL, 4U)
    );

    (void)gpu_driver_shutdown();
}

static void test_defensive_device_out_of_bounds(void)
{
    TEST_ASSERT_EQUAL_INT(GPU_STATUS_OK, gpu_driver_init(GPU_BACKEND_MOCK));

    uint32_t count = 0U;
    TEST_ASSERT_EQUAL_INT(GPU_STATUS_OK, gpu_driver_get_device_count(&count));

    gpu_device_info_t info;
    TEST_ASSERT_EQUAL_INT(
        GPU_STATUS_ERROR_DEVICE_NOT_FOUND,
        gpu_driver_get_device_info(count, &info)
    );

    TEST_ASSERT_EQUAL_INT(
        GPU_STATUS_ERROR_DEVICE_NOT_FOUND,
        gpu_driver_get_device_info(count + 10U, &info)
    );

    (void)gpu_driver_shutdown();
}

/* ========================================================================= */
/* Telemetry Inspection Tests                                                */
/* ========================================================================= */

static void test_mock_device_telemetry(void)
{
    TEST_ASSERT_EQUAL_INT(GPU_STATUS_OK, gpu_driver_init(GPU_BACKEND_MOCK));

    uint32_t count = 0U;
    TEST_ASSERT_EQUAL_INT(GPU_STATUS_OK, gpu_driver_get_device_count(&count));
    TEST_ASSERT_EQUAL_UINT32(2U, count);

    gpu_device_info_t dev0;
    TEST_ASSERT_EQUAL_INT(GPU_STATUS_OK, gpu_driver_get_device_info(0U, &dev0));
    TEST_ASSERT_EQUAL_UINT32(0U, dev0.device_index);
    TEST_ASSERT_EQUAL_STRING("NVIDIA GeForce RTX 4090 [SIMULATED]", dev0.name);
    TEST_ASSERT_EQUAL_STRING("00000000:01:00.0", dev0.pcie_bus_id);
    TEST_ASSERT_EQUAL_UINT64(25769803776ULL, dev0.vram_total_bytes);
    TEST_ASSERT_EQUAL_UINT64(dev0.vram_total_bytes, dev0.vram_used_bytes + dev0.vram_free_bytes);
    TEST_ASSERT_TRUE(dev0.temperature_celsius > 0U);
    TEST_ASSERT_TRUE(dev0.sm_clock_mhz > 0U);
    TEST_ASSERT_TRUE(dev0.mem_clock_mhz > 0U);
    TEST_ASSERT_EQUAL_INT(GPU_BACKEND_MOCK, dev0.backend);

    gpu_device_info_t dev1;
    TEST_ASSERT_EQUAL_INT(GPU_STATUS_OK, gpu_driver_get_device_info(1U, &dev1));
    TEST_ASSERT_EQUAL_UINT32(1U, dev1.device_index);
    TEST_ASSERT_EQUAL_STRING("NVIDIA H100 80GB HBM3 [SIMULATED]", dev1.name);
    TEST_ASSERT_EQUAL_STRING("00000000:41:00.0", dev1.pcie_bus_id);
    TEST_ASSERT_EQUAL_UINT64(85899345920ULL, dev1.vram_total_bytes);
    TEST_ASSERT_EQUAL_UINT64(dev1.vram_total_bytes, dev1.vram_used_bytes + dev1.vram_free_bytes);
    TEST_ASSERT_EQUAL_INT(GPU_BACKEND_MOCK, dev1.backend);

    (void)gpu_driver_shutdown();
}

/* ========================================================================= */
/* Compute Tests                                                             */
/* ========================================================================= */

static void test_vector_addition_correctness(void)
{
    TEST_ASSERT_EQUAL_INT(GPU_STATUS_OK, gpu_driver_init(GPU_BACKEND_MOCK));

    const float a[5] = {1.0f, -2.5f, 0.0f, 100.25f, -50.0f};
    const float b[5] = {3.5f,  2.5f, 0.0f, -0.25f,   25.0f};
    float out[5] = {0.0f};

    gpu_status_t status = gpu_driver_vector_add(a, b, out, 5U);
    TEST_ASSERT_EQUAL_INT(GPU_STATUS_OK, status);

    TEST_ASSERT_FLOAT_WITHIN(1e-5f, 4.5f, out[0]);
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, 0.0f, out[1]);
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, 0.0f, out[2]);
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, 100.0f, out[3]);
    TEST_ASSERT_FLOAT_WITHIN(1e-5f, -25.0f, out[4]);

    (void)gpu_driver_shutdown();
}

static void test_vector_addition_zero_length(void)
{
    TEST_ASSERT_EQUAL_INT(GPU_STATUS_OK, gpu_driver_init(GPU_BACKEND_MOCK));

    /* Zero length should return OK immediately without dereferencing pointers */
    gpu_status_t status = gpu_driver_vector_add(NULL, NULL, NULL, 0U);
    TEST_ASSERT_EQUAL_INT(GPU_STATUS_OK, status);

    (void)gpu_driver_shutdown();
}

static void test_compute_demo_execution(void)
{
    TEST_ASSERT_EQUAL_INT(GPU_STATUS_OK, gpu_driver_init(GPU_BACKEND_MOCK));
    TEST_ASSERT_EQUAL_INT(GPU_STATUS_OK, gpu_driver_run_compute_demo());
    (void)gpu_driver_shutdown();
}

/* ========================================================================= */
/* Helper & Conversion Tests                                                 */
/* ========================================================================= */

static void test_status_strings(void)
{
    TEST_ASSERT_NOT_NULL(gpu_driver_status_to_string(GPU_STATUS_OK));
    TEST_ASSERT_NOT_NULL(gpu_driver_status_to_string(GPU_STATUS_ERROR_NOT_INITIALIZED));
    TEST_ASSERT_NOT_NULL(gpu_driver_status_to_string(GPU_STATUS_ERROR_ALREADY_INITIALIZED));
    TEST_ASSERT_NOT_NULL(gpu_driver_status_to_string(GPU_STATUS_ERROR_NULL_POINTER));
    TEST_ASSERT_NOT_NULL(gpu_driver_status_to_string(GPU_STATUS_ERROR_INVALID_PARAM));
    TEST_ASSERT_NOT_NULL(gpu_driver_status_to_string(GPU_STATUS_ERROR_DRIVER_NOT_FOUND));
    TEST_ASSERT_NOT_NULL(gpu_driver_status_to_string(GPU_STATUS_ERROR_DEVICE_NOT_FOUND));
    TEST_ASSERT_NOT_NULL(gpu_driver_status_to_string(GPU_STATUS_ERROR_BACKEND_UNAVAILABLE));
    TEST_ASSERT_NOT_NULL(gpu_driver_status_to_string(GPU_STATUS_ERROR_COMPUTE_FAILED));
    TEST_ASSERT_NOT_NULL(gpu_driver_status_to_string(GPU_STATUS_ERROR_UNKNOWN));
    TEST_ASSERT_NOT_NULL(gpu_driver_status_to_string((gpu_status_t)999));
}

static void test_backend_names(void)
{
    TEST_ASSERT_EQUAL_STRING("NVIDIA NVML (Hardware)", gpu_driver_get_backend_name(GPU_BACKEND_NVML));
    TEST_ASSERT_EQUAL_STRING("Mock (Simulation)", gpu_driver_get_backend_name(GPU_BACKEND_MOCK));
    TEST_ASSERT_EQUAL_STRING("Auto (Uninitialized)", gpu_driver_get_backend_name(GPU_BACKEND_AUTO));
    TEST_ASSERT_EQUAL_STRING("Unknown", gpu_driver_get_backend_name((gpu_backend_t)999));
}

/* ========================================================================= */
/* Test Runner Entry Point                                                   */
/* ========================================================================= */

int main(void)
{
    UnityBegin("test_gpu_driver.c");

    /* Lifecycle */
    RUN_TEST(test_lifecycle_uninitialized_calls_fail);
    RUN_TEST(test_lifecycle_init_and_shutdown_mock);
    RUN_TEST(test_lifecycle_double_init_fails);
    RUN_TEST(test_lifecycle_invalid_backend_rejected);

    /* Defensive Parameter Checks */
    RUN_TEST(test_defensive_null_pointer_checks);
    RUN_TEST(test_defensive_device_out_of_bounds);

    /* Telemetry */
    RUN_TEST(test_mock_device_telemetry);

    /* Compute */
    RUN_TEST(test_vector_addition_correctness);
    RUN_TEST(test_vector_addition_zero_length);
    RUN_TEST(test_compute_demo_execution);

    /* Helpers */
    RUN_TEST(test_status_strings);
    RUN_TEST(test_backend_names);

    return UnityEnd();
}
