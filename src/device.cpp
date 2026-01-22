#include "epaper/core/device.hpp"

#include <chrono>
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <thread>
#include <unistd.h>
#include <utility>
#include <vector>

namespace epaper {

// SPI configuration constants
namespace {
// SPI protocol parameters - hardware-specific values for e-paper displays
constexpr std::uint8_t SPI_BITS_PER_WORD = 8;           // 8 bits per transfer, MSB first (industry standard)
constexpr std::uint32_t SPI_SINGLE_TRANSFER_LENGTH = 1; // Single byte transfers for command/data
constexpr int INVALID_FILE_DESCRIPTOR = -1;             // Sentinel value for unopened file descriptors
} // namespace

auto Device::rebuild_line_request() -> bool {
  if (chip_ == nullptr || pin_configs_.empty()) {
    return false;
  }

  // Release old request if exists - libgpiod v2 requires rebuilding entire request
  // when adding new pins (no incremental add API available)
  if (line_request_ != nullptr) {
    gpiod_line_request_release(line_request_);
    line_request_ = nullptr;
  }

  // Create line config - container for all pin settings
  // This will be populated with individual pin configurations below
  struct gpiod_line_config *line_cfg = gpiod_line_config_new();
  if (line_cfg == nullptr) {
    return false;
  }

  // Add all pins to the config - must configure every pin in single batch
  for (const auto &[pin_num, config] : pin_configs_) {
    // Create settings for this specific pin
    struct gpiod_line_settings *settings = gpiod_line_settings_new();
    if (settings == nullptr) {
      gpiod_line_config_free(line_cfg);
      return false;
    }

    // Configure direction (input vs output) and initial state
    if (config.is_output) {
      gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);
      gpiod_line_settings_set_output_value(settings, config.initial_value);
    } else {
      gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_INPUT);
    }

    // Apply settings to this pin's offset (hardware line number)
    unsigned int offset = config.offset;
    if (gpiod_line_config_add_line_settings(line_cfg, &offset, 1, settings) < 0) {
      gpiod_line_settings_free(settings);
      gpiod_line_config_free(line_cfg);
      return false;
    }
    gpiod_line_settings_free(settings); // Settings copied, can free
  }

  // Create request config with consumer name for debugging
  // Consumer name appears in `/sys/kernel/debug/gpio` on Linux
  struct gpiod_request_config *req_cfg = gpiod_request_config_new();
  if (req_cfg == nullptr) {
    gpiod_line_config_free(line_cfg);
    return false;
  }
  gpiod_request_config_set_consumer(req_cfg, "libepaper");

  // Request all lines in a single atomic operation
  // This locks the pins for exclusive access until release
  line_request_ = gpiod_chip_request_lines(chip_, req_cfg, line_cfg);

  gpiod_request_config_free(req_cfg);
  gpiod_line_config_free(line_cfg);

  return line_request_ != nullptr;
}

auto Device::cleanup() noexcept -> void {
  // Release resources in reverse order of acquisition for exception safety

  // First, release GPIO line request (must be done before closing chip)
  if (line_request_ != nullptr) {
    gpiod_line_request_release(line_request_);
    line_request_ = nullptr;
  }
  pin_configs_.clear(); // Clear pin tracking

  // Close GPIO chip (releases file descriptor)
  if (chip_ != nullptr) {
    gpiod_chip_close(chip_);
    chip_ = nullptr;
  }

  // Close SPI device file descriptor
  if (spi_fd_ >= 0) {
    ::close(spi_fd_);
    spi_fd_ = INVALID_FILE_DESCRIPTOR;
  }

  // Reset initialization flags
  initialized_ = false;
  spi_initialized_ = false;
}

Device::Device() : Device(DeviceConfig{}) {}

Device::Device(DeviceConfig config) : config_(std::move(config)) {}

Device::~Device() noexcept { cleanup(); }

