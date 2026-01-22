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
 * combining an error code with an optional detailed message. Designed for
 * use with std::expected<T, Error> return types.
 *
 * @note The what() method returns the detailed message if available,
 *       otherwise returns the error code's string representation.
 *
 * @example
 * ```cpp
 * // Function returning std::expected
 * auto initialize_display() -> std::expected<Display, Error> {
 *   Device device{};
 *   if (auto result = device.init(); !result) {
 *     // Forward error with context
 *     return std::unexpected(result.error());
 *   }
 *   // ... create display
 * }
 *
 * // Error handling at call site
 * auto display_result = initialize_display();
 * if (!display_result) {
 *   std::cerr << "Initialization failed: "
 *             << display_result.error().what() << std::endl;
 *   return EXIT_FAILURE;
 * }
 * auto display = std::move(*display_result);
 *
 * // Explicit error creation
 * if (width == 0) {
 *   return std::unexpected(Error(ErrorCode::InvalidDimensions,
 *                                "Width cannot be zero"));
 * }
 * ```
 *
 * @see ErrorCode, make_error(), std::expected
 */
struct Error {
  ErrorCode code;      ///< Error code
  std::string message; ///< Optional detailed error message

  /**
   * @brief Construct error with code only.
   *
   * @param c Error code
   */
  constexpr Error(ErrorCode c) : code(c) {}

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
 * Automatically appends context to the error code's string representation.
 *
 * @param code Error code identifying the error type
 * @param context Optional context string to append (e.g., filename, pin number)
 * @return Error object with combined message
 *
 * @example
 * ```cpp
 * // Simple error without context
 * return std::unexpected(make_error(ErrorCode::DeviceNotInitialized));
 *
 * // Error with contextual information
 * if (pin_number > 27) {
 *   return std::unexpected(make_error(ErrorCode::InvalidPin,
 *                                     "Pin " + std::to_string(pin_number)));
 * }
 *
 * // File operation error with filename
 * if (!file_exists) {
 *   return std::unexpected(make_error(ErrorCode::FileNotFound, filename));
 * }
 * ```
 *
 * @see Error, ErrorCode
 */
[[nodiscard]] inline auto make_error(ErrorCode code, std::string_view context = {}) -> Error {
  if (context.empty()) {
    return {code};
  }
  return {code, std::string(to_string(code)) + ": " + std::string(context)};
}

} // namespace epaper
