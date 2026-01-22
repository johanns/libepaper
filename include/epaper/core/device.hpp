#pragma once

#include "epaper/core/errors.hpp"
#include "epaper/hal/gpio.hpp"
#include "epaper/hal/spi.hpp"
#include <cstdint>
#include <expected>
#include <gpiod.h>
#include <span>
#include <string>
#include <unordered_map>

namespace epaper {

/**
 * @brief Type-safe GPIO pin identifier.
 *
 * Wraps a GPIO pin number to provide type safety and prevent
 * accidental use of raw integers as pin numbers. Forces explicit
 * construction to avoid implicit conversions.
 *
 * @note Pin numbers are device-specific (e.g., BCM numbering on Raspberry Pi).
 *       Consult your hardware documentation for pin mappings.
 *
 * @example
 * ```cpp
 * // Explicit pin construction (compile-time checked)
 * Pin reset_pin{17};
 * Pin data_cmd_pin{25};
 *
 * // Type safety prevents accidental mixing of pins and values
 * device.write_pin(reset_pin, true);  // OK
 * // device.write_pin(17, true);      // Compile error - requires Pin
 * ```
 *
 * @see Device::set_pin_output(), Device::write_pin()
 */
class Pin {
public:
  constexpr explicit Pin(std::uint8_t pin_number) : pin_(pin_number) {}