Device::Device(Device &&other) noexcept
    : chip_(other.chip_), line_request_(other.line_request_), pin_configs_(std::move(other.pin_configs_)),
      spi_fd_(other.spi_fd_), initialized_(other.initialized_), spi_initialized_(other.spi_initialized_),
      config_(std::move(other.config_)) {
  other.chip_ = nullptr;
  other.line_request_ = nullptr;
  other.spi_fd_ = INVALID_FILE_DESCRIPTOR;
  other.initialized_ = false;
  other.spi_initialized_ = false;
}

auto Device::operator=(Device &&other) noexcept -> Device & {
  if (this != &other) {
    cleanup();

    chip_ = other.chip_;
    line_request_ = other.line_request_;
    pin_configs_ = std::move(other.pin_configs_);
    spi_fd_ = other.spi_fd_;
    initialized_ = other.initialized_;
    spi_initialized_ = other.spi_initialized_;
    config_ = std::move(other.config_);

    other.chip_ = nullptr;
    other.line_request_ = nullptr;
    other.spi_fd_ = INVALID_FILE_DESCRIPTOR;
    other.initialized_ = false;
    other.spi_initialized_ = false;
  }
  return *this;
}

auto Device::init() -> std::expected<void, Error> {
  // Idempotent - safe to call multiple times
  if (initialized_) {
    return {};
  }

  // Initialize GPIO chip (libgpiod v2 API)
  // Opens /dev/gpiochipN for character device access
  chip_ = gpiod_chip_open(config_.gpio_chip.c_str());
  if (chip_ == nullptr) {
    return std::unexpected(Error(ErrorCode::GPIOInitFailed, std::string("Failed to open ") + config_.gpio_chip));
  }
  initialized_ = true;

  // Initialize SPI device (Linux SPIdev userspace API)
  // Opens /dev/spidevX.Y for SPI communication
  spi_fd_ = open(config_.spi_device.c_str(), O_RDWR);
  if (spi_fd_ < 0) {
    cleanup();
    return std::unexpected(Error(ErrorCode::SPIDeviceOpenFailed, std::string("Failed to open ") + config_.spi_device));
  }

  // Configure SPI mode 0 (CPOL=0, CPHA=0)
  // - CPOL=0: Clock idle state is low
  // - CPHA=0: Data sampled on leading (rising) edge
  // This is the most common mode for e-paper displays
  std::uint8_t mode = SPI_MODE_0;
  if (ioctl(spi_fd_, SPI_IOC_WR_MODE, &mode) < 0) {
    cleanup();
    return std::unexpected(Error(ErrorCode::SPIConfigFailed, "Failed to set SPI mode"));
  }

  // Configure SPI clock speed (max frequency)
  // Default ~1.95 MHz is conservative and works across most hardware
  // Higher speeds (4-10 MHz) possible but may require signal integrity validation
  const auto spi_speed = config_.spi_speed_hz;
  if (ioctl(spi_fd_, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed) < 0) {
    cleanup();
    return std::unexpected(Error(ErrorCode::SPIConfigFailed, "Failed to set SPI speed"));
  }

  // Configure bits per word (8-bit transfers, MSB first)
  // E-paper displays expect byte-oriented communication
  if (ioctl(spi_fd_, SPI_IOC_WR_BITS_PER_WORD, &SPI_BITS_PER_WORD) < 0) {
    cleanup();
    return std::unexpected(Error(ErrorCode::SPIConfigFailed, "Failed to set SPI bits per word"));
  }

  spi_initialized_ = true;

  return {};
}

auto Device::is_initialized() const noexcept -> bool { return initialized_; }

auto Device::set_pin_output(Pin pin) -> void {
  if (!initialized_ || chip_ == nullptr) {
    return;
  }

  const auto pin_num = pin.number();
  const auto offset = static_cast<unsigned int>(pin_num);

  // Create configuration for this pin
  PinConfig config{};
  config.offset = offset;
  config.is_output = true;
  config.initial_value = GPIOD_LINE_VALUE_INACTIVE; // Start low (0V)
  pin_configs_[pin_num] = config;

  // Rebuild line request to include new pin
  // libgpiod v2 requires requesting all lines together
  if (!rebuild_line_request()) {
    pin_configs_.erase(pin_num); // Rollback on failure
  }
}

