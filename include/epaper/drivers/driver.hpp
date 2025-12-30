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

  // Wake display from sleep mode
  // Returns error if wake is not supported or fails
  [[nodiscard]] virtual auto wake() -> std::expected<void, DriverError> = 0;

  // Turn display power completely off (hardware power down)
  // Returns error if power off is not supported or fails
  [[nodiscard]] virtual auto power_off() -> std::expected<void, DriverError> = 0;

  // Turn display power on (hardware power up)
  // Returns error if power on is not supported or fails
  [[nodiscard]] virtual auto power_on() -> std::expected<void, DriverError> = 0;

  // Get display dimensions
  [[nodiscard]] virtual auto width() const noexcept -> std::size_t = 0;
  [[nodiscard]] virtual auto height() const noexcept -> std::size_t = 0;

  // Get current display mode
  [[nodiscard]] virtual auto mode() const noexcept -> DisplayMode = 0;

  // Calculate required buffer size for current mode
  [[nodiscard]] virtual auto buffer_size() const noexcept -> std::size_t = 0;

  // Driver capabilities (query at runtime)
  [[nodiscard]] virtual auto supports_partial_refresh() const noexcept -> bool = 0;
  [[nodiscard]] virtual auto supports_wake() const noexcept -> bool = 0;
  [[nodiscard]] virtual auto supports_power_control() const noexcept -> bool = 0;

protected:
  Driver() = default;

  // Non-copyable, movable
  Driver(const Driver &) = delete;
  Driver &operator=(const Driver &) = delete;
  Driver(Driver &&) noexcept = default;
  Driver &operator=(Driver &&) noexcept = default;
};

} // namespace epaper
