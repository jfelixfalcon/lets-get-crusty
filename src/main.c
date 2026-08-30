/**
 * @file main.c
 * @brief Command-line interface and telemetry dashboard entrypoint.
 *
 * Demonstrates querying GPU hardware telemetry and executing compute kernels
 * in compliance with BARR-C:2018.
 */

#include "gpu_driver.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BAR_WIDTH (20U)

/**
 * @brief Formats a 20-character ASCII utilization bar.
 *
 * @param[in]  percentage Percentage value [0-100].
 * @param[out] p_buffer   Destination character buffer of at least 21 bytes.
 */
static void format_utilization_bar(uint32_t percentage, char * const p_buffer)
{
    if (p_buffer == NULL)
    {
        return;
    }

    uint32_t clamped = (percentage > 100U) ? 100U : percentage;
    uint32_t filled = (clamped * BAR_WIDTH) / 100U;

    for (uint32_t i = 0U; i < BAR_WIDTH; ++i)
    {
        if (i < filled)
        {
            p_buffer[i] = '=';
        }
        else
        {
            p_buffer[i] = '-';
        }
    }
    p_buffer[BAR_WIDTH] = '\0';
}

/**
 * @brief Prints CLI usage instructions.
 *
 * @param[in] p_prog_name Executable name string.
 */
static void print_usage(const char * const p_prog_name)
{
    const char * const p_name = (p_prog_name != NULL) ? p_prog_name : "gpu_app";
    printf("Usage: %s [OPTIONS]\n\n", p_name);
    printf("Options:\n");
    printf("  --help, -h       Display usage information and exit.\n");
    printf("  --mock, -m       Force mock/simulation mode (simulates RTX 4090 & H100).\n");
    printf("  --nvml, -n       Enforce real NVIDIA NVML driver (fails if absent).\n");
    printf("  --info, -i       Query and display detailed GPU device telemetry.\n");
    printf("  --compute, -c    Run vector addition compute demo on GPU.\n");
}

int main(int argc, char *argv[])
{
    gpu_backend_t backend = GPU_BACKEND_AUTO;
    bool show_info = false;
    bool run_compute = false;
    bool action_specified = false;

    for (int32_t i = 1; i < argc; ++i)
    {
        const char * const p_arg = argv[i];
        if ((strcmp(p_arg, "--help") == 0) || (strcmp(p_arg, "-h") == 0))
        {
            print_usage(argv[0]);
            return EXIT_SUCCESS;
        }
        else if ((strcmp(p_arg, "--mock") == 0) || (strcmp(p_arg, "-m") == 0))
        {
            backend = GPU_BACKEND_MOCK;
        }
        else if ((strcmp(p_arg, "--nvml") == 0) || (strcmp(p_arg, "-n") == 0))
        {
            backend = GPU_BACKEND_NVML;
        }
        else if ((strcmp(p_arg, "--info") == 0) || (strcmp(p_arg, "-i") == 0))
        {
            show_info = true;
            action_specified = true;
        }
        else if ((strcmp(p_arg, "--compute") == 0) || (strcmp(p_arg, "-c") == 0))
        {
            run_compute = true;
            action_specified = true;
        }
        else
        {
            fprintf(stderr, "Unknown option: %s\n\n", p_arg);
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    /* Default behavior: run both telemetry query and compute demo */
    if (!action_specified)
    {
        show_info = true;
        run_compute = true;
    }

    printf("=================================================================\n");
    printf("  Initializing GPU Subsystem...\n");
    printf("=================================================================\n");

    gpu_status_t init_status = gpu_driver_init(backend);
    if (init_status != GPU_STATUS_OK)
    {
        fprintf(stderr, "Driver Initialization Failed: %s (code %d)\n",
                gpu_driver_status_to_string(init_status), (int32_t)init_status);
        return EXIT_FAILURE;
    }

    printf("Driver Status     : Initialized\n");
    printf("Active Backend    : %s\n\n",
           gpu_driver_get_backend_name(gpu_driver_get_active_backend()));

    if (show_info)
    {
        uint32_t device_count = 0U;
        gpu_status_t count_status = gpu_driver_get_device_count(&device_count);
        if (count_status != GPU_STATUS_OK)
        {
            fprintf(stderr, "Failed to query device count: %s\n",
                    gpu_driver_status_to_string(count_status));
            (void)gpu_driver_shutdown();
            return EXIT_FAILURE;
        }

        printf("Detected %u GPU device(s):\n", device_count);

        for (uint32_t idx = 0U; idx < device_count; ++idx)
        {
            gpu_device_info_t info;
            gpu_status_t info_status = gpu_driver_get_device_info(idx, &info);
            if (info_status != GPU_STATUS_OK)
            {
                fprintf(stderr, "Failed to query GPU index %u: %s\n",
                        idx, gpu_driver_status_to_string(info_status));
                continue;
            }

            char gpu_bar[BAR_WIDTH + 1U];
            char mem_bar[BAR_WIDTH + 1U];
            format_utilization_bar(info.gpu_utilization_pct, gpu_bar);
            format_utilization_bar(info.mem_utilization_pct, mem_bar);

            double gib_total = (double)info.vram_total_bytes / (1024.0 * 1024.0 * 1024.0);
            double gib_used = (double)info.vram_used_bytes / (1024.0 * 1024.0 * 1024.0);
            double gib_free = (double)info.vram_free_bytes / (1024.0 * 1024.0 * 1024.0);

            printf("-----------------------------------------------------------------\n");
            printf("GPU Index         : %u\n", info.device_index);
            printf("Device Model      : %s\n", info.name);
            printf("PCIe Bus ID       : %s\n", info.pcie_bus_id);
            printf("VRAM Total        : %.2f GiB (%" PRIu64 " bytes)\n", gib_total, info.vram_total_bytes);
            printf("VRAM Used         : %.2f GiB (%" PRIu64 " bytes)\n", gib_used, info.vram_used_bytes);
            printf("VRAM Free         : %.2f GiB (%" PRIu64 " bytes)\n", gib_free, info.vram_free_bytes);
            printf("GPU Utilization   : [%s] %3u%%\n", gpu_bar, info.gpu_utilization_pct);
            printf("Mem Utilization   : [%s] %3u%%\n", mem_bar, info.mem_utilization_pct);
            printf("Temperature       : %u °C\n", info.temperature_celsius);
            printf("SM Clock Speed    : %u MHz\n", info.sm_clock_mhz);
            printf("Memory Clock Speed: %u MHz\n", info.mem_clock_mhz);
        }

        if (device_count > 0U)
        {
            printf("-----------------------------------------------------------------\n");
        }
        printf("\n");
    }

    if (run_compute)
    {
        gpu_status_t compute_status = gpu_driver_run_compute_demo();
        if (compute_status != GPU_STATUS_OK)
        {
            fprintf(stderr, "Compute Demo Failed: %s\n",
                    gpu_driver_status_to_string(compute_status));
            (void)gpu_driver_shutdown();
            return EXIT_FAILURE;
        }
        printf("\n");
    }

    (void)gpu_driver_shutdown();
    printf("GPU subsystem shutdown cleanly.\n");

    return EXIT_SUCCESS;
}
