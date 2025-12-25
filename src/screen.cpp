#include "epaper/screen.hpp"
#include <algorithm>
#include <stdexcept>

namespace epaper {

Screen::Screen(Driver &driver)
    : driver_(driver), width_(driver.width()), height_(driver.height()), mode_(driver.mode()) {

  buffer_.resize(driver_.buffer_size());
  clear(Color::White);
}

auto Screen::calculate_bw_position(std::size_t x, std::size_t y) const -> std::pair<std::size_t, std::uint8_t> {
  const auto width_bytes = (width_ % 8 == 0) ? (width_ / 8) : (width_ / 8 + 1);
  const auto byte_index = x / 8 + y * width_bytes;
  const auto bit_offset = static_cast<std::uint8_t>(x % 8);
  return {byte_index, bit_offset};
}

auto Screen::calculate_gray_position(std::size_t x, std::size_t y) const -> std::pair<std::size_t, std::uint8_t> {
  const auto width_bytes = (width_ % 4 == 0) ? (width_ / 4) : (width_ / 4 + 1);
  const auto byte_index = x / 4 + y * width_bytes;
  const auto pixel_offset = static_cast<std::uint8_t>((x % 4) * 2);
  return {byte_index, pixel_offset};
}

auto Screen::set_pixel(std::size_t x, std::size_t y, Color color) -> void {
  if (x >= width_ || y >= height_) {
    return; // Silently ignore out-of-bounds
  }

  if (mode_ == DisplayMode::BlackWhite) {
    auto [byte_index, bit_offset] = calculate_bw_position(x, y);

    if (byte_index >= buffer_.size()) {
      return;
    }

    auto &byte_val = buffer_[byte_index];
    const std::uint8_t mask = 0x80 >> bit_offset;

    if (color == Color::White) {
      byte_val = static_cast<std::byte>(static_cast<std::uint8_t>(byte_val) | mask);
    } else {
      byte_val = static_cast<std::byte>(static_cast<std::uint8_t>(byte_val) & ~mask);
    }
  } else {
    // Grayscale mode
    auto [byte_index, pixel_offset] = calculate_gray_position(x, y);

    if (byte_index >= buffer_.size()) {
      return;
    }

    auto &byte_val = buffer_[byte_index];
    const std::uint8_t mask = 0xC0 >> pixel_offset;
    const std::uint8_t color_bits = (static_cast<std::uint8_t>(color) & 0xC0) >> pixel_offset;

    // Clear the 2 bits and set new color
    byte_val = static_cast<std::byte>((static_cast<std::uint8_t>(byte_val) & ~mask) | (color_bits & mask));
  }
}

auto Screen::get_pixel(std::size_t x, std::size_t y) const -> Color {
  if (x >= width_ || y >= height_) {
    return Color::White;
  }

  if (mode_ == DisplayMode::BlackWhite) {
    auto [byte_index, bit_offset] = calculate_bw_position(x, y);

    if (byte_index >= buffer_.size()) {
      return Color::White;
    }

    const auto byte_val = static_cast<std::uint8_t>(buffer_[byte_index]);
    const std::uint8_t mask = 0x80 >> bit_offset;

    return (byte_val & mask) ? Color::White : Color::Black;
  } else {
    // Grayscale mode
    auto [byte_index, pixel_offset] = calculate_gray_position(x, y);

    if (byte_index >= buffer_.size()) {
      return Color::White;
    }

    const auto byte_val = static_cast<std::uint8_t>(buffer_[byte_index]);
    const std::uint8_t mask = 0xC0 >> pixel_offset;
    const std::uint8_t color_bits = (byte_val & mask) << (6 - pixel_offset);

    return static_cast<Color>(color_bits);
  }
}

auto Screen::clear(Color color) -> void {
  if (mode_ == DisplayMode::BlackWhite) {
    const std::byte fill_byte = (color == Color::White) ? std::byte{0xFF} : std::byte{0x00};
    std::fill(buffer_.begin(), buffer_.end(), fill_byte);
  } else {
    // Grayscale: each byte contains 4 pixels (2 bits each)
    std::uint8_t fill_value = 0;
    const std::uint8_t color_byte = static_cast<std::uint8_t>(color);

    // Replicate the color 4 times in the byte
    for (int i = 0; i < 4; ++i) {
      fill_value |= ((color_byte >> 6) << (6 - i * 2));
    }

    std::fill(buffer_.begin(), buffer_.end(), static_cast<std::byte>(fill_value));
  }
}

auto Screen::clear_region(std::size_t x_start, std::size_t y_start, std::size_t x_end, std::size_t y_end, Color color)
    -> void {
  // Clamp to screen bounds
  x_start = std::min(x_start, width_);
  y_start = std::min(y_start, height_);
  x_end = std::min(x_end, width_);
  y_end = std::min(y_end, height_);

  for (std::size_t y = y_start; y < y_end; ++y) {
    for (std::size_t x = x_start; x < x_end; ++x) {
      set_pixel(x, y, color);
    }
  }
}

auto Screen::refresh() -> void { driver_.display(buffer_); }

auto Screen::set_mode(DisplayMode mode) -> void {
  if (mode != mode_) {
    mode_ = mode;
    buffer_.resize(driver_.buffer_size());
    clear(Color::White);
  }
}

} // namespace epaper
