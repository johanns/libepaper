#pragma once

#include "epaper/errors.hpp"
#include <cstdint>
#include <expected>
#include <memory>
#include <span>
#include <string>

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
 * @brief Configuration for device initialization.
 *
 * Allows customization of GPIO chip path, SPI device path, and SPI speed.
 * Provides default values matching typical Raspberry Pi configuration.
 *
 * @example
 * @code{.cpp}
 * // Use default configuration
 * DeviceConfig config{};
 * Device device{config};
 *
 * // Or customize
 * DeviceConfig config{
 *     .gpio_chip = "/dev/gpiochip0",
 *     .spi_device = "/dev/spidev0.0",
 *     .spi_speed_hz = 4000000
 * };
 * Device device{config};
 * @endcode
 */
struct DeviceConfig {
  std::string gpio_chip = "/dev/gpiochip0";  ///< Path to GPIO chip device
  std::string spi_device = "/dev/spidev0.0"; ///< Path to SPI device
  std::uint32_t spi_speed_hz = 1953125;      ///< SPI clock speed in Hz (~1.95MHz)
};

// Forward declaration for PImpl
struct DeviceImpl;

/**
 * @brief RAII wrapper for GPIO and SPI device access.
 *
 * Manages the lifecycle of libgpiod (GPIO) and Linux SPIdev (SPI) interfaces.
 * Follows the RAII pattern with automatic resource cleanup.
 *
 * @note Exception Safety: Strong guarantee for all operations.
 *       Destructor is noexcept and will never throw.
 *       Move operations are noexcept.
 *
 * @note GPIO operations can be used for button handling:
 *       @code{.cpp}
 *       device.set_pin_input(button_pin);
 *       bool pressed = device.read_pin(button_pin);
 *       @endcode
 */
class Device {
public:
  /**
   * @brief Construct a Device with default configuration.
   *
   * Uses default DeviceConfig values:
   * - GPIO chip: "/dev/gpiochip0"
   * - SPI device: "/dev/spidev0.0"
   * - SPI speed: 1953125 Hz (~1.95MHz)
   */
  Device();

  /**
   * @brief Construct a Device with custom configuration.
   *
   * @param config Device configuration specifying paths and settings
   */
  explicit Device(const DeviceConfig &config);

  /**
   * @brief Destructor. Cleans up GPIO and SPI resources.
   *
   * @note Exception Safety: Nothrow guarantee - never throws exceptions.
   */
  ~Device() noexcept;

  // Non-copyable, movable
  Device(const Device &) = delete;
  auto operator=(const Device &) -> Device & = delete;

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
  auto operator=(Device &&other) noexcept -> Device &;

  /**
   * @brief Initialize the GPIO (libgpiod) and SPI (SPIdev) interfaces.
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
  [[nodiscard]] auto is_initialized() const noexcept -> bool;

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
  std::unique_ptr<DeviceImpl> pimpl_;
};

} // namespace epaper
