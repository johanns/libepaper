#pragma once

#include "epaper/core/device.hpp"
#include "epaper/drivers/capabilities.hpp"
#include "epaper/drivers/driver.hpp"
#include "epaper/drivers/driver_concepts.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>
#include <utility>

namespace epaper {

/**
 * @brief E-paper display command codes for EPD27.
 */
enum class Command : std::uint8_t {
  BOOSTER_SOFT_START = 0x06,        ///< Booster soft start control
  DATA_START_TRANSMISSION_1 = 0x10, ///< Start data transmission (old data)
  DISPLAY_REFRESH = 0x12,           ///< Refresh display
  DATA_START_TRANSMISSION_2 = 0x13, ///< Start data transmission (new data)
  PARTIAL_DISPLAY_REFRESH = 0x16,   ///< Partial display refresh control
  LUT_VCOM = 0x20,                  ///< VCOM LUT register
  LUT_WW = 0x21,                    ///< White-to-White LUT register
  LUT_BW = 0x22,                    ///< Black-to-White LUT register
  LUT_WB = 0x23,                    ///< White-to-Black LUT register
  LUT_BB = 0x24,                    ///< Black-to-Black LUT register
  LUT_WW2 = 0x25,                   ///< Additional White-to-White LUT register
  PLL_CONTROL = 0x30,               ///< PLL control (frame rate)
  VCOM_DATA_INTERVAL = 0x50,        ///< VCOM and data interval setting
  RESOLUTION_SETTING = 0x61,        ///< Resolution setting
  GET_STATUS = 0x71,                ///< Get display status
  VCM_DC_SETTING = 0x82,            ///< VCM DC setting
  POWER_OPTIMIZATION = 0xF8,        ///< Power optimization register
  PANEL_SETTING = 0x00,             ///< Panel setting register
  POWER_SETTING = 0x01,             ///< Power setting
  POWER_OFF = 0x02,                 ///< Power off command
  POWER_ON = 0x04,                  ///< Power on command
  DEEP_SLEEP = 0x07                 ///< Deep sleep mode
};

/**
 * @brief Timing constants for EPD27 operations (in milliseconds).
 */
namespace Timing {
constexpr std::uint32_t BUSY_WAIT_DELAY_MS = 200;       ///< Delay after busy wait
constexpr std::uint32_t DISPLAY_REFRESH_DELAY_MS = 200; ///< Delay after display refresh
constexpr std::uint32_t RESET_DELAY_MS = 200;           ///< Reset signal delay
constexpr std::uint32_t RESET_PULSE_MS = 2;             ///< Reset pulse duration
constexpr std::uint32_t BUSY_POLL_DELAY_MS = 10;        ///< Polling delay when checking busy status
} // namespace Timing

/**
 * @brief Power configuration settings for EPD27.
 *
 * Configuration values for power supply and booster settings.
 */
struct PowerConfig {
  std::uint8_t vds_en_vdg_en;   ///< VDS_EN and VDG_EN control
  std::uint8_t vcom_hv_vghl_lv; ///< VCOM_HV and VGHL_LV control
  std::uint8_t vdh;             ///< VDH voltage setting
  std::uint8_t vdl;             ///< VDL voltage setting
  std::uint8_t vdhr;            ///< VDHR voltage setting (black/white mode only)
};

/**
 * @brief Power configuration for black/white mode.
 */
constexpr PowerConfig POWER_CONFIG_BW = {
    .vds_en_vdg_en = 0x03, .vcom_hv_vghl_lv = 0x00, .vdh = 0x2B, .vdl = 0x2B, .vdhr = 0x09};

/**
 * @brief Power configuration for grayscale mode.
 */
constexpr PowerConfig POWER_CONFIG_GRAYSCALE = {
    .vds_en_vdg_en = 0x03,
    .vcom_hv_vghl_lv = 0x00,
    .vdh = 0x2B,
    .vdl = 0x2B,
    .vdhr = 0x00 // Not used in grayscale mode
};

/**
 * @brief Booster soft start configuration.
 *
 * Controls the booster soft start behavior for power supply.
 */
struct BoosterConfig {
  std::uint8_t phase1; ///< Phase 1 setting
  std::uint8_t phase2; ///< Phase 2 setting
  std::uint8_t phase3; ///< Phase 3 setting
};

/**
 * @brief Standard booster soft start configuration.
 */
constexpr BoosterConfig BOOSTER_CONFIG = {.phase1 = 0x07, .phase2 = 0x07, .phase3 = 0x17};

/**
 * @brief Power optimization register settings.
 *
 * Sequence of register and value pairs for power optimization.
 */
namespace PowerOptimization {
constexpr std::uint8_t REG1 = 0x60;
constexpr std::uint8_t VAL1 = 0xA5;
constexpr std::uint8_t REG2 = 0x89;
constexpr std::uint8_t VAL2 = 0xA5;
constexpr std::uint8_t REG3 = 0x90;
constexpr std::uint8_t VAL3 = 0x00;
constexpr std::uint8_t REG4 = 0x93;
constexpr std::uint8_t VAL4 = 0x2A;
constexpr std::uint8_t REG5 = 0xA0;
constexpr std::uint8_t VAL5 = 0xA5;
constexpr std::uint8_t REG6 = 0xA1;
constexpr std::uint8_t VAL6 = 0x00;
constexpr std::uint8_t REG7 = 0x73;
constexpr std::uint8_t VAL7 = 0x41;
} // namespace PowerOptimization

/**
 * @brief Panel configuration constants.
 */
namespace PanelConfig {
constexpr std::uint8_t PANEL_SETTING_BW = 0xAF;        ///< Panel setting for black/white mode
constexpr std::uint8_t PANEL_SETTING_GRAYSCALE = 0xBF; ///< Panel setting for grayscale mode
constexpr std::uint8_t PLL_SETTING_BW = 0x3A;          ///< PLL 100Hz for black/white mode
constexpr std::uint8_t PLL_SETTING_GRAYSCALE = 0x90;   ///< PLL 100Hz for grayscale mode
constexpr std::uint8_t VCM_DC_SETTING_VALUE = 0x12;    ///< VCM DC setting value
} // namespace PanelConfig

/**
 * @brief Resolution setting constants.
 */
namespace Resolution {
constexpr std::uint8_t WIDTH_HIGH = 0x00;  ///< Width high byte (176 >> 8)
constexpr std::uint8_t WIDTH_LOW = 0xB0;   ///< Width low byte (176 & 0xFF)
constexpr std::uint8_t HEIGHT_HIGH = 0x01; ///< Height high byte (264 >> 8)
constexpr std::uint8_t HEIGHT_LOW = 0x08;  ///< Height low byte (264 & 0xFF)
} // namespace Resolution

/**
 * @brief Bit manipulation constants for grayscale processing.
 */
namespace Grayscale {
constexpr std::uint8_t BLACK_MASK = 0x00;  ///< Bit pattern for black pixel
constexpr std::uint8_t BIT_SHIFT = 2;      ///< Bit shift for 2-bit pixels
constexpr std::uint8_t GRAY1_MASK = 0x80;  ///< Bit pattern for gray level 1
constexpr std::uint8_t GRAY2_MASK = 0x40;  ///< Bit pattern for gray level 2
constexpr std::uint8_t PIXEL_MASK = 0xC0;  ///< Mask for extracting 2-bit pixel
constexpr std::size_t TOTAL_PIXELS = 5808; ///< Total pixels / 8 for grayscale (176*264/8)
constexpr std::uint8_t WHITE_MASK = 0xC0;  ///< Bit pattern for white pixel
} // namespace Grayscale

/**
 * @brief Display operation constants.
 */
namespace DisplayOps {
constexpr std::uint8_t BUSY_STATUS_MASK = 0x01;             ///< Mask for busy status bit
constexpr std::uint8_t CLEAR_FILL_VALUE = 0xFF;             ///< Fill value for clearing (white)
constexpr std::uint8_t PARTIAL_REFRESH_DISABLE = 0x00;      ///< Disable partial refresh
constexpr std::uint8_t SLEEP_VCOM_DATA_INTERVAL = 0xF7;     ///< VCOM interval for sleep mode
constexpr std::uint8_t DEEP_SLEEP_MAGIC = 0xA5;             ///< Magic value for deep sleep
constexpr std::uint8_t VCOM_DATA_INTERVAL_GRAYSCALE = 0x97; ///< VCOM interval for grayscale
} // namespace DisplayOps

/**
 * @brief Pin configuration for standard Raspberry Pi HATs.
 */
struct EPD27PinConfig {
  Pin rst{0};
  Pin dc{0};
  Pin cs{0};
  Pin busy{0};
  std::optional<Pin> pwr;

