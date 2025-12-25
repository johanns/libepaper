#include "epaper/drivers/epd27.hpp"
#include <algorithm>

namespace epaper {

EPD27::EPD27(Device &device) : device_(device) {}

auto EPD27::buffer_size() const noexcept -> std::size_t {
  if (current_mode_ == DisplayMode::BlackWhite) {
    // 1 bit per pixel, packed into bytes
    const auto width_bytes = (WIDTH % 8 == 0) ? (WIDTH / 8) : (WIDTH / 8 + 1);
    return width_bytes * HEIGHT;
  } else {
    // 2 bits per pixel for 4-level grayscale
    const auto width_bytes = (WIDTH % 4 == 0) ? (WIDTH / 4) : (WIDTH / 4 + 1);
    return width_bytes * HEIGHT;
  }
}

auto EPD27::reset() -> void {
  device_.write_pin(pins::RST, true);
  Device::delay_ms(200);
  device_.write_pin(pins::RST, false);
  Device::delay_ms(2);
  device_.write_pin(pins::RST, true);
  Device::delay_ms(200);
}

auto EPD27::send_command(std::uint8_t command) -> void {
  device_.write_pin(pins::DC, false); // Command mode
  device_.write_pin(pins::CS, false); // Select device
  device_.spi_transfer(command);
  device_.write_pin(pins::CS, true); // Deselect device
}

auto EPD27::send_data(std::uint8_t data) -> void {
  device_.write_pin(pins::DC, true);  // Data mode
  device_.write_pin(pins::CS, false); // Select device
  device_.spi_transfer(data);
  device_.write_pin(pins::CS, true); // Deselect device
}

auto EPD27::wait_busy() -> void {
  bool busy = false;
  do {
    send_command(0x71); // Get status command
    busy = device_.read_pin(pins::BUSY);
    busy = !(busy & 0x01);
  } while (busy);
  Device::delay_ms(200);
}

auto EPD27::set_lut_bw() -> void {
  // VCOM LUT
  send_command(0x20);
  for (const auto byte : LUT_VCOM_DC) {
    send_data(byte);
  }

  // WW LUT
  send_command(0x21);
  for (const auto byte : LUT_WW) {
    send_data(byte);
  }

  // BW LUT
  send_command(0x22);
  for (const auto byte : LUT_BW) {
    send_data(byte);
  }

  // WB LUT
  send_command(0x23);
  for (const auto byte : LUT_BB) {
    send_data(byte);
  }

  // BB LUT
  send_command(0x24);
  for (const auto byte : LUT_WB) {
    send_data(byte);
  }
}

auto EPD27::set_lut_grayscale() -> void {
  // VCOM LUT
  send_command(0x20);
  for (const auto byte : LUT_VCOM_GRAY) {
    send_data(byte);
  }

  // WW LUT
  send_command(0x21);
  for (const auto byte : LUT_WW_GRAY) {
    send_data(byte);
  }

  // BW LUT
  send_command(0x22);
  for (const auto byte : LUT_BW_GRAY) {
    send_data(byte);
  }

  // WB LUT
  send_command(0x23);
  for (const auto byte : LUT_WB_GRAY) {
    send_data(byte);
  }

  // BB LUT
  send_command(0x24);
  for (const auto byte : LUT_BB_GRAY) {
    send_data(byte);
  }

  // Additional LUT for grayscale
  send_command(0x25);
  for (const auto byte : LUT_WW_GRAY) {
    send_data(byte);
  }
}

auto EPD27::init(DisplayMode mode) -> std::expected<void, DriverError> {
  if (!device_.is_initialized()) {
    return std::unexpected(DriverError::NotInitialized);
  }

  current_mode_ = mode;
  reset();

  if (mode == DisplayMode::BlackWhite) {
    // Black/White mode initialization
    send_command(0x01); // POWER_SETTING
    send_data(0x03);    // VDS_EN, VDG_EN
    send_data(0x00);    // VCOM_HV, VGHL_LV[1], VGHL_LV[0]
    send_data(0x2b);    // VDH
    send_data(0x2b);    // VDL
    send_data(0x09);    // VDHR

    send_command(0x06); // BOOSTER_SOFT_START
    send_data(0x07);
    send_data(0x07);
    send_data(0x17);

    // Power optimization
    send_command(0xF8);
    send_data(0x60);
    send_data(0xA5);

    send_command(0xF8);
    send_data(0x89);
    send_data(0xA5);

    send_command(0xF8);
    send_data(0x90);
    send_data(0x00);

    send_command(0xF8);
    send_data(0x93);
    send_data(0x2A);

    send_command(0xF8);
    send_data(0xA0);
    send_data(0xA5);

    send_command(0xF8);
    send_data(0xA1);
    send_data(0x00);

    send_command(0xF8);
    send_data(0x73);
    send_data(0x41);

    send_command(0x16); // PARTIAL_DISPLAY_REFRESH
    send_data(0x00);

    send_command(0x04); // POWER_ON
    wait_busy();

    send_command(0x00); // PANEL_SETTING
    send_data(0xAF);    // KW-BF   KWR-AF    BWROTP 0f

    send_command(0x30); // PLL_CONTROL
    send_data(0x3A);    // 3A 100HZ   29 150Hz 39 200HZ    31 171HZ

    send_command(0x82); // VCM_DC_SETTING_REGISTER
    send_data(0x12);

    set_lut_bw();
  } else {
    // Grayscale mode initialization
    send_command(0x01); // POWER SETTING
    send_data(0x03);
    send_data(0x00);
    send_data(0x2b);
    send_data(0x2b);

    send_command(0x06); // booster soft start
    send_data(0x07);
    send_data(0x07);
    send_data(0x17);

    send_command(0xF8); // boost
    send_data(0x60);
    send_data(0xA5);

    send_command(0xF8);
    send_data(0x89);
    send_data(0xA5);

    send_command(0xF8);
    send_data(0x90);
    send_data(0x00);

    send_command(0xF8);
    send_data(0x93);
    send_data(0x2A);

    send_command(0xF8);
    send_data(0xa0);
    send_data(0xa5);

    send_command(0xF8);
    send_data(0xa1);
    send_data(0x00);

    send_command(0xF8);
    send_data(0x73);
    send_data(0x41);

    send_command(0x16);
    send_data(0x00);

    send_command(0x04);
    wait_busy();

    send_command(0x00); // panel setting
    send_data(0xbf);    // KW-BF   KWR-AF    BWROTP 0f

    send_command(0x30); // PLL setting
    send_data(0x90);    // 100hz

    send_command(0x61); // resolution setting
    send_data(0x00);    // 176
    send_data(0xb0);
    send_data(0x01); // 264
    send_data(0x08);

    send_command(0x82); // vcom_DC setting
    send_data(0x12);

    send_command(0X50); // VCOM AND DATA INTERVAL SETTING
    send_data(0x97);
  }

  initialized_ = true;
  return {};
}

auto EPD27::clear() -> void {
  const auto width_bytes = (WIDTH % 8 == 0) ? (WIDTH / 8) : (WIDTH / 8 + 1);

  send_command(0x10);
  for (std::size_t j = 0; j < HEIGHT; ++j) {
    for (std::size_t i = 0; i < width_bytes; ++i) {
      send_data(0xFF);
    }
  }

  send_command(0x13);
  for (std::size_t j = 0; j < HEIGHT; ++j) {
    for (std::size_t i = 0; i < width_bytes; ++i) {
      send_data(0xFF);
    }
  }

  send_command(0x12); // DISPLAY_REFRESH
  wait_busy();
}

auto EPD27::display(std::span<const std::byte> buffer) -> void {
  if (current_mode_ == DisplayMode::BlackWhite) {
    const auto width_bytes = (WIDTH % 8 == 0) ? (WIDTH / 8) : (WIDTH / 8 + 1);

    send_command(0x13);
    for (std::size_t j = 0; j < HEIGHT; ++j) {
      for (std::size_t i = 0; i < width_bytes; ++i) {
        const auto index = i + j * width_bytes;
        send_data(static_cast<std::uint8_t>(buffer[index]));
      }
    }
    send_command(0x12); // DISPLAY_REFRESH
    wait_busy();
  } else {
    // Grayscale display
    constexpr std::size_t total_pixels = 5808; // (176 * 264) / 8

    // Old data
    send_command(0x10);
    for (std::size_t i = 0; i < total_pixels; ++i) {
      std::uint8_t temp3 = 0;
      for (std::size_t j = 0; j < 2; ++j) {
        std::uint8_t temp1 = static_cast<std::uint8_t>(buffer[i * 2 + j]);
        for (std::size_t k = 0; k < 2; ++k) {
          const std::uint8_t temp2 = temp1 & 0xC0;
          if (temp2 == 0xC0)
            temp3 |= 0x01; // white
          else if (temp2 == 0x00)
            temp3 |= 0x00; // black
          else if (temp2 == 0x80)
            temp3 |= 0x01; // gray1
          else             // 0x40
            temp3 |= 0x00; // gray2
          temp3 <<= 1;

          temp1 <<= 2;
          const std::uint8_t temp2_2 = temp1 & 0xC0;
          if (temp2_2 == 0xC0)
            temp3 |= 0x01;
          else if (temp2_2 == 0x00)
            temp3 |= 0x00;
          else if (temp2_2 == 0x80)
            temp3 |= 0x01;
          else
            temp3 |= 0x00;
          if (j != 1 || k != 1)
            temp3 <<= 1;

          temp1 <<= 2;
        }
      }
      send_data(temp3);
    }

    // New data
    send_command(0x13);
    for (std::size_t i = 0; i < total_pixels; ++i) {
      std::uint8_t temp3 = 0;
      for (std::size_t j = 0; j < 2; ++j) {
        std::uint8_t temp1 = static_cast<std::uint8_t>(buffer[i * 2 + j]);
        for (std::size_t k = 0; k < 2; ++k) {
          const std::uint8_t temp2 = temp1 & 0xC0;
          if (temp2 == 0xC0)
            temp3 |= 0x01; // white
          else if (temp2 == 0x00)
            temp3 |= 0x00; // black
          else if (temp2 == 0x80)
            temp3 |= 0x00; // gray1
          else             // 0x40
            temp3 |= 0x01; // gray2
          temp3 <<= 1;

          temp1 <<= 2;
          const std::uint8_t temp2_2 = temp1 & 0xC0;
          if (temp2_2 == 0xC0)
            temp3 |= 0x01;
          else if (temp2_2 == 0x00)
            temp3 |= 0x00;
          else if (temp2_2 == 0x80)
            temp3 |= 0x00;
          else
            temp3 |= 0x01;
          if (j != 1 || k != 1)
            temp3 <<= 1;

          temp1 <<= 2;
        }
      }
      send_data(temp3);
    }

    set_lut_grayscale();
    send_command(0x12);
    Device::delay_ms(200);
    wait_busy();
  }
}

auto EPD27::sleep() -> void {
  send_command(0X50);
  send_data(0xf7);
  send_command(0X02); // power off
  send_command(0X07); // deep sleep
  send_data(0xA5);
}

} // namespace epaper
