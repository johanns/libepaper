#pragma once

#include <string>
#include <string_view>

namespace epaper {

/**
 * @brief Unified error codes for all libepaper operations.
 *
 * This enum consolidates all error types across the library,
 * providing a consistent error handling interface.
 */
enum class ErrorCode {
  // Device errors
  DeviceNotInitialized, ///< Device has not been initialized
  DeviceInitFailed,     ///< Device initialization failed
  GPIOInitFailed,       ///< GPIO initialization failed (libgpiod)
  GPIORequestFailed,    ///< GPIO line request failed
  SPIInitFailed,        ///< SPI initialization failed
  SPIDeviceOpenFailed,  ///< SPI device open failed
  SPIConfigFailed,      ///< SPI configuration failed
  InvalidPin,           ///< Invalid GPIO pin number
  TransferFailed,       ///< SPI/data transfer failed

  // Driver errors
  DriverNotInitialized, ///< Driver has not been initialized
  DriverInitFailed,     ///< Driver initialization failed
  InvalidMode,          ///< Invalid display mode specified
  Timeout,              ///< Operation timed out

  // Display errors
  DisplayNotReady, ///< Display is not ready for operation
  RefreshFailed,   ///< Display refresh operation failed

  // Bitmap errors
  FileNotFound,     ///< Image file not found
  InvalidFormat,    ///< Invalid or unsupported image format
  LoadFailed,       ///< Failed to load image data
  InvalidDimensions ///< Invalid image dimensions
};

/**
 * @brief Convert error code to string representation.
 *
 * @param code Error code to convert
 * @return String view describing the error
 */
[[nodiscard]] constexpr auto to_string(ErrorCode code) -> std::string_view {
  switch (code) {
  // Device errors
  case ErrorCode::DeviceNotInitialized:
    return "Device not initialized";
  case ErrorCode::DeviceInitFailed:
    return "Device initialization failed";
  case ErrorCode::GPIOInitFailed:
    return "GPIO initialization failed";
  case ErrorCode::GPIORequestFailed:
    return "GPIO line request failed";
  case ErrorCode::SPIInitFailed:
    return "SPI initialization failed";
  case ErrorCode::SPIDeviceOpenFailed:
    return "SPI device open failed";
  case ErrorCode::SPIConfigFailed:
    return "SPI configuration failed";
  case ErrorCode::InvalidPin:
    return "Invalid pin number";
  case ErrorCode::TransferFailed:
    return "Data transfer failed";

  // Driver errors
  case ErrorCode::DriverNotInitialized:
    return "Driver not initialized";
  case ErrorCode::DriverInitFailed:
    return "Driver initialization failed";
  case ErrorCode::InvalidMode:
    return "Invalid display mode";
  case ErrorCode::Timeout:
    return "Operation timed out";

  // Display errors
  case ErrorCode::DisplayNotReady:
    return "Display not ready";
  case ErrorCode::RefreshFailed:
    return "Display refresh failed";

  // Bitmap errors
  case ErrorCode::FileNotFound:
    return "File not found";
  case ErrorCode::InvalidFormat:
    return "Invalid format";
  case ErrorCode::LoadFailed:
    return "Load failed";
  case ErrorCode::InvalidDimensions:
    return "Invalid dimensions";
  }
  return "Unknown error";
}

/**
 * @brief Unified error type containing code and optional message.
 *
 * This struct provides a consistent error representation across the library,
 * combining an error code with an optional detailed message.
 */
struct Error {
  ErrorCode code;      ///< Error code
  std::string message; ///< Optional detailed error message

  /**
   * @brief Construct error with code only.
   *
   * @param c Error code
   */
  constexpr Error(ErrorCode c) : code(c), message{} {}

  /**
   * @brief Construct error with code and message.
   *
   * @param c Error code
   * @param msg Detailed error message
   */
  Error(ErrorCode c, std::string msg) : code(c), message(std::move(msg)) {}

  /**
   * @brief Get string representation of the error.
   *
   * Returns the detailed message if available, otherwise returns
   * the string representation of the error code.
   *
   * @return String describing the error
   */
  [[nodiscard]] auto what() const -> std::string_view {
    if (!message.empty()) {
      return message;
    }
    return to_string(code);
  }
};

/**
 * @brief Create an error with optional context.
 *
 * Convenience function to create errors with additional context information.
 *
 * @param code Error code
 * @param context Optional context string to append to error message
 * @return Error object with combined message
 */
[[nodiscard]] inline auto make_error(ErrorCode code, std::string_view context = {}) -> Error {
  if (context.empty()) {
    return Error(code);
  }
  return Error(code, std::string(to_string(code)) + ": " + std::string(context));
}

} // namespace epaper
