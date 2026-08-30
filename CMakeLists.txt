cmake_minimum_required(VERSION 3.25)
project(gpu_starter
    VERSION 1.0.0
    DESCRIPTION "NVIDIA GPU Communication & Compute Starter in C"
    LANGUAGES C
)

# Standard C11 configuration
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Export compile commands for IDEs (Clangd, CLion, VS Code)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Default output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# BARR-C:2018 Compiler Warnings
if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wshadow
        -Wstrict-prototypes
        -Wundef
        -Wpointer-arith
        -Wcast-qual
        -Wmissing-prototypes
    )
endif()

# Core GPU Driver Library Target
add_library(gpu_driver STATIC
    src/gpu_driver.c
    src/gpu_mock.c
)

target_include_directories(gpu_driver
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(gpu_driver
    PRIVATE
        ${CMAKE_DL_LIBS}
        m
)

# CLI Dashboard Application Target
add_executable(gpu_app
    src/main.c
)

target_link_libraries(gpu_app
    PRIVATE
        gpu_driver
        m
)

# Testing Support with Unity and CTest
include(CTest)
if(BUILD_TESTING)
    add_library(unity STATIC
        tests/unity/unity.c
    )

    target_include_directories(unity
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/unity
    )

    target_link_libraries(unity
        PRIVATE
            m
    )

    if(NOT MSVC)
        target_compile_options(unity PRIVATE -Wno-missing-prototypes -Wno-conversion)
    endif()

    add_executable(test_gpu_driver
        tests/test_gpu_driver.c
    )

    target_include_directories(test_gpu_driver
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )

    target_link_libraries(test_gpu_driver
        PRIVATE
            gpu_driver
            unity
            m
    )

    add_test(NAME test_gpu_driver COMMAND test_gpu_driver)
endif()

# Convenience Custom Targets
add_custom_target(run
    COMMAND $<TARGET_FILE:gpu_app>
    DEPENDS gpu_app
    COMMENT "Executing GPU Application..."
)

add_custom_target(run-mock
    COMMAND $<TARGET_FILE:gpu_app> --mock
    DEPENDS gpu_app
    COMMENT "Executing GPU Application in Mock Mode..."
)

if(BUILD_TESTING)
    add_custom_target(check
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS test_gpu_driver
        COMMENT "Executing Unit Test Suite..."
    )
endif()

# Installation Rules
include(GNUInstallDirs)
install(TARGETS gpu_driver gpu_app
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