  static constexpr auto waveshare_hat() -> EPD27PinConfig {
    return EPD27PinConfig{.rst = Pin{17}, .dc = Pin{25}, .cs = Pin{8}, .busy = Pin{24}, .pwr = Pin{18}};
  }
};

/**
 * @brief 2.7 inch e-paper display driver (176x264 pixels).
 *
 * 2.7 inch e-paper display driver for Linux (Raspberry Pi).
 */
class EPD27 {
public:
  static constexpr std::size_t WIDTH = 176;
  static constexpr std::size_t HEIGHT = 264;

  /**
   * @brief Construct EPD27 driver with direct hardware resources.
   *
   * @param spi SPI bus instance
   * @param cs Chip Select pin
   * @param dc Data/Command pin
   * @param rst Reset pin
   * @param busy Busy signal pin
   * @param pwr Optional power control pin
   */
  EPD27(Device::HalSpi spi, Device::HalOutput cs, Device::HalOutput dc, Device::HalOutput rst, Device::HalInput busy,
        std::optional<Device::HalOutput> pwr = std::nullopt)
      : spi_(spi), cs_(cs), dc_(dc), rst_(rst), busy_(busy), pwr_(std::move(pwr)) {}

  /**
   * @brief Construct using custom pin configuration.
   *
   * @param device Initialized Linux device
   * @param pins Pin map
   */
  EPD27(Device &device, EPD27PinConfig pins)
      : EPD27(device.get_spi(), device.get_output(pins.cs), device.get_output(pins.dc), device.get_output(pins.rst),
              device.get_input(pins.busy), pins.pwr ? std::make_optional(device.get_output(*pins.pwr)) : std::nullopt) {
  }

