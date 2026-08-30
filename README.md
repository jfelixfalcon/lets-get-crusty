# NVIDIA GPU Communication & Compute Starter in C

A professional, production-ready C starter skeleton designed for interacting with **NVIDIA GPUs**. Built from the ground up following the **BARR-C:2018** (Embedded C Coding Standard) and modern C99/C11 best practices.

---

## 🌟 Features

- **Direct Hardware Interaction**: Seamlessly communicates with host NVIDIA GPUs via dynamic runtime loading of the NVIDIA Management Library (`libnvidia-ml.so`).
- **Zero-Dependency Compilation**: Compiles on any machine out of the box without requiring the heavy NVIDIA CUDA Toolkit / proprietary SDK headers at build time.
- **Deterministic Mock Backend**: Includes an offline simulated GPU backend (`GPU_BACKEND_MOCK`) for rapid testing, CI/CD pipelines, and development on non-GPU environments.
- **Enterprise-Grade Testing**: Vendored [Unity Test Framework](http://www.throwtheswitch.org/unity) with native `ctest` automation and zero external runtime dependencies.
- **BARR-C:2018 Compliance**: Fixed-width integer types (`uint32_t`, `uint64_t`), defensive pointer validation, `const` correctness, strict braces, and deterministic error codes.
- **Modern CMake (3.25+) Build System**: Target-based architecture, `CMakePresets.json` (Release, Debug, Sanitize), CTest integration, strict BARR-C compiler warnings, ASan/UBSan sanitizers, and standard install rules.
- **IDE & Tooling Ready**: Auto-generates `compile_commands.json` for Clangd, CLion, VS Code CMake Tools, and Neovim LSP.
- **Telemetry & Compute**: Real-time inspection of VRAM (Total, Used, Free), GPU utilization, core temperature, PCIe Bus ID, SM/Memory clock frequencies, and a starter GPU compute kernel roundtrip.

---

## 📁 Project Structure

```
.
├── CMakeLists.txt               # Modern target-based CMake configuration (3.25+)
├── CMakePresets.json            # Standardized Release, Debug, and Sanitizer presets
├── .clang-format                # BARR-C:2018 conforming code formatter configuration
├── .gitignore                   # Ignores build artifacts, objects, binaries, and presets
├── README.md                    # Project documentation and architectural guide
│
├── include/                     # Public Header Files
│   ├── gpu_driver.h             # High-level GPU API declarations & Doxygen docstrings
│   ├── gpu_nvml_types.h         # Dynamic NVIDIA NVML definitions & function pointers
│   └── gpu_types.h              # BARR-C data structures, status enums, and constants
│
├── src/                         # Implementation Sources
│   ├── main.c                   # CLI entrypoint with telemetry dashboard & compute demo
│   ├── gpu_driver.c             # Driver core (NVML dynamic loading & hardware bridge)
│   ├── gpu_mock.c               # Hardware simulation backend for offline CI/CD
│   └── gpu_mock.h               # Internal header for mock operations
│
└── tests/                       # Unit Test Suite
    ├── test_gpu_driver.c        # Comprehensive unit tests verifying BARR-C & GPU logic
    └── unity/                   # Vendored Unity Test Framework
        ├── unity.h
        ├── unity.c
        └── unity_internals.h
```

---

## 🚀 Modern CMake Quick Start (2026 Standard)

### Prerequisites
- **Compiler**: `gcc` or `clang` (C99 or newer)
- **Build System**: `cmake` (version 3.25 or newer) & `ctest`
- **Build Tool**: `ninja` or GNU `make`
- **Optional**: NVIDIA GPU Driver installed on host for live hardware telemetry

---

### 1. Using CMake Presets (Recommended)

`CMakePresets.json` standardizes builds across CLion, VS Code, CI/CD, and CLI:

#### Optimized Release Build
```bash
# Configure, build, and test
cmake --preset release
cmake --build --preset release
ctest --preset release

# Run application
./build/release/bin/gpu_app
```

#### Debug Build (with Symbols & Assertions)
```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

#### AddressSanitizer & UndefinedBehaviorSanitizer (ASan / UBSan)
```bash
cmake --preset sanitize
cmake --build --preset sanitize
ctest --preset sanitize
```

---

### 2. Manual CMake Workflow

If you prefer configuring without presets:

```bash
# Configure Release build in ./build directory
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Compile all targets (gpu_driver, gpu_app, test_runner)
cmake --build build

# Run unit tests with CTest
ctest --test-dir build --output-on-failure

# Execute the application
./build/bin/gpu_app
```

---

### 3. CMake Custom Targets

Convenience targets are included directly in `CMakeLists.txt`:

```bash
# Run application directly
cmake --build build/release --target run

# Force mock GPU mode (even if NVIDIA driver is present)
cmake --build build/release --target run-mock

# Run test suite
cmake --build build/release --target check
```

---

## 💻 CLI Usage

```text
Usage: build/bin/gpu_app [OPTIONS]

Options:
  --help, -h       Display usage information and exit.
  --mock, -m       Force mock/simulation mode (simulates RTX 4090 & H100).
  --nvml, -n       Enforce real NVIDIA NVML driver (fails if absent).
  --info, -i       Query and display detailed GPU device telemetry.
  --compute, -c    Run vector addition compute demo on GPU.
```

### Example Live Output
```text
=================================================================
  Initializing GPU Subsystem...
=================================================================
Driver Status     : Initialized
Active Backend    : NVIDIA NVML (Hardware)

Detected 1 GPU device(s):
-----------------------------------------------------------------
GPU Index         : 0
Device Model      : NVIDIA GeForce RTX 4090 Laptop GPU [HARDWARE]
PCIe Bus ID       : 00000000:01:00.0
VRAM Total        : 15.99 GiB (17171480576 bytes)
VRAM Used         : 0.32 GiB (343932928 bytes)
VRAM Free         : 15.67 GiB (16827547648 bytes)
GPU Utilization   : [--------------------]   0%
Mem Utilization   : [--------------------]   0%
Temperature       : 44 °C
SM Clock Speed    : 1455 MHz
Memory Clock Speed: 6001 MHz
-----------------------------------------------------------------

[GPU Compute Demo] Running Vector Addition (C = A + B)...
Vector size: 8 single-precision floats

Compute Results:
  Index |    Vec A    +    Vec B    =   Result (GPU)
  ------+-------------+-------------+---------------
   [0]  |        1.50 +        0.50 =        2.00
   [1]  |        2.50 +        1.50 =        4.00
   [2]  |        3.00 +        7.00 =       10.00
   [3]  |        4.25 +        5.75 =       10.00
   [4]  |        5.00 +        5.00 =       10.00
   [5]  |       10.00 +       90.00 =      100.00
   [6]  |       20.00 +       30.00 =       50.00
   [7]  |      100.00 +      200.00 =      300.00

Vector addition completed and verified successfully.

GPU subsystem shutdown cleanly.
```

---

## 📐 Architecture & GPU Communication Design

Interacting with an NVIDIA GPU from pure C is typically done via one of three tiers:

```
┌─────────────────────────────────────────────────────────────┐
│                    Application (src/main.c)                 │
└──────────────────────────────┬──────────────────────────────┘
                               │
┌──────────────────────────────▼──────────────────────────────┐
│                GPU Driver API (include/gpu_driver.h)        │
└──────────────┬──────────────────────────────┬───────────────┘
               │                              │
┌──────────────▼──────────────┐┌──────────────▼──────────────┐
│  NVML Dynamic Loader (dl)   ││      Deterministic Mock     │
│    (libnvidia-ml.so)        ││     (src/gpu_mock.c)        │
└──────────────┬──────────────┘└─────────────────────────────┘
               │
┌──────────────▼──────────────┐
│   NVIDIA Physical Hardware  │
└─────────────────────────────┘
```

1. **NVML (NVIDIA Management Library)**:
   - Dynamic library `libnvidia-ml.so` shipped with all standard NVIDIA GPU drivers.
   - Provides hardware telemetry (GPU name, PCIe bus ID, VRAM allocation, core clock, temperature, fan speed, power usage, and utilization rates).
   - Our driver resolves these entry points at runtime using `dlopen`/`dlsym`, eliminating hard compile-time dependencies.

2. **CUDA Driver API (`libcuda.so`) / CUDA Runtime (`libcudart.so`)**:
   - For launching raw PTX compute kernels or CUDA C kernels.
   - You can integrate `nvcc` or dynamic `cuLaunchKernel` by extending `gpu_driver_run_compute_demo()`.

3. **Software Simulation / Mock Backend**:
   - `gpu_mock.c` implements the complete device interface with simulated RTX 4090 and H100 profiles, enabling offline development and CI test verification.

---

## 🛡️ BARR-C:2018 Coding Standards Applied

This project adheres strictly to the **BARR-C:2018 (Embedded C Coding Standard)**:

1. **Explicit Fixed-Width Types**:
   - All variables and function signatures use standard `<stdint.h>` types (`uint32_t`, `int32_t`, `uint64_t`, `size_t`, `bool`).
   - Plain `int` or `char` are restricted (using `char` solely for null-terminated strings).

2. **Compound Statements & Braces**:
   - Every `if`, `else`, `while`, `for`, `do`, and `switch` block is enclosed in explicit `{ }` braces.
   - No single-line shortcut statements.

3. **Defensive Pointer Verification**:
   - Every public API function validates pointers before dereferencing, returning `GPU_STATUS_ERROR_NULL_POINTER`.

4. **Const Correctness**:
   - Read-only parameters and arrays are marked `const` (e.g. `const float * const p_vec_a`).

5. **Naming Conventions**:
   - Constants & Macros: `ALL_CAPS_WITH_UNDERSCORES` (e.g. `GPU_MAX_NAME_LEN`).
   - Types: `*_t` suffix (e.g. `gpu_device_info_t`, `gpu_status_t`).
   - Module Static Variables: Prefixed with `s_` (e.g. `s_is_initialized`).
   - Pointer Variables: Prefixed with `p_` (e.g. `p_info`).

6. **Comprehensive Doxygen Docstrings**:
   - Every header file and function is documented with `@file`, `@brief`, `@param[in/out]`, `@return`, and `@note`.

---

## 🧪 Unit Testing with Unity

Unit tests are implemented in `tests/test_gpu_driver.c` using the [Unity Test Framework](https://github.com/ThrowTheSwitch/Unity):

- **Lifecycle testing**: verifies proper initialization, repeated initialization safeguards, and clean teardown.
- **Defensive API testing**: verifies all NULL pointers and boundary violations are rejected safely.
- **Telemetry verification**: validates VRAM memory calculations (`total == used + free`), device count, and temperature checks.
- **Compute validation**: verifies vector arithmetic correctness and output integrity.

Run the test suite at any time:
```bash
# Using Modern CMake Presets (Recommended)
ctest --preset release

# Or directly with CTest in manual build directory
ctest --test-dir build --output-on-failure
```

---

## 📄 License

This project is licensed under the MIT License - feel free to use, modify, and distribute for your own projects.
# lets-get-crusty