auto Device::set_pin_input(Pin pin) -> void {
  if (!initialized_ || chip_ == nullptr) {
    return;
  }

  const auto pin_num = pin.number();
  const auto offset = static_cast<unsigned int>(pin_num);

  // Create configuration for input pin
  PinConfig config{};
  config.offset = offset;
  config.is_output = false;
  config.initial_value = GPIOD_LINE_VALUE_INACTIVE; // Unused for input pins
  pin_configs_[pin_num] = config;

  // Rebuild line request to include new pin
  if (!rebuild_line_request()) {
    pin_configs_.erase(pin_num); // Rollback on failure
  }
}

auto Device::write_pin(Pin pin, bool value) -> void {
  if (!initialized_ || line_request_ == nullptr) {
    return;
  }

  const auto pin_num = pin.number();
  if (auto it = pin_configs_.find(pin_num); it != pin_configs_.end()) {
    // Convert bool to libgpiod line value (high/low voltage)
    enum gpiod_line_value line_value = value ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE;
    gpiod_line_request_set_value(line_request_, it->second.offset, line_value);
  }
}

auto Device::read_pin(Pin pin) -> bool {
  if (!initialized_ || line_request_ == nullptr) {
    return false;
  }

  const auto pin_num = pin.number();
  if (auto it = pin_configs_.find(pin_num); it != pin_configs_.end()) {
    // Read current pin state (typically used for BUSY signal monitoring)
    enum gpiod_line_value value = gpiod_line_request_get_value(line_request_, it->second.offset);
    return value == GPIOD_LINE_VALUE_ACTIVE;
  }
  return false;
}

auto Device::spi_transfer(std::uint8_t value) const -> std::uint8_t {
  if (!spi_initialized_ || spi_fd_ < 0) {
    return 0;
  }

  // Setup SPI transfer structure for full-duplex communication
  // (simultaneous send and receive - standard SPI behavior)
  struct spi_ioc_transfer transfer{};
  std::uint8_t tx_buf = value; // Byte to transmit
  std::uint8_t rx_buf = 0;     // Byte to receive

  transfer.tx_buf = reinterpret_cast<std::uintptr_t>(&tx_buf);
  transfer.rx_buf = reinterpret_cast<std::uintptr_t>(&rx_buf);
  transfer.len = SPI_SINGLE_TRANSFER_LENGTH;
  transfer.speed_hz = config_.spi_speed_hz;
  transfer.bits_per_word = SPI_BITS_PER_WORD;

  // Execute transfer via ioctl - blocking until complete
  if (ioctl(spi_fd_, SPI_IOC_MESSAGE(1), &transfer) < 0) {
    return 0;
  }

  return rx_buf; // Return received byte (often unused for displays)
}

auto Device::spi_write(std::span<const std::byte> data) const -> void {
  if (!spi_initialized_ || spi_fd_ < 0 || data.empty()) {
    return;
  }

  // Bulk SPI write - used for framebuffer transfers
  // Converts std::byte span to uint8_t for SPIdev API
  struct spi_ioc_transfer transfer{};
  std::vector<std::uint8_t> tx_buf(data.size());
  std::vector<std::uint8_t> rx_buf(data.size()); // Required by API but unused

  // Copy std::byte to std::uint8_t (same bit pattern, different type)
  for (std::size_t i = 0; i < data.size(); ++i) {
    tx_buf[i] = static_cast<std::uint8_t>(data[i]);
  }

  transfer.tx_buf = reinterpret_cast<std::uintptr_t>(tx_buf.data());
  transfer.rx_buf = reinterpret_cast<std::uintptr_t>(rx_buf.data());
  transfer.len = static_cast<std::uint32_t>(data.size());
  transfer.speed_hz = config_.spi_speed_hz;
  transfer.bits_per_word = SPI_BITS_PER_WORD;

  // Execute bulk transfer - blocks until all bytes sent
  ioctl(spi_fd_, SPI_IOC_MESSAGE(1), &transfer);
}

auto Device::delay_ms(std::uint32_t milliseconds) -> void {
  std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

auto Device::delay_us(std::uint32_t microseconds) -> void {
  std::this_thread::sleep_for(std::chrono::microseconds(microseconds));
}

} // namespace epaper