  /**
   * @brief Construct using default Waveshare HAT pins.
   *
   * @param device Initialized Linux device
   */
  explicit EPD27(Device &device) : EPD27(device, EPD27PinConfig::waveshare_hat()) {}

  ~EPD27() = default;

  // Driver interface implementation
  [[nodiscard]] auto init(DisplayMode mode) -> std::expected<void, Error> {
    if (is_color_mode(mode)) {
      return std::unexpected(
          Error(ErrorCode::InvalidMode, "Color modes not supported by this EPD27 driver configuration"));
    }

    current_mode_ = mode;
    reset();

    if (pwr_.has_value()) {
      pwr_->write(true); // Turn on potential power switch
    }

    if (mode == DisplayMode::BlackWhite) {
      init_bw();
    } else {
      init_grayscale();
    }

    initialized_ = true;
    is_asleep_ = false;
    return {};
  }

  [[nodiscard]] auto clear() -> std::expected<void, Error> {
    if (!initialized_) {
      return std::unexpected(Error(ErrorCode::DriverNotInitialized));
    }
    if (is_asleep_) {
      if (auto res = wake(); !res) {
        return res;
      }
    }

    const auto width_bytes = (WIDTH + 7) / 8;

    send_command(Command::DATA_START_TRANSMISSION_1);
    for (std::size_t j = 0; j < HEIGHT; ++j) {
      for (std::size_t i = 0; i < width_bytes; ++i) {
        send_data(DisplayOps::CLEAR_FILL_VALUE);
      }
    }

    send_command(Command::DATA_START_TRANSMISSION_2);
    for (std::size_t j = 0; j < HEIGHT; ++j) {
      for (std::size_t i = 0; i < width_bytes; ++i) {
        send_data(DisplayOps::CLEAR_FILL_VALUE);
      }
    }

    send_command(Command::DISPLAY_REFRESH);
    wait_busy();
    return {};
  }

