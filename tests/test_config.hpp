#pragma once

/**
 * @file test_config.hpp
 * @brief Compile-time driver selection for tests
 *
 * This header provides a TestDriver type alias that resolves to the
 * driver specified via the TEST_DRIVER CMake define.
 *
 * Usage in CMake:
 *   -DTEST_DRIVER=EPD27    # Use real EPD27 hardware
 *   -DTEST_DRIVER=Mock     # Use MockDriver (no hardware)
 *
 * Usage in tests:
 *   #include "test_config.hpp"
 *   TestDriver driver{device};  // Resolves to correct driver
 */

#include "epaper/drivers/epd27.hpp"
#include "epaper/drivers/mock_driver.hpp"

namespace epaper {

// Stringify macro helper
#define STRINGIFY(x) #x
#define TO_STRING(x) STRINGIFY(x)

// Define unique values for driver selection
#define DRIVER_EPD27 1
#define DRIVER_MOCK 2

#if TEST_DRIVER == DRIVER_MOCK
using TestDriver = MockDriver;
constexpr auto TEST_DRIVER_NAME = "MockDriver";
#else
// Default to EPD27 (Linux binding)
using TestDriver = EPD27;
constexpr auto TEST_DRIVER_NAME = "EPD27";
#endif

} // namespace epaper

// Default to EPD27 if TEST_DRIVER not defined
#ifndef TEST_DRIVER
#define TEST_DRIVER DRIVER_EPD27
#endif
