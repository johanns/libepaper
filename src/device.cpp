#include "epaper/device.hpp"

#include <chrono>
#include <fcntl.h>
#include <gpiod.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <vector>

namespace epaper {

// SPI configuration constants
namespace {
constexpr std::uint32_t SPI_SPEED_HZ = 1953125;         // ~1.95MHz (250MHz / 128)
constexpr std::uint8_t SPI_BITS_PER_WORD = 8;           // 8 bits per transfer, MSB first
constexpr std::uint32_t SPI_SINGLE_TRANSFER_LENGTH = 1; // Length for single byte transfers
constexpr int INVALID_FILE_DESCRIPTOR = -1;             // Invalid file descriptor value
} // namespace

// Pin configuration tracking
struct PinConfig {
  unsigned int offset;
  bool is_output;
  enum gpiod_line_value initial_value;
};

// PImpl structure to hide implementation details
struct DeviceImpl {
  struct gpiod_chip *chip = nullptr;
  struct gpiod_line_request *line_request = nullptr;
  std::unordered_map<std::uint8_t, PinConfig> pin_configs; // Track all configured pins
  int spi_fd = INVALID_FILE_DESCRIPTOR;
  bool initialized = false;
  bool spi_initialized = false;

  // Delete copy operations (can't safely copy file descriptors and GPIO resources)
  DeviceImpl(const DeviceImpl &) = delete;
  auto operator=(const DeviceImpl &) -> DeviceImpl & = delete;

  // Default move operations
  DeviceImpl(DeviceImpl &&) = default;
  auto operator=(DeviceImpl &&) -> DeviceImpl & = default;

  DeviceImpl() = default;

  // Rebuild line request with all configured pins
  auto rebuild_line_request() -> bool {
    if (chip == nullptr || pin_configs.empty()) {
      return false;
    }

    // Release old request if exists
    if (line_request != nullptr) {
      gpiod_line_request_release(line_request);
      line_request = nullptr;
    }

    // Create line config with all pins
    struct gpiod_line_config *line_cfg = gpiod_line_config_new();
    if (line_cfg == nullptr) {
      return false;
    }

    // Add all pins to the config (create settings for each pin)
    for (const auto &[pin_num, config] : pin_configs) {
      struct gpiod_line_settings *settings = gpiod_line_settings_new();
      if (settings == nullptr) {
        gpiod_line_config_free(line_cfg);
        return false;
      }

      if (config.is_output) {
        gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);
        gpiod_line_settings_set_output_value(settings, config.initial_value);
      } else {
        gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_INPUT);
      }

      unsigned int offset = config.offset;
      if (gpiod_line_config_add_line_settings(line_cfg, &offset, 1, settings) < 0) {
        gpiod_line_settings_free(settings);
        gpiod_line_config_free(line_cfg);
        return false;
      }
      // Settings are copied into config, so we can free them
      gpiod_line_settings_free(settings);
    }

    // Create request config
    struct gpiod_request_config *req_cfg = gpiod_request_config_new();
    if (req_cfg == nullptr) {
      gpiod_line_config_free(line_cfg);
      return false;
    }
    gpiod_request_config_set_consumer(req_cfg, "libepaper");

    // Request all lines
    line_request = gpiod_chip_request_lines(chip, req_cfg, line_cfg);

    // Clean up temporary objects
    gpiod_request_config_free(req_cfg);
    gpiod_line_config_free(line_cfg);

    return line_request != nullptr;
  }

  ~DeviceImpl() { cleanup(); }

  void cleanup() noexcept {
    // Release line request
    if (line_request != nullptr) {
      gpiod_line_request_release(line_request);
      line_request = nullptr;
    }
    pin_configs.clear();

    // Close GPIO chip
    if (chip != nullptr) {
      gpiod_chip_close(chip);
      chip = nullptr;
    }

    // Close SPI device
    if (spi_fd >= 0) {
      ::close(spi_fd);
      spi_fd = INVALID_FILE_DESCRIPTOR;
    }

    initialized = false;
    spi_initialized = false;
  }
};

Device::Device() : pimpl_(std::make_unique<DeviceImpl>()) {}

Device::~Device() noexcept = default;

Device::Device(Device &&other) noexcept : pimpl_(std::move(other.pimpl_)) {
  other.pimpl_ = std::make_unique<DeviceImpl>();
}

auto Device::operator=(Device &&other) noexcept -> Device & {
  if (this != &other) {
    pimpl_ = std::move(other.pimpl_);
    other.pimpl_ = std::make_unique<DeviceImpl>();
  }
  return *this;
}

auto Device::init() -> std::expected<void, Error> {
  if (pimpl_->initialized) {
    return {};
  }

  // Initialize GPIO chip (libgpiod v2)
  pimpl_->chip = gpiod_chip_open("/dev/gpiochip0");
  if (pimpl_->chip == nullptr) {
    return std::unexpected(Error(ErrorCode::GPIOInitFailed, "Failed to open /dev/gpiochip0"));
  }
  pimpl_->initialized = true;

  // Initialize SPI device (Linux SPIdev)
  pimpl_->spi_fd = open("/dev/spidev0.0", O_RDWR);
  if (pimpl_->spi_fd < 0) {
    pimpl_->cleanup();
    return std::unexpected(Error(ErrorCode::SPIDeviceOpenFailed, "Failed to open /dev/spidev0.0"));
  }

  // Configure SPI mode (SPI_MODE_0: CPOL=0, CPHA=0)
  std::uint8_t mode = SPI_MODE_0;
  if (ioctl(pimpl_->spi_fd, SPI_IOC_WR_MODE, &mode) < 0) {
    pimpl_->cleanup();
    return std::unexpected(Error(ErrorCode::SPIConfigFailed, "Failed to set SPI mode"));
  }

  // Configure SPI speed
  if (ioctl(pimpl_->spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &SPI_SPEED_HZ) < 0) {
    pimpl_->cleanup();
    return std::unexpected(Error(ErrorCode::SPIConfigFailed, "Failed to set SPI speed"));
  }

  // Configure bits per word
  if (ioctl(pimpl_->spi_fd, SPI_IOC_WR_BITS_PER_WORD, &SPI_BITS_PER_WORD) < 0) {
    pimpl_->cleanup();
    return std::unexpected(Error(ErrorCode::SPIConfigFailed, "Failed to set SPI bits per word"));
  }

  pimpl_->spi_initialized = true;

  // Set up default pins for e-paper
  set_pin_output(pins::RST);
  set_pin_output(pins::DC);
  set_pin_output(pins::CS);
  set_pin_input(pins::BUSY);

  return {};
}