  [[nodiscard]] auto display(std::span<const std::byte> buffer) -> std::expected<void, Error> {
    if (is_asleep_) {
      if (auto res = wake(); !res) {
        return res;
      }
    }

    if (current_mode_ == DisplayMode::BlackWhite) {
      const auto width_bytes = (WIDTH + 7) / 8;
      send_command(Command::DATA_START_TRANSMISSION_2);
      for (std::size_t j = 0; j < HEIGHT; ++j) {
        for (std::size_t i = 0; i < width_bytes; ++i) {
          const auto index = i + (j * width_bytes);
          send_data(static_cast<std::uint8_t>(buffer[index]));
        }
      }
      send_command(Command::DISPLAY_REFRESH);
      wait_busy();
      return {};
    } // Grayscale
    const auto total_pixels = Grayscale::TOTAL_PIXELS;

    send_command(Command::DATA_START_TRANSMISSION_1);
    for (std::size_t i = 0; i < total_pixels; ++i) {
      std::size_t src_idx = i * 2;
      auto b1 = static_cast<std::uint8_t>(buffer[src_idx]);
      auto b2 = static_cast<std::uint8_t>(buffer[src_idx + 1]);
      send_data(convert_grayscale_pixel(b1, b2, true));
    }

    send_command(Command::DATA_START_TRANSMISSION_2);
    for (std::size_t i = 0; i < total_pixels; ++i) {
      std::size_t src_idx = i * 2;
      auto b1 = static_cast<std::uint8_t>(buffer[src_idx]);
      auto b2 = static_cast<std::uint8_t>(buffer[src_idx + 1]);
      send_data(convert_grayscale_pixel(b1, b2, false));
    }

    set_lut_grayscale();
    send_command(Command::DISPLAY_REFRESH);
    Device::Delay::delay_ms(Timing::DISPLAY_REFRESH_DELAY_MS);
    wait_busy();
    return {};
  }

  [[nodiscard]] auto display_planes(std::span<const std::span<const std::byte>> planes) -> std::expected<void, Error> {
    if (!initialized_) {
      return std::unexpected(Error(ErrorCode::DriverNotInitialized));
    }
    if (is_asleep_) {
      if (auto res = wake(); !res) {
        return res;
      }
    }
    if (planes.empty()) {
      return std::unexpected(Error(ErrorCode::InvalidDimensions, "No planes provided"));
    }
    if (current_mode_ == DisplayMode::BlackWhite || current_mode_ == DisplayMode::Grayscale4) {
      return display(planes[0]);
    }
    return std::unexpected(Error(ErrorCode::InvalidMode, "Color planes not supported"));
  }

  [[nodiscard]] auto sleep() -> std::expected<void, Error> {
    if (is_asleep_) {
      return {};
    }
    if (!initialized_) {
      return std::unexpected(Error(ErrorCode::DriverNotInitialized));
    }

    send_command(Command::VCOM_DATA_INTERVAL);
    send_data(DisplayOps::SLEEP_VCOM_DATA_INTERVAL);
    send_command(Command::POWER_OFF);
    send_command(Command::DEEP_SLEEP);
    send_data(DisplayOps::DEEP_SLEEP_MAGIC);

    is_asleep_ = true;
    return {};
  }

  [[nodiscard]] auto wake() -> std::expected<void, Error> {
    if (!is_asleep_) {
      return {};
    }
    return init(current_mode_);
  }

  [[nodiscard]] auto power_off() -> std::expected<void, Error> {
    if (!initialized_) {
      return std::unexpected(Error(ErrorCode::DriverNotInitialized));
    }
    send_command(Command::POWER_OFF);
    wait_busy_simple();
    return {};
  }

  [[nodiscard]] auto power_on() -> std::expected<void, Error> {
    if (!initialized_) {
      return std::unexpected(Error(ErrorCode::DriverNotInitialized));
    }
    send_command(Command::POWER_ON);
    wait_busy_simple();
    return {};
  }

  [[nodiscard]] static auto width() noexcept -> std::size_t { return WIDTH; }
  [[nodiscard]] static auto height() noexcept -> std::size_t { return HEIGHT; }
  [[nodiscard]] auto mode() const noexcept -> DisplayMode { return current_mode_; }

  [[nodiscard]] auto buffer_size() const noexcept -> std::size_t {
    if (current_mode_ == DisplayMode::BlackWhite) {
      return ((WIDTH + 7) / 8) * HEIGHT;
    }
    return ((WIDTH + 3) / 4) * HEIGHT;
  }

  [[nodiscard]] static auto supports_partial_refresh() noexcept -> bool { return false; }
  [[nodiscard]] static auto supports_power_control() noexcept -> bool { return true; }
  [[nodiscard]] static auto supports_wake() noexcept -> bool { return false; }

private:
  Device::HalSpi spi_;
  Device::HalOutput cs_;
  Device::HalOutput dc_;
  Device::HalOutput rst_;
  Device::HalInput busy_;
  std::optional<Device::HalOutput> pwr_;
  DisplayMode current_mode_ = DisplayMode::BlackWhite;
  bool initialized_ = false;
  bool is_asleep_ = false;

