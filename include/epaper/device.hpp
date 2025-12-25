#pragma once

#include <bcm2835.h>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <string_view>

namespace epaper {

// Error types for device operations
enum class DeviceError { InitializationFailed, SPIInitFailed, InvalidPin, TransferFailed };

// Convert error to string for debugging
[[nodiscard]] constexpr auto to_string(DeviceError error) -> std::string_view {
  switch (error) {
  case DeviceError::InitializationFailed:
    return "Device initialization failed";
  case DeviceError::SPIInitFailed:
    return "SPI initialization failed";
  case DeviceError::InvalidPin:
    return "Invalid pin number";
  case DeviceError::TransferFailed:
    return "SPI transfer failed";
  }
  return "Unknown error";
}

// Type-safe pin wrapper
class Pin {
public:
  constexpr explicit Pin(std::uint8_t pin_number) : pin_(pin_number) {}

  [[nodiscard]] constexpr auto number() const noexcept -> std::uint8_t { return pin_; }

private:
  std::uint8_t pin_;
};

// Predefined pins for e-paper display
namespace pins {
constexpr Pin RST{17};  // GPIO 17
constexpr Pin DC{25};   // GPIO 25
constexpr Pin CS{8};    // GPIO 8 (CE0)
constexpr Pin BUSY{24}; // GPIO 24
constexpr Pin PWR{18};  // GPIO 18 (optional power control)
} // namespace pins

// RAII wrapper for BCM2835 device
class Device {
public:
  Device() = default;
  ~Device() noexcept;

  // Non-copyable, movable
  Device(const Device &) = delete;
  Device &operator=(const Device &) = delete;
  Device(Device &&other) noexcept;
  Device &operator=(Device &&other) noexcept;

  // Initialize the BCM2835 library and SPI
  [[nodiscard]] auto init() -> std::expected<void, DeviceError>;

  // Check if device is initialized
  [[nodiscard]] auto is_initialized() const noexcept -> bool { return initialized_; }

  // GPIO operations
  auto set_pin_output(Pin pin) -> void;
  auto set_pin_input(Pin pin) -> void;
  auto write_pin(Pin pin, bool value) -> void;
  [[nodiscard]] auto read_pin(Pin pin) -> bool;

  // SPI operations
  auto spi_transfer(std::uint8_t value) -> std::uint8_t;
  auto spi_write(std::span<const std::byte> data) -> void;

  // Delay utilities
  static auto delay_ms(std::uint32_t milliseconds) -> void;
  static auto delay_us(std::uint32_t microseconds) -> void;

private:
  bool initialized_ = false;
  bool spi_initialized_ = false;
};

} // namespace epaper
