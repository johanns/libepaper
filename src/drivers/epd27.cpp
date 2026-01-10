#include "epaper/drivers/epd27.hpp"

namespace epaper {

EPD27::EPD27(Device &device) : device_(device) {}

auto EPD27::buffer_size() const noexcept -> std::size_t {
  if (current_mode_ == DisplayMode::BlackWhite) {
    // 1 bit per pixel, packed into bytes
    const auto width_bytes = (WIDTH % 8 == 0) ? (WIDTH / 8) : ((WIDTH / 8) + 1);
    return width_bytes * HEIGHT;
  }
  // 2 bits per pixel for 4-level grayscale
  const auto width_bytes = (WIDTH % 4 == 0) ? (WIDTH / 4) : ((WIDTH / 4) + 1);
  return width_bytes * HEIGHT;
}

auto EPD27::reset() -> void {
  device_.write_pin(pins::RST, true);
  Device::delay_ms(Timing::RESET_DELAY_MS);
  device_.write_pin(pins::RST, false);
  Device::delay_ms(Timing::RESET_PULSE_MS);
  device_.write_pin(pins::RST, true);
  Device::delay_ms(Timing::RESET_DELAY_MS);
}

auto EPD27::send_command(Command command) -> void {
  device_.write_pin(pins::DC, false); // Command mode
  device_.write_pin(pins::CS, false); // Select device
  device_.spi_transfer(static_cast<std::uint8_t>(command));
  device_.write_pin(pins::CS, true); // Deselect device
}

auto EPD27::send_data(std::uint8_t data) -> void {
  device_.write_pin(pins::DC, true);  // Data mode
  device_.write_pin(pins::CS, false); // Select device
  device_.spi_transfer(data);
  device_.write_pin(pins::CS, true); // Deselect device
}

auto EPD27::wait_busy() -> void {
  bool busy = true;
  while (busy) {
    send_command(Command::GET_STATUS);
    const bool busy_pin = device_.read_pin(pins::BUSY);
    busy = ((busy_pin ? 1 : 0) & DisplayOps::BUSY_STATUS_MASK) == 0;
  }
  Device::delay_ms(Timing::BUSY_WAIT_DELAY_MS);
}

auto EPD27::wait_busy_simple() -> void {
  // Simple busy wait - just poll BUSY pin without sending commands.
  // Used after POWER_OFF/POWER_ON when display may not respond to GET_STATUS.
  // Per datasheet: BUSY_N (active-low) drops LOW when busy, rises HIGH when done.
  // So we wait while BUSY is LOW (busy state).

  // First, wait for BUSY to go LOW (command accepted)
  constexpr int initial_timeout = 100; // 1 second
  int iterations = 0;
  while (device_.read_pin(pins::BUSY) && iterations < initial_timeout) {
    Device::delay_ms(Timing::BUSY_POLL_DELAY_MS);
    iterations++;
  }

  // Then wait for BUSY to go HIGH (command complete)
  constexpr int max_iterations = 1000; // 10 seconds
  iterations = 0;
  while (!device_.read_pin(pins::BUSY) && iterations < max_iterations) {
    Device::delay_ms(Timing::BUSY_POLL_DELAY_MS);
    iterations++;
  }

  Device::delay_ms(Timing::BUSY_WAIT_DELAY_MS);
}

// Helper function to convert grayscale pixel data for display transmission
// Processes 4 pixels (2 bytes input) into EPD format (1 byte output)
// is_old_data: true for DATA_START_TRANSMISSION_1, false for DATA_START_TRANSMISSION_2
// NOLINTNEXTLINE(readability-function-cognitive-complexity) - Hardware-specific pixel format conversion
auto EPD27::convert_grayscale_pixel(std::uint8_t byte1, std::uint8_t byte2, bool is_old_data) -> std::uint8_t {
  std::uint8_t result = 0;

  for (std::size_t j = 0; j < 2; ++j) {
    auto temp1 = (j == 0) ? byte1 : byte2;
    for (std::size_t k = 0; k < 2; ++k) {
      const std::uint8_t temp2 = temp1 & Grayscale::PIXEL_MASK;

      // Convert pixel based on old/new data mode
      if (is_old_data) {
        // Old data: WHITE and GRAY1 map to 0x01, BLACK and GRAY2 map to 0x00
        if (temp2 == Grayscale::WHITE_MASK || temp2 == Grayscale::GRAY1_MASK) {
          result |= 0x01;
        }
      } else {
        // New data: WHITE and GRAY2 map to 0x01, BLACK and GRAY1 map to 0x00
        if (temp2 == Grayscale::WHITE_MASK || temp2 == Grayscale::GRAY2_MASK) {
          result |= 0x01;
        }
      }
      result <<= 1;

      temp1 <<= Grayscale::BIT_SHIFT;
      const std::uint8_t temp2_2 = temp1 & Grayscale::PIXEL_MASK;

      if (is_old_data) {
        if (temp2_2 == Grayscale::WHITE_MASK || temp2_2 == Grayscale::GRAY1_MASK) {
          result |= 0x01;
        }
      } else {
        if (temp2_2 == Grayscale::WHITE_MASK || temp2_2 == Grayscale::GRAY2_MASK) {
          result |= 0x01;
        }
      }

      if (j != 1 || k != 1) {
        result <<= 1;
      }

      temp1 <<= Grayscale::BIT_SHIFT;
    }
  }

  return result;
}

auto EPD27::set_lut_bw() -> void {
  // VCOM LUT
  send_command(Command::LUT_VCOM);
  for (const auto byte : LUT_VCOM_DC) {
    send_data(byte);
  }

  // WW LUT
  send_command(Command::LUT_WW);
  for (const auto byte : LUT_WW) {
    send_data(byte);
  }

  // BW LUT
  send_command(Command::LUT_BW);
  for (const auto byte : LUT_BW) {
    send_data(byte);
  }

  // WB LUT
  send_command(Command::LUT_WB);
  for (const auto byte : LUT_BB) {
    send_data(byte);
  }

  // BB LUT
  send_command(Command::LUT_BB);
  for (const auto byte : LUT_WB) {
    send_data(byte);
  }
}

auto EPD27::set_lut_grayscale() -> void {
  // VCOM LUT
  send_command(Command::LUT_VCOM);
  for (const auto byte : LUT_VCOM_GRAY) {
    send_data(byte);
  }

  // WW LUT
  send_command(Command::LUT_WW);
  for (const auto byte : LUT_WW_GRAY) {
    send_data(byte);
  }

  // BW LUT
  send_command(Command::LUT_BW);
  for (const auto byte : LUT_BW_GRAY) {
    send_data(byte);
  }

  // WB LUT
  send_command(Command::LUT_WB);
  for (const auto byte : LUT_WB_GRAY) {
    send_data(byte);
  }

  // BB LUT
  send_command(Command::LUT_BB);
  for (const auto byte : LUT_BB_GRAY) {
    send_data(byte);
  }

  // Additional LUT for grayscale
  send_command(Command::LUT_WW2);
  for (const auto byte : LUT_WW_GRAY) {
    send_data(byte);
  }
}

auto EPD27::init(DisplayMode mode) -> std::expected<void, Error> {
  if (!device_.is_initialized()) {
    return std::unexpected(Error(ErrorCode::DriverNotInitialized));
  }

  current_mode_ = mode;
  reset();

  if (mode == DisplayMode::BlackWhite) {
    // Black/White mode initialization
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

    // Power optimization
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
  } else {
    // Grayscale mode initialization
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

  initialized_ = true;
  is_asleep_ = false; // Mark as awake after successful initialization
  return {};
}

auto EPD27::clear() -> std::expected<void, Error> {
  if (!initialized_) {
    return std::unexpected(Error(ErrorCode::DriverNotInitialized));
  }

  // Auto-wake if asleep
  if (is_asleep_) {
    auto wake_result = wake();
    if (!wake_result) {
      return wake_result;
    }
  }

  const auto width_bytes = (WIDTH % 8 == 0) ? (WIDTH / 8) : ((WIDTH / 8) + 1);

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

auto EPD27::display(std::span<const std::byte> buffer) -> std::expected<void, Error> {
  // AUTO-WAKE: Check if display is asleep and wake it transparently
  if (is_asleep_) {
    auto wake_result = wake();
    if (!wake_result) {
      return wake_result; // Propagate wake error
    }
  }

  if (current_mode_ == DisplayMode::BlackWhite) {
    const auto width_bytes = (WIDTH % 8 == 0) ? (WIDTH / 8) : ((WIDTH / 8) + 1);

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
  }

  {
    // Grayscale display
    constexpr std::size_t total_pixels = Grayscale::TOTAL_PIXELS;

    // Old data (DATA_START_TRANSMISSION_1)
    send_command(Command::DATA_START_TRANSMISSION_1);
    for (std::size_t i = 0; i < total_pixels; ++i) {
      const auto byte1 = static_cast<std::uint8_t>(buffer[(i * 2)]);
      const auto byte2 = static_cast<std::uint8_t>(buffer[(i * 2) + 1]);
      const auto converted_pixel = convert_grayscale_pixel(byte1, byte2, true);
      send_data(converted_pixel);
    }

    // New data (DATA_START_TRANSMISSION_2)
    send_command(Command::DATA_START_TRANSMISSION_2);
    for (std::size_t i = 0; i < total_pixels; ++i) {
      const auto byte1 = static_cast<std::uint8_t>(buffer[(i * 2)]);
      const auto byte2 = static_cast<std::uint8_t>(buffer[(i * 2) + 1]);
      const auto converted_pixel = convert_grayscale_pixel(byte1, byte2, false);
      send_data(converted_pixel);
    }

    set_lut_grayscale();
    send_command(Command::DISPLAY_REFRESH);
    Device::delay_ms(Timing::DISPLAY_REFRESH_DELAY_MS);
    wait_busy();
  }
  return {};
}

auto EPD27::sleep() -> std::expected<void, Error> {
  if (is_asleep_) {
    return {}; // Already asleep, no-op
  }

  if (!initialized_) {
    return std::unexpected(Error(ErrorCode::DriverNotInitialized));
  }

  send_command(Command::VCOM_DATA_INTERVAL);
  send_data(DisplayOps::SLEEP_VCOM_DATA_INTERVAL);
  send_command(Command::POWER_OFF);
  send_command(Command::DEEP_SLEEP);
  send_data(DisplayOps::DEEP_SLEEP_MAGIC);

  is_asleep_ = true; // Track sleep state
  return {};
}

auto EPD27::wake() -> std::expected<void, Error> {
  if (!is_asleep_) {
    return {}; // Already awake, no-op
  }

  // EPD27 requires full re-initialization after deep sleep
  // This is a limitation of the hardware - it cannot wake from deep sleep
  // without re-initialization
  if (!initialized_) {
    return std::unexpected(Error(ErrorCode::DriverNotInitialized));
  }

  // Re-initialize with the current mode
  auto result = init(current_mode_);
  if (result) {
    is_asleep_ = false; // Track state - awake after successful init
  }
  return result;
}

auto EPD27::power_off() -> std::expected<void, Error> {
  if (!device_.is_initialized()) {
    return std::unexpected(Error(ErrorCode::DriverNotInitialized));
  }

  // Per datasheet: After POWER_OFF, BUSY_N drops LOW then rises HIGH when done.
  // We can't send GET_STATUS after POWER_OFF, so use simple BUSY polling.
  send_command(Command::POWER_OFF);
  wait_busy_simple(); // Use simple wait - don't send commands while display powers off

  return {};
}

auto EPD27::power_on() -> std::expected<void, Error> {
  if (!device_.is_initialized()) {
    return std::unexpected(Error(ErrorCode::DriverNotInitialized));
  }

  // Per datasheet: After POWER_ON, BUSY_N drops LOW then rises HIGH when done.
  // Similar to POWER_OFF, we use simple BUSY polling without GET_STATUS.
  send_command(Command::POWER_ON);
  wait_busy_simple(); // Use simple wait - display may not respond to commands during power-on

  return {};
}

} // namespace epaper