  void reset() {
    rst_.write(true);
    Device::Delay::delay_ms(Timing::RESET_DELAY_MS);
    rst_.write(false);
    Device::Delay::delay_ms(Timing::RESET_PULSE_MS);
    rst_.write(true);
    Device::Delay::delay_ms(Timing::RESET_DELAY_MS);
  }

  void send_command(Command command) {
    dc_.write(false);
    cs_.write(false);
    spi_.transfer(static_cast<std::uint8_t>(command));
    cs_.write(true);
  }

  void send_data(std::uint8_t data) {
    dc_.write(true);
    cs_.write(false);
    spi_.transfer(data);
    cs_.write(true);
  }

  void wait_busy() {
    // Advanced wait with GET_STATUS could go here, but omitted for brevity/reliability
    // Falling back to simple wait which is safer during transitions
    wait_busy_simple();
  }

  void wait_busy_simple() {
    int iterations = 0;
    while (busy_.read() && iterations < 100) {
      Device::Delay::delay_ms(Timing::BUSY_POLL_DELAY_MS);
      iterations++;
    }
    iterations = 0;
    while (!busy_.read() && iterations < 1000) {
      Device::Delay::delay_ms(Timing::BUSY_POLL_DELAY_MS);
      iterations++;
    }
    Device::Delay::delay_ms(Timing::BUSY_WAIT_DELAY_MS);
  }

  void init_bw() {
    send_command(Command::POWER_SETTING);
    send_data(POWER_CONFIG_BW.vds_en_vdg_en);
    send_data(POWER_CONFIG_BW.vcom_hv_vghl_lv);
    send_data(POWER_CONFIG_BW.vdh);
    send_data(POWER_CONFIG_BW.vdl);
    send_data(POWER_CONFIG_BW.vdhr);

    send_command(Command::BOOSTER_SOFT_START);
    send_data(BOOSTER_CONFIG.phase1);
    send_data(BOOSTER_CONFIG.phase2);
    send_data(BOOSTER_CONFIG.phase3);

    send_command(Command::POWER_OPTIMIZATION);
    send_data(PowerOptimization::REG1);
    send_data(PowerOptimization::VAL1);
    send_command(Command::POWER_OPTIMIZATION);
    send_data(PowerOptimization::REG2);
    send_data(PowerOptimization::VAL2);
    send_command(Command::POWER_OPTIMIZATION);
    send_data(PowerOptimization::REG3);
    send_data(PowerOptimization::VAL3);
    send_command(Command::POWER_OPTIMIZATION);
    send_data(PowerOptimization::REG4);
    send_data(PowerOptimization::VAL4);
    send_command(Command::POWER_OPTIMIZATION);
    send_data(PowerOptimization::REG5);
    send_data(PowerOptimization::VAL5);
    send_command(Command::POWER_OPTIMIZATION);
    send_data(PowerOptimization::REG6);
    send_data(PowerOptimization::VAL6);
    send_command(Command::POWER_OPTIMIZATION);
    send_data(PowerOptimization::REG7);
    send_data(PowerOptimization::VAL7);

    send_command(Command::PARTIAL_DISPLAY_REFRESH);
    send_data(DisplayOps::PARTIAL_REFRESH_DISABLE);

    send_command(Command::POWER_ON);
    wait_busy();

    send_command(Command::PANEL_SETTING);
    send_data(PanelConfig::PANEL_SETTING_BW);
    send_command(Command::PLL_CONTROL);
    send_data(PanelConfig::PLL_SETTING_BW);
    send_command(Command::VCM_DC_SETTING);
    send_data(PanelConfig::VCM_DC_SETTING_VALUE);
    set_lut_bw();
  }

