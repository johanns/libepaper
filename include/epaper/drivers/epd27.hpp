#pragma once

#include "epaper/device.hpp"
#include "epaper/drivers/capabilities.hpp"
#include "epaper/drivers/driver.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>

namespace epaper {

/**
 * @brief E-paper display command codes for EPD27.
 *
 * Command codes sent to the display controller to control various operations.
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

// 2.7 inch e-paper display driver (176x264 pixels)
class EPD27 : public Driver {
public:
  static constexpr std::size_t WIDTH = 176;
  static constexpr std::size_t HEIGHT = 264;

  explicit EPD27(Device &device);
  ~EPD27() override = default;

  // Driver interface implementation
  [[nodiscard]] auto init(DisplayMode mode) -> std::expected<void, Error> override;
  auto clear() -> void override;
  [[nodiscard]] auto display(std::span<const std::byte> buffer) -> std::expected<void, Error> override;
  auto sleep() -> void override;
  [[nodiscard]] auto wake() -> std::expected<void, Error> override;
  [[nodiscard]] auto power_off() -> std::expected<void, Error> override;
  [[nodiscard]] auto power_on() -> std::expected<void, Error> override;

  [[nodiscard]] auto width() const noexcept -> std::size_t override { return WIDTH; }
  [[nodiscard]] auto height() const noexcept -> std::size_t override { return HEIGHT; }
  [[nodiscard]] auto mode() const noexcept -> DisplayMode override { return current_mode_; }
  [[nodiscard]] auto buffer_size() const noexcept -> std::size_t override;
  [[nodiscard]] auto supports_partial_refresh() const noexcept -> bool override { return false; }
  [[nodiscard]] auto supports_wake() const noexcept -> bool override { return false; }
  [[nodiscard]] auto supports_power_control() const noexcept -> bool override { return true; }

private:
  // Hardware reset
  auto reset() -> void;

  // Low-level command/data transmission
  auto send_command(Command command) -> void;
  auto send_data(std::uint8_t data) -> void;

  // Wait for display to be ready
  auto wait_busy() -> void;

  // LUT (Look-Up Table) initialization
  auto set_lut_bw() -> void;
  auto set_lut_grayscale() -> void;

  Device &device_;
  DisplayMode current_mode_ = DisplayMode::BlackWhite;
  bool initialized_ = false;

  // LUT tables for black/white mode
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
};

/**
 * @brief Capability specialization for EPD27 driver.
 *
 * EPD27 is a 2.7" e-paper display supporting both 1-bit black/white
 * and 2-bit 4-level grayscale modes.
 */
template <> struct driver_capabilities<EPD27> {
  static constexpr ColorDepth color_depth = ColorDepth::Bits2; ///< Supports 2-bit grayscale
  static constexpr bool supports_grayscale = true;             ///< Grayscale capable
  static constexpr bool supports_partial_refresh = true;       ///< Partial refresh capable
  static constexpr bool supports_power_control = true;         ///< Power control capable
  static constexpr bool supports_wake_from_sleep = false;      ///< Requires re-init after sleep
  static constexpr std::size_t max_width = 176;                 ///< Maximum width in pixels
  static constexpr std::size_t max_height = 264;                ///< Maximum height in pixels
};

} // namespace epaper
