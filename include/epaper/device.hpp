#pragma once

#include "epaper/errors.hpp"
#include <bcm2835.h>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <string_view>

namespace epaper {

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

/**
 * @brief RAII wrapper for BCM2835 device.
 *
 * Manages the lifecycle of the BCM2835 library and SPI interface.
 * Follows the RAII pattern with automatic resource cleanup.
 *
 * @note Exception Safety: Strong guarantee for all operations.
 *       Destructor is noexcept and will never throw.
 *       Move operations are noexcept.
 */
class Device {
public:
  Device() = default;

  /**
   * @brief Destructor. Cleans up BCM2835 and SPI resources.
   *
   * @note Exception Safety: Nothrow guarantee - never throws exceptions.
   */
  ~Device() noexcept;

  // Non-copyable, movable
  Device(const Device &) = delete;
  Device &operator=(const Device &) = delete;

  /**
   * @brief Move constructor.
   *
   * @param other Device to move from
   * @note Exception Safety: Nothrow guarantee.
   */
  Device(Device &&other) noexcept;

  /**
   * @brief Move assignment operator.
   *
   * @param other Device to move from
   * @return Reference to this device
   * @note Exception Safety: Nothrow guarantee.
   */
  Device &operator=(Device &&other) noexcept;

  /**
   * @brief Initialize the BCM2835 library and SPI.
   *
   * This must be called before any other operations on the device.
   * Idempotent - calling init() on an already initialized device succeeds.
   *
   * @return void on success, Error on failure
   * @note Exception Safety: Strong guarantee - if initialization fails,
   *       the device remains in an uninitialized state.
   */
  [[nodiscard]] auto init() -> std::expected<void, Error>;

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