auto Device::is_initialized() const noexcept -> bool { return pimpl_ && pimpl_->initialized; }

auto Device::set_pin_output(Pin pin) -> void {
  if (!pimpl_->initialized || pimpl_->chip == nullptr) {
    return;
  }

  const auto pin_num = pin.number();
  const auto offset = static_cast<unsigned int>(pin_num);

  // Update pin configuration
  PinConfig config{};
  config.offset = offset;
  config.is_output = true;
  config.initial_value = GPIOD_LINE_VALUE_INACTIVE;
  pimpl_->pin_configs[pin_num] = config;

  // Rebuild line request with all pins
  if (!pimpl_->rebuild_line_request()) {
    // If rebuild fails, remove the pin we just added to maintain consistency
    pimpl_->pin_configs.erase(pin_num);
  }
}

auto Device::set_pin_input(Pin pin) -> void {
  if (!pimpl_->initialized || pimpl_->chip == nullptr) {
    return;
  }

  const auto pin_num = pin.number();
  const auto offset = static_cast<unsigned int>(pin_num);

  // Update pin configuration
  PinConfig config{};
  config.offset = offset;
  config.is_output = false;
  config.initial_value = GPIOD_LINE_VALUE_INACTIVE;
  pimpl_->pin_configs[pin_num] = config;

  // Rebuild line request with all pins
  if (!pimpl_->rebuild_line_request()) {
    // If rebuild fails, remove the pin we just added to maintain consistency
    pimpl_->pin_configs.erase(pin_num);
  }
}

auto Device::write_pin(Pin pin, bool value) -> void {
  if (!pimpl_->initialized || pimpl_->line_request == nullptr) {
    return;
  }

  const auto pin_num = pin.number();
  if (auto it = pimpl_->pin_configs.find(pin_num); it != pimpl_->pin_configs.end()) {
    enum gpiod_line_value line_value = value ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE;
    gpiod_line_request_set_value(pimpl_->line_request, it->second.offset, line_value);
  }
}

auto Device::read_pin(Pin pin) -> bool {
  if (!pimpl_->initialized || pimpl_->line_request == nullptr) {
    return false;
  }

  const auto pin_num = pin.number();
  if (auto it = pimpl_->pin_configs.find(pin_num); it != pimpl_->pin_configs.end()) {
    enum gpiod_line_value value = gpiod_line_request_get_value(pimpl_->line_request, it->second.offset);
    return value == GPIOD_LINE_VALUE_ACTIVE;
  }
  return false;
}

auto Device::spi_transfer(std::uint8_t value) -> std::uint8_t {
  if (!pimpl_->spi_initialized || pimpl_->spi_fd < 0) {
    return 0;
  }

  struct spi_ioc_transfer transfer{};
  std::uint8_t tx_buf = value;
  std::uint8_t rx_buf = 0;

  transfer.tx_buf = reinterpret_cast<std::uintptr_t>(&tx_buf);
  transfer.rx_buf = reinterpret_cast<std::uintptr_t>(&rx_buf);
  transfer.len = SPI_SINGLE_TRANSFER_LENGTH;
  transfer.speed_hz = SPI_SPEED_HZ;
  transfer.bits_per_word = SPI_BITS_PER_WORD;

  if (ioctl(pimpl_->spi_fd, SPI_IOC_MESSAGE(1), &transfer) < 0) {
    return 0;
  }

  return rx_buf;
}

auto Device::spi_write(std::span<const std::byte> data) -> void {
  if (!pimpl_->spi_initialized || pimpl_->spi_fd < 0 || data.empty()) {
    return;
  }

  struct spi_ioc_transfer transfer{};
  std::vector<std::uint8_t> tx_buf(data.size());
  std::vector<std::uint8_t> rx_buf(data.size());

  // Convert std::byte to uint8_t
  for (std::size_t i = 0; i < data.size(); ++i) {
    tx_buf[i] = static_cast<std::uint8_t>(data[i]);
  }

  transfer.tx_buf = reinterpret_cast<std::uintptr_t>(tx_buf.data());
  transfer.rx_buf = reinterpret_cast<std::uintptr_t>(rx_buf.data());
  transfer.len = static_cast<std::uint32_t>(data.size());
  transfer.speed_hz = SPI_SPEED_HZ;
  transfer.bits_per_word = SPI_BITS_PER_WORD;

  ioctl(pimpl_->spi_fd, SPI_IOC_MESSAGE(1), &transfer);
}

auto Device::delay_ms(std::uint32_t milliseconds) -> void {
  std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

auto Device::delay_us(std::uint32_t microseconds) -> void {
  std::this_thread::sleep_for(std::chrono::microseconds(microseconds));
}

} // namespace epaper