  [[nodiscard]] constexpr auto number() const noexcept -> std::uint8_t { return pin_; }

private:
  std::uint8_t pin_;
};

/**
 * @brief Configuration for device initialization.
 *
 * Allows customization of GPIO chip path, SPI device path, and SPI speed.
 * Provides default values matching typical Raspberry Pi configuration.
 *
 * @note Default SPI speed (1953125 Hz) is conservative and reliable.
 *       Some displays support higher speeds (up to 4-10 MHz) but this
 *       may require signal integrity verification.
 *
 * @example
 * ```cpp
 * // Use default configuration
 * Device device{};
 * auto result = device.init();
 * if (!result) {
 *   std::cerr << "Init failed: " << result.error().what() << std::endl;
 * }
 *
 * // Custom configuration for different hardware
 * DeviceConfig config{
 *     .gpio_chip = "/dev/gpiochip1",     // Alternative GPIO chip
 *     .spi_device = "/dev/spidev1.0",    // SPI bus 1, CS 0
 *     .spi_speed_hz = 4000000             // 4 MHz (verify with scope)
 * };
 * Device custom_device{config};
 * ```
 *
 * @see Device::init(), Device::Device(DeviceConfig)
 */
struct DeviceConfig {
  std::string gpio_chip = "/dev/gpiochip0";  ///< Path to GPIO chip device
  std::string spi_device = "/dev/spidev0.0"; ///< Path to SPI device
  std::uint32_t spi_speed_hz = 1953125;      ///< SPI clock speed in Hz (~1.95MHz)
};

/**
 * @brief Pin configuration tracking (internal use).
 *
 * Stores per-pin state for reconstructing libgpiod line requests.
 * This is an implementation detail of the dynamic pin configuration
 * system - users do not interact with PinConfig directly.
 *
 * @note libgpiod v2 requires all pins to be requested in a single batch.
 *       PinConfig enables rebuilding requests when pins are added.
 */
struct PinConfig {
  unsigned int offset;
  bool is_output;
  enum gpiod_line_value initial_value;
};

/**
 * @brief RAII wrapper for GPIO and SPI device access.
 *
 * Manages the lifecycle of libgpiod (GPIO) and Linux SPIdev (SPI) interfaces.
 * Follows the RAII pattern with automatic resource cleanup on destruction.
 *
 * **Resource Management:**
 * - Acquires GPIO chip and SPI device file descriptors on init()
 * - Releases all resources in destructor (exception-safe)
 * - Move semantics transfer ownership without resource duplication
 *
 * **Thread Safety:** Not thread-safe. Synchronize external access if
 * using from multiple threads.
 *
 * @note Exception Safety: Strong guarantee for all operations.
 *       Destructor is noexcept and will never throw.
 *       Move operations are noexcept.
 *
 * @note GPIO operations can be used for button handling:
 *       ```cpp
 *       device.set_pin_input(button_pin);
 *       bool pressed = device.read_pin(button_pin);
 *       ```
 *
 * @example
 * ```cpp
 * // Basic usage with error checking
 * Device device{};
 * if (auto result = device.init(); !result) {
 *   return std::unexpected(result.error());
 * }
 *
 * // Configure pins for e-paper display
 * Pin dc_pin{25};   // Data/Command
 * Pin rst_pin{17};  // Reset
 * Pin busy_pin{24}; // Busy status
 *
 * device.set_pin_output(dc_pin);
 * device.set_pin_output(rst_pin);
 * device.set_pin_input(busy_pin);
 *
 * // Reset sequence
 * device.write_pin(rst_pin, false);
 * Device::delay_ms(10);
 * device.write_pin(rst_pin, true);
 *
 * // Wait for display ready
 * while (device.read_pin(busy_pin)) {
 *   Device::delay_ms(1);
 * }
 *
 * // SPI communication
 * device.write_pin(dc_pin, false);  // Command mode
 * device.spi_transfer(0x12);        // Send command
 * ```
 *
 * @see Pin, DeviceConfig, Error
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
  explicit Device(DeviceConfig config);

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
  auto spi_transfer(std::uint8_t value) const -> std::uint8_t;
  auto spi_write(std::span<const std::byte> data) const -> void;

  // Delay utilities
  static auto delay_ms(std::uint32_t milliseconds) -> void;
  static auto delay_us(std::uint32_t microseconds) -> void;

  struct Delay {
    static auto delay_ms(std::uint32_t ms) -> void { Device::delay_ms(ms); }
  };

  // -- HAL Adapters --

  class HalOutput {
  public:
    HalOutput(Device &dev, Pin pin) : dev_(&dev), pin_(pin) {}
    auto write(bool level) -> void { dev_->write_pin(pin_, level); }

  private:
    Device *dev_;
    Pin pin_;
  };
  static_assert(hal::DigitalOutput<HalOutput>);

  class HalInput {
  public:
    HalInput(Device &dev, Pin pin) : dev_(&dev), pin_(pin) {}
    auto read() -> bool { return dev_->read_pin(pin_); }

  private:
    Device *dev_;
    Pin pin_;
  };
  static_assert(hal::DigitalInput<HalInput>);

  class HalSpi {
  public:
    explicit HalSpi(Device &dev) : dev_(&dev) {}
    auto transfer(std::uint8_t byte) -> std::uint8_t { return dev_->spi_transfer(byte); }
    auto write(std::span<const std::byte> data) -> void { dev_->spi_write(data); }

  private:
    Device *dev_;
  };
  static_assert(hal::SpiBus<HalSpi>);

  // -- HAL Factory Methods --

  [[nodiscard]] auto get_output(Pin pin) -> HalOutput {
    set_pin_output(pin);
    return HalOutput(*this, pin);
  }

  [[nodiscard]] auto get_input(Pin pin) -> HalInput {
    set_pin_input(pin);
    return HalInput(*this, pin);
  }

  [[nodiscard]] auto get_spi() -> HalSpi { return HalSpi(*this); }

private:
  auto rebuild_line_request() -> bool;
  auto cleanup() noexcept -> void;

  struct gpiod_chip *chip_ = nullptr;
  struct gpiod_line_request *line_request_ = nullptr;
  std::unordered_map<std::uint8_t, PinConfig> pin_configs_;
  int spi_fd_ = -1;
  bool initialized_ = false;
  bool spi_initialized_ = false;
  DeviceConfig config_;
};

} // namespace epaper
