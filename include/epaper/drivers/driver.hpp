#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <string_view>

namespace epaper {

// Display operation errors
enum class DriverError { NotInitialized, InitializationFailed, InvalidMode, TransferFailed, Timeout };

[[nodiscard]] constexpr auto to_string(DriverError error) -> std::string_view {
  switch (error) {
  case DriverError::NotInitialized:
    return "Driver not initialized";
  case DriverError::InitializationFailed:
    return "Driver initialization failed";
  case DriverError::InvalidMode:
    return "Invalid display mode";
  case DriverError::TransferFailed:
    return "Data transfer failed";
  case DriverError::Timeout:
    return "Operation timed out";
  }
  return "Unknown error";
}

// Display modes
enum class DisplayMode {
  BlackWhite, // 1-bit black and white
  Grayscale4  // 2-bit 4-level grayscale
};

// Abstract driver interface for e-paper displays
class Driver {
public:
  virtual ~Driver() = default;

  // Initialize the display with specified mode
  [[nodiscard]] virtual auto init(DisplayMode mode) -> std::expected<void, DriverError> = 0;

  // Clear the display (typically to white)
  virtual auto clear() -> void = 0;

  // Send buffer data to display and refresh
  virtual auto display(std::span<const std::byte> buffer) -> void = 0;

  // Put display into low-power sleep mode
  virtual auto sleep() -> void = 0;

  // Get display dimensions
  [[nodiscard]] virtual auto width() const noexcept -> std::size_t = 0;
  [[nodiscard]] virtual auto height() const noexcept -> std::size_t = 0;

  // Get current display mode
  [[nodiscard]] virtual auto mode() const noexcept -> DisplayMode = 0;

  // Calculate required buffer size for current mode
  [[nodiscard]] virtual auto buffer_size() const noexcept -> std::size_t = 0;

protected:
  Driver() = default;

  // Non-copyable, non-movable (interface class)
  Driver(const Driver &) = delete;
  Driver &operator=(const Driver &) = delete;
  Driver(Driver &&) = delete;
  Driver &operator=(Driver &&) = delete;
};

} // namespace epaper