  void init_grayscale() {
    send_command(Command::POWER_SETTING);
    send_data(POWER_CONFIG_GRAYSCALE.vds_en_vdg_en);
    send_data(POWER_CONFIG_GRAYSCALE.vcom_hv_vghl_lv);
    send_data(POWER_CONFIG_GRAYSCALE.vdh);
    send_data(POWER_CONFIG_GRAYSCALE.vdl);
    send_command(Command::BOOSTER_SOFT_START);
    send_data(BOOSTER_CONFIG.phase1);
    send_data(BOOSTER_CONFIG.phase2);
    send_data(BOOSTER_CONFIG.phase3);

    send_command(Command::POWER_OPTIMIZATION);
    send_data(PowerOptimization::REG1);
    send_data(PowerOptimization::VAL1);
    send_command(Command::POWER_OPTIMIZATION);
    send_data(PowerOptimization::REG2);
    send_data(PowerOptimization::VAL2);
    send_command(Command::POWER_OPTIMIZATION);
    send_data(PowerOptimization::REG3);
    send_data(PowerOptimization::VAL3);
    send_command(Command::POWER_OPTIMIZATION);
    send_data(PowerOptimization::REG4);
    send_data(PowerOptimization::VAL4);
    send_command(Command::POWER_OPTIMIZATION);
    send_data(PowerOptimization::REG5);
    send_data(PowerOptimization::VAL5);
    send_command(Command::POWER_OPTIMIZATION);
    send_data(PowerOptimization::REG6);
    send_data(PowerOptimization::VAL6);
    send_command(Command::POWER_OPTIMIZATION);
    send_data(PowerOptimization::REG7);
    send_data(PowerOptimization::VAL7);

    send_command(Command::PARTIAL_DISPLAY_REFRESH);
    send_data(DisplayOps::PARTIAL_REFRESH_DISABLE);

    send_command(Command::POWER_ON);
    wait_busy();

    send_command(Command::PANEL_SETTING);
    send_data(PanelConfig::PANEL_SETTING_GRAYSCALE);
    send_command(Command::PLL_CONTROL);
    send_data(PanelConfig::PLL_SETTING_GRAYSCALE);
    send_command(Command::RESOLUTION_SETTING);
    send_data(Resolution::WIDTH_HIGH);
    send_data(Resolution::WIDTH_LOW);
    send_data(Resolution::HEIGHT_HIGH);
    send_data(Resolution::HEIGHT_LOW);
    send_command(Command::VCM_DC_SETTING);
    send_data(PanelConfig::VCM_DC_SETTING_VALUE);
    send_command(Command::VCOM_DATA_INTERVAL);
    send_data(DisplayOps::VCOM_DATA_INTERVAL_GRAYSCALE);
  }

