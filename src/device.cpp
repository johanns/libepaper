#include "epaper/device.hpp"
#include <algorithm>
#include <thread>

namespace epaper {

Device::~Device() noexcept {
  if (spi_initialized_) {
    bcm2835_spi_end();
  }
  if (initialized_) {
    bcm2835_close();
  }
}

Device::Device(Device &&other) noexcept : initialized_(other.initialized_), spi_initialized_(other.spi_initialized_) {
  other.initialized_ = false;
  other.spi_initialized_ = false;
}

Device &Device::operator=(Device &&other) noexcept {
  if (this != &other) {
    // Clean up current resources
    if (spi_initialized_) {
      bcm2835_spi_end();
    }
    if (initialized_) {
      bcm2835_close();
    }

    // Transfer ownership
    initialized_ = other.initialized_;
    spi_initialized_ = other.spi_initialized_;
    other.initialized_ = false;
    other.spi_initialized_ = false;
  }
  return *this;
}

auto Device::init() -> std::expected<void, DeviceError> {
  if (initialized_) {
    return {};
  }

  // Initialize BCM2835 library
  if (bcm2835_init() == 0) {
    return std::unexpected(DeviceError::InitializationFailed);
  }
  initialized_ = true;

  // Initialize SPI
  if (bcm2835_spi_begin() == 0) {
    bcm2835_close();
    initialized_ = false;
    return std::unexpected(DeviceError::SPIInitFailed);
  }
  spi_initialized_ = true;

  // Configure SPI parameters
  bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
  bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
  bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_128);

  // Set up default pins for e-paper
  set_pin_output(pins::RST);
  set_pin_output(pins::DC);
  set_pin_output(pins::CS);
  set_pin_input(pins::BUSY);

  return {};
}

auto Device::set_pin_output(Pin pin) -> void { bcm2835_gpio_fsel(pin.number(), BCM2835_GPIO_FSEL_OUTP); }

auto Device::set_pin_input(Pin pin) -> void { bcm2835_gpio_fsel(pin.number(), BCM2835_GPIO_FSEL_INPT); }

auto Device::write_pin(Pin pin, bool value) -> void { bcm2835_gpio_write(pin.number(), value ? HIGH : LOW); }

auto Device::read_pin(Pin pin) -> bool { return bcm2835_gpio_lev(pin.number()) == HIGH; }

auto Device::spi_transfer(std::uint8_t value) -> std::uint8_t { return bcm2835_spi_transfer(value); }

auto Device::spi_write(std::span<const std::byte> data) -> void {
  // BCM2835 library expects char*, so we need to cast
  const auto *ptr = reinterpret_cast<const char *>(data.data());
  for (const auto byte : data) {
    bcm2835_spi_transfer(static_cast<std::uint8_t>(byte));
  }
}

auto Device::delay_ms(std::uint32_t milliseconds) -> void { bcm2835_delay(milliseconds); }

auto Device::delay_us(std::uint32_t microseconds) -> void { bcm2835_delayMicroseconds(microseconds); }

} // namespace epaper