  static constexpr std::array<std::uint8_t, 44> LUT_VCOM_DC = {
      0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x02, 0x60, 0x28, 0x28, 0x00, 0x00, 0x01, 0x00,
      0x14, 0x00, 0x00, 0x00, 0x01, 0x00, 0x12, 0x12, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  static constexpr std::array<std::uint8_t, 42> LUT_WW = {
      0x40, 0x08, 0x00, 0x00, 0x00, 0x02, 0x90, 0x28, 0x28, 0x00, 0x00, 0x01, 0x40, 0x14,
      0x00, 0x00, 0x00, 0x01, 0xA0, 0x12, 0x12, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  static constexpr std::array<std::uint8_t, 42> LUT_BW = {
      0x40, 0x08, 0x00, 0x00, 0x00, 0x02, 0x90, 0x28, 0x28, 0x00, 0x00, 0x01, 0x40, 0x14,
      0x00, 0x00, 0x00, 0x01, 0xA0, 0x12, 0x12, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  static constexpr std::array<std::uint8_t, 42> LUT_BB = {
      0x80, 0x08, 0x00, 0x00, 0x00, 0x02, 0x90, 0x28, 0x28, 0x00, 0x00, 0x01, 0x80, 0x14,
      0x00, 0x00, 0x00, 0x01, 0x50, 0x12, 0x12, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  static constexpr std::array<std::uint8_t, 42> LUT_WB = {
      0x80, 0x08, 0x00, 0x00, 0x00, 0x02, 0x90, 0x28, 0x28, 0x00, 0x00, 0x01, 0x80, 0x14,
      0x00, 0x00, 0x00, 0x01, 0x50, 0x12, 0x12, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  // LUT tables for 4-level grayscale mode
  static constexpr std::array<std::uint8_t, 44> LUT_VCOM_GRAY = {
      0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x01, 0x60, 0x14, 0x14, 0x00, 0x00, 0x01, 0x00,
      0x14, 0x00, 0x00, 0x00, 0x01, 0x00, 0x13, 0x0A, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  static constexpr std::array<std::uint8_t, 42> LUT_WW_GRAY = {
      0x40, 0x0A, 0x00, 0x00, 0x00, 0x01, 0x90, 0x14, 0x14, 0x00, 0x00, 0x01, 0x10, 0x14,
      0x0A, 0x00, 0x00, 0x01, 0xA0, 0x13, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  static constexpr std::array<std::uint8_t, 42> LUT_BW_GRAY = {
      0x40, 0x0A, 0x00, 0x00, 0x00, 0x01, 0x90, 0x14, 0x14, 0x00, 0x00, 0x01, 0x00, 0x14,
      0x0A, 0x00, 0x00, 0x01, 0x99, 0x0C, 0x01, 0x03, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  static constexpr std::array<std::uint8_t, 42> LUT_WB_GRAY = {
      0x40, 0x0A, 0x00, 0x00, 0x00, 0x01, 0x90, 0x14, 0x14, 0x00, 0x00, 0x01, 0x00, 0x14,
      0x0A, 0x00, 0x00, 0x01, 0x99, 0x0B, 0x04, 0x04, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  static constexpr std::array<std::uint8_t, 42> LUT_BB_GRAY = {
      0x80, 0x0A, 0x00, 0x00, 0x00, 0x01, 0x90, 0x14, 0x14, 0x00, 0x00, 0x01, 0x20, 0x14,
      0x0A, 0x00, 0x00, 0x01, 0x50, 0x13, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  void set_lut_bw() {
    send_command(Command::LUT_VCOM);
    for (auto b : LUT_VCOM_DC) {
      send_data(b);
    }
    send_command(Command::LUT_WW);
    for (auto b : LUT_WW) {
      send_data(b);
    }
    send_command(Command::LUT_BW);
    for (auto b : LUT_BW) {
      send_data(b);
    }
    send_command(Command::LUT_WB);
    for (auto b : LUT_BB) { // Original code behavior preserved
      send_data(b);
    }
    send_command(Command::LUT_BB);
    for (auto b : LUT_WB) { // Original code behavior preserved
      send_data(b);
    }
  }

  void set_lut_grayscale() {
    send_command(Command::LUT_VCOM);
    for (auto b : LUT_VCOM_GRAY) {
      send_data(b);
    }
    send_command(Command::LUT_WW);
    for (auto b : LUT_WW_GRAY) {
      send_data(b);
    }
    send_command(Command::LUT_BW);
    for (auto b : LUT_BW_GRAY) {
      send_data(b);
    }
    send_command(Command::LUT_WB);
    for (auto b : LUT_WB_GRAY) {
      send_data(b);
    }
    send_command(Command::LUT_BB);
    for (auto b : LUT_BB_GRAY) {
      send_data(b);
    }
    send_command(Command::LUT_WW2);
    for (auto b : LUT_WW_GRAY) {
      send_data(b);
    }
  }

  static auto convert_grayscale_pixel(std::uint8_t byte1, std::uint8_t byte2, bool is_old_data) -> std::uint8_t {
    std::uint8_t result = 0;
    for (std::size_t j = 0; j < 2; ++j) {
      auto temp1 = (j == 0) ? byte1 : byte2;
      for (std::size_t k = 0; k < 4; ++k) {
        const std::uint8_t temp2 = temp1 & Grayscale::PIXEL_MASK;
        result <<= 1;
        if (is_old_data) {
          if (temp2 == Grayscale::WHITE_MASK || temp2 == Grayscale::GRAY1_MASK) {
            result |= 0x01;
          }
        } else {
          if (temp2 == Grayscale::WHITE_MASK || temp2 == Grayscale::GRAY2_MASK) {
            result |= 0x01;
          }
        }
        temp1 <<= Grayscale::BIT_SHIFT;
      }
    }
    return result;
  }
};

template <> struct driver_traits<EPD27> {
  static constexpr DisplayMode max_mode = DisplayMode::Grayscale4;
  static constexpr bool supports_grayscale = true;
  static constexpr bool supports_partial_refresh = true;
  static constexpr bool supports_power_control = true;
  static constexpr bool supports_wake_from_sleep = false;
  static constexpr std::size_t max_width = 176;
  static constexpr std::size_t max_height = 264;
};

static_assert(Driver<EPD27>, "EPD27 must satisfy Driver concept");

} // namespace epaper
