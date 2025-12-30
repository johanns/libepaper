#include "epaper/display.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace epaper {

Display::Display(std::unique_ptr<Driver> driver, Orientation orientation, bool auto_sleep)
    : driver_(std::move(driver)), width_(driver_->width()), height_(driver_->height()), mode_(driver_->mode()),
      orientation_(orientation), auto_sleep_enabled_(auto_sleep) {

  buffer_.resize(driver_->buffer_size());
  clear(Color::White);
}

auto Display::effective_width() const noexcept -> std::size_t {
  // For 90° and 270° rotations, width and height are swapped
  if (orientation_ == Orientation::Landscape90 || orientation_ == Orientation::Landscape270) {
    return height_;
  }
  return width_;
}

auto Display::effective_height() const noexcept -> std::size_t {
  // For 90° and 270° rotations, width and height are swapped
  if (orientation_ == Orientation::Landscape90 || orientation_ == Orientation::Landscape270) {
    return width_;
  }
  return height_;
}

auto Display::transform_coordinates(std::size_t x, std::size_t y) const -> std::pair<std::size_t, std::size_t> {
  switch (orientation_) {
  case Orientation::Portrait0:
    // No transformation
    return {x, y};

  case Orientation::Landscape90:
    // Clockwise 90°: logical top-left -> physical top-right
    // As x increases (right in rotated), y increases physically (down)
    // As y increases (down in rotated), x decreases physically (left)
    return {width_ - 1 - y, x};

  case Orientation::Portrait180:
    // 180°: logical top-left -> physical bottom-right
    return {width_ - 1 - x, height_ - 1 - y};

  case Orientation::Landscape270:
    // Counter-clockwise 90°: logical top-left -> physical bottom-left
    // As x increases (right in rotated), y decreases physically (up)
    // As y increases (down in rotated), x increases physically (right)
    return {y, height_ - 1 - x};

  default:
    // Should never happen, but fallback to no transformation
    return {x, y};
  }
}

auto Display::calculate_bw_position(std::size_t x, std::size_t y) const -> std::pair<std::size_t, std::uint8_t> {
  const auto width_bytes = (width_ % 8 == 0) ? (width_ / 8) : (width_ / 8 + 1);
  const auto byte_index = x / 8 + y * width_bytes;
  const auto bit_offset = static_cast<std::uint8_t>(x % 8);
  return {byte_index, bit_offset};
}

auto Display::calculate_gray_position(std::size_t x, std::size_t y) const -> std::pair<std::size_t, std::uint8_t> {
  const auto width_bytes = (width_ % 4 == 0) ? (width_ / 4) : (width_ / 4 + 1);
  const auto byte_index = x / 4 + y * width_bytes;
  const auto pixel_offset = static_cast<std::uint8_t>((x % 4) * 2);
  return {byte_index, pixel_offset};
}

auto Display::set_pixel(std::size_t x, std::size_t y, Color color) -> void {
  // Check bounds against effective dimensions
  const auto eff_width = effective_width();
  const auto eff_height = effective_height();
  if (x >= eff_width || y >= eff_height) {
    return; // Silently ignore out-of-bounds
  }

  // Transform coordinates based on orientation
  auto [phys_x, phys_y] = transform_coordinates(x, y);

  if (mode_ == DisplayMode::BlackWhite) {
    auto [byte_index, bit_offset] = calculate_bw_position(phys_x, phys_y);

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
    auto [byte_index, pixel_offset] = calculate_gray_position(phys_x, phys_y);

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

auto Display::get_pixel(std::size_t x, std::size_t y) const -> Color {
  // Check bounds against effective dimensions
  const auto eff_width = effective_width();
  const auto eff_height = effective_height();
  if (x >= eff_width || y >= eff_height) {
    return Color::White;
  }

  // Transform coordinates based on orientation
  auto [phys_x, phys_y] = transform_coordinates(x, y);

  if (mode_ == DisplayMode::BlackWhite) {
    auto [byte_index, bit_offset] = calculate_bw_position(phys_x, phys_y);

    if (byte_index >= buffer_.size()) {
      return Color::White;
    }

    const auto byte_val = static_cast<std::uint8_t>(buffer_[byte_index]);
    const std::uint8_t mask = 0x80 >> bit_offset;

    return (byte_val & mask) ? Color::White : Color::Black;
  } else {
    // Grayscale mode
    auto [byte_index, pixel_offset] = calculate_gray_position(phys_x, phys_y);

    if (byte_index >= buffer_.size()) {
      return Color::White;
    }

    const auto byte_val = static_cast<std::uint8_t>(buffer_[byte_index]);
    const std::uint8_t mask = 0xC0 >> pixel_offset;
    const std::uint8_t color_bits = static_cast<std::uint8_t>((byte_val & mask) << (6U - pixel_offset));

    return static_cast<Color>(color_bits);
  }
}

auto Display::clear(Color color) -> void {
  if (mode_ == DisplayMode::BlackWhite) {
    const std::byte fill_byte = (color == Color::White) ? std::byte{0xFF} : std::byte{0x00};
    std::fill(buffer_.begin(), buffer_.end(), fill_byte);
  } else {
    // Grayscale: each byte contains 4 pixels (2 bits each)
    std::uint8_t fill_value = 0;
    const std::uint8_t color_byte = static_cast<std::uint8_t>(color);

    // Replicate the color 4 times in the byte
    for (std::uint8_t i = 0; i < 4; ++i) {
      fill_value |= ((color_byte >> 6) << (6 - static_cast<int>(i) * 2));
    }

    std::fill(buffer_.begin(), buffer_.end(), static_cast<std::byte>(fill_value));
  }
}

auto Display::clear_region(std::size_t x_start, std::size_t y_start, std::size_t x_end, std::size_t y_end, Color color)
    -> void {
  // Clamp to effective screen bounds (accounting for rotation)
  const auto eff_width = effective_width();
  const auto eff_height = effective_height();
  x_start = std::min(x_start, eff_width);
  y_start = std::min(y_start, eff_height);
  x_end = std::min(x_end, eff_width);
  y_end = std::min(y_end, eff_height);

  // Iterate through user coordinate space; set_pixel handles transformation
  for (std::size_t y = y_start; y < y_end; ++y) {
    for (std::size_t x = x_start; x < x_end; ++x) {
      set_pixel(x, y, color);
    }
  }
}

auto Display::refresh() -> std::expected<void, Error> {
  auto result = driver_->display(buffer_);
  if (!result) {
    return result;
  }
  if (auto_sleep_enabled_) {
    driver_->sleep();
  }
  return {};
}

auto Display::sleep() -> void { driver_->sleep(); }

auto Display::wake() -> std::expected<void, Error> { return driver_->wake(); }

auto Display::power_off() -> std::expected<void, Error> { return driver_->power_off(); }

auto Display::power_on() -> std::expected<void, Error> { return driver_->power_on(); }

// Moved to inline implementation in header (always returns true now)

auto Display::supports_power_control() const noexcept -> bool { return driver_->supports_power_control(); }

auto Display::draw_horizontal_line(std::size_t x_start, std::size_t x_end, std::size_t y, Color color, DotPixel width)
    -> void {
  if (x_start > x_end) {
    std::swap(x_start, x_end);
  }

  const auto line_width = static_cast<std::size_t>(width);
  for (std::size_t x = x_start; x <= x_end; ++x) {
    for (std::size_t w = 0; w < line_width; ++w) {
      set_pixel(x, y + w, color);
    }
  }
}

auto Display::draw_vertical_line(std::size_t x, std::size_t y_start, std::size_t y_end, Color color, DotPixel width)
    -> void {
  if (y_start > y_end) {
    std::swap(y_start, y_end);
  }

  const auto line_width = static_cast<std::size_t>(width);
  for (std::size_t y = y_start; y <= y_end; ++y) {
    for (std::size_t w = 0; w < line_width; ++w) {
      set_pixel(x + w, y, color);
    }
  }
}

auto Display::draw_point(std::size_t x, std::size_t y, Color color, DotPixel pixel_size) -> void {
  const auto size = static_cast<std::size_t>(pixel_size);

  for (std::size_t dy = 0; dy < size; ++dy) {
    for (std::size_t dx = 0; dx < size; ++dx) {
      set_pixel(x + dx, y + dy, color);
    }
  }
}

auto Display::draw_line(std::size_t x_start, std::size_t y_start, std::size_t x_end, std::size_t y_end, Color color,
                        DotPixel line_width, LineStyle style) -> void {
  // Bresenham's line algorithm
  const auto dx = static_cast<std::int32_t>(x_end > x_start ? x_end - x_start : x_start - x_end);
  const auto dy = static_cast<std::int32_t>(y_end > y_start ? y_end - y_start : y_start - y_end);

  const std::int32_t x_inc = x_start < x_end ? 1 : -1;
  const std::int32_t y_inc = y_start < y_end ? 1 : -1;

  std::int32_t esp = dx - dy;
  std::int32_t x = static_cast<std::int32_t>(x_start);
  std::int32_t y = static_cast<std::int32_t>(y_start);

  std::size_t dot_count = 0;

  while (true) {
    const bool should_draw = (style == LineStyle::Solid) || ((dot_count % 6) < 3);

    if (should_draw) {
      draw_point(static_cast<std::size_t>(x), static_cast<std::size_t>(y), color, line_width);
    }
    ++dot_count;

    if (x == static_cast<std::int32_t>(x_end) && y == static_cast<std::int32_t>(y_end)) {
      break;
    }

    const std::int32_t esp2 = 2 * esp;
    if (esp2 >= -dy) {
      esp -= dy;
      x += x_inc;
    }
    if (esp2 <= dx) {
      esp += dx;
      y += y_inc;
    }
  }
}

auto Display::draw_rectangle(std::size_t x_start, std::size_t y_start, std::size_t x_end, std::size_t y_end,
                             Color color, DotPixel line_width, DrawFill fill) -> void {
  if (x_start > x_end) {
    std::swap(x_start, x_end);
  }
  if (y_start > y_end) {
    std::swap(y_start, y_end);
  }

  if (fill == DrawFill::Full) {
    // Filled rectangle
    for (std::size_t y = y_start; y <= y_end; ++y) {
      for (std::size_t x = x_start; x <= x_end; ++x) {
        set_pixel(x, y, color);
      }
    }
  } else {
    // Hollow rectangle - draw four lines
    draw_horizontal_line(x_start, x_end, y_start, color, line_width);
    draw_horizontal_line(x_start, x_end, y_end, color, line_width);
    draw_vertical_line(x_start, y_start, y_end, color, line_width);
    draw_vertical_line(x_end, y_start, y_end, color, line_width);
  }
}

auto Display::draw_circle(std::size_t x_center, std::size_t y_center, std::size_t radius, Color color,
                          DotPixel line_width, DrawFill fill) -> void {
  if (radius == 0) {
    return;
  }

  // Midpoint circle algorithm
  std::int32_t x = 0;
  std::int32_t y = static_cast<std::int32_t>(radius);
  std::int32_t d = 3 - 2 * static_cast<std::int32_t>(radius);

  const auto cx = static_cast<std::int32_t>(x_center);
  const auto cy = static_cast<std::int32_t>(y_center);

  auto draw_circle_points = [&](std::int32_t px, std::int32_t py) {
    if (fill == DrawFill::Full) {
      // Draw horizontal lines to fill
      draw_horizontal_line(static_cast<std::size_t>(std::max(0, cx - px)), static_cast<std::size_t>(cx + px),
                           static_cast<std::size_t>(cy + py), color, line_width);
      draw_horizontal_line(static_cast<std::size_t>(std::max(0, cx - px)), static_cast<std::size_t>(cx + px),
                           static_cast<std::size_t>(cy - py), color, line_width);
    } else {
      // Draw 8 symmetric points
      draw_point(static_cast<std::size_t>(cx + px), static_cast<std::size_t>(cy + py), color, line_width);
      draw_point(static_cast<std::size_t>(cx - px), static_cast<std::size_t>(cy + py), color, line_width);
      draw_point(static_cast<std::size_t>(cx + px), static_cast<std::size_t>(cy - py), color, line_width);
      draw_point(static_cast<std::size_t>(cx - px), static_cast<std::size_t>(cy - py), color, line_width);
      draw_point(static_cast<std::size_t>(cx + py), static_cast<std::size_t>(cy + px), color, line_width);
      draw_point(static_cast<std::size_t>(cx - py), static_cast<std::size_t>(cy + px), color, line_width);
      draw_point(static_cast<std::size_t>(cx + py), static_cast<std::size_t>(cy - px), color, line_width);
      draw_point(static_cast<std::size_t>(cx - py), static_cast<std::size_t>(cy - px), color, line_width);
    }
  };

  while (x <= y) {
    draw_circle_points(x, y);

    if (d < 0) {
      d = d + 4 * x + 6;
    } else {
      d = d + 4 * (x - y) + 10;
      --y;
    }
    ++x;
  }
}

auto Display::draw_char(std::size_t x, std::size_t y, char character, const Font &font, Color foreground,
                        Color background) -> void {
  if (character < 0x20 || character > 0x7E) {
    return; // Only support printable ASCII
  }

  const auto char_data = font.char_data(character);
  const auto char_width = font.width();
  const auto char_height = font.height();
  const auto width_bytes = static_cast<std::size_t>((char_width % 8 == 0) ? (char_width / 8) : (char_width / 8 + 1));

  for (std::uint16_t row = 0; row < char_height; ++row) {
    for (std::uint16_t col = 0; col < char_width; ++col) {
      const auto byte_index = static_cast<std::size_t>(row) * width_bytes + static_cast<std::size_t>(col) / 8;
      const auto bit_index = 7U - static_cast<unsigned>(col % 8);

      if (byte_index < char_data.size()) {
        const bool pixel_set = (char_data[byte_index] >> bit_index) & 0x01;
        const auto color = pixel_set ? foreground : background;
        set_pixel(x + col, y + row, color);
      }
    }
  }
}

auto Display::draw_string(std::size_t x, std::size_t y, std::string_view text, const Font &font, Color foreground,
                          Color background) -> void {
  std::size_t current_x = x;
  std::size_t current_y = y;

  for (const char c : text) {
    if (c == '\n') {
      current_x = x;
      current_y += font.height();
    } else if (c == '\r') {
      current_x = x;
    } else {
      draw_char(current_x, current_y, c, font, foreground, background);
      current_x += font.width();
    }
  }
}

auto Display::draw_number(std::size_t x, std::size_t y, std::int32_t number, const Font &font, Color foreground,
                          Color background) -> void {
  const auto text = std::to_string(number);
  draw_string(x, y, text, font, foreground, background);
}

auto Display::draw_decimal(std::size_t x, std::size_t y, double number, std::uint8_t decimal_places, const Font &font,
                           Color foreground, Color background) -> void {
  // Simple decimal formatting
  const auto integer_part = static_cast<std::int64_t>(number);
  const auto fractional_part = std::abs(number - static_cast<double>(integer_part));

  auto text = std::to_string(integer_part) + ".";

  // Add decimal places
  double multiplier = 1.0;
  for (std::uint8_t i = 0; i < decimal_places; ++i) {
    multiplier *= 10.0;
  }

  const auto fraction_digits = static_cast<std::uint64_t>(fractional_part * multiplier);
  auto fraction_str = std::to_string(fraction_digits);

  // Pad with leading zeros if necessary
  while (fraction_str.length() < decimal_places) {
    fraction_str = "0" + fraction_str;
  }

  text += fraction_str;
  draw_string(x, y, text, font, foreground, background);
}

auto Display::rgb_to_color(std::uint8_t r, std::uint8_t g, std::uint8_t b) -> Color {
  // Convert RGB to grayscale using standard luminance formula
  const auto gray = static_cast<std::uint8_t>(0.299 * static_cast<double>(r) + 0.587 * static_cast<double>(g) +
                                              0.114 * static_cast<double>(b));

  if (mode_ == DisplayMode::BlackWhite) {
    // Simple threshold for black/white mode
    return gray >= 128 ? Color::White : Color::Black;
  } else {
    // 4-level grayscale mode
    if (gray >= 192) {
      return Color::White;
    } else if (gray >= 128) {
      return Color::Gray1;
    } else if (gray >= 64) {
      return Color::Gray2;
    } else {
      return Color::Black;
    }
  }
}

auto Display::draw_bitmap(std::size_t x, std::size_t y, std::span<const Color> pixels, std::size_t bitmap_width,
                          std::size_t bitmap_height, std::size_t target_width, std::size_t target_height) -> void {
  // Validate input dimensions
  if (bitmap_width == 0 || bitmap_height == 0) {
    return;
  }

  if (pixels.size() < bitmap_width * bitmap_height) {
    return; // Not enough pixel data
  }

  // If target dimensions are 0, use original dimensions (no scaling)
  if (target_width == 0) {
    target_width = bitmap_width;
  }
  if (target_height == 0) {
    target_height = bitmap_height;
  }

  // Draw with nearest-neighbor interpolation
  for (std::size_t dst_y = 0; dst_y < target_height; ++dst_y) {
    for (std::size_t dst_x = 0; dst_x < target_width; ++dst_x) {
      // Calculate source pixel coordinates using nearest-neighbor
      const auto src_x = (dst_x * bitmap_width) / target_width;
      const auto src_y = (dst_y * bitmap_height) / target_height;

      // Bounds check for source coordinates
      if (src_x >= bitmap_width || src_y >= bitmap_height) {
        continue;
      }

      // Get pixel from source bitmap
      const auto pixel_index = src_y * bitmap_width + src_x;
      if (pixel_index >= pixels.size()) {
        continue;
      }

      const auto color = pixels[pixel_index];

      // Draw pixel at destination coordinates (with automatic bounds checking in set_pixel)
      set_pixel(x + dst_x, y + dst_y, color);
    }
  }
}

auto Display::draw_bitmap_from_file(std::size_t x, std::size_t y, std::string_view file_path, std::size_t target_width,
                                    std::size_t target_height) -> std::expected<void, Error> {
  // Load image using stb_image
  int width = 0;
  int height = 0;
  int channels = 0;

  // Convert string_view to null-terminated string for stb_image
  const std::string path_str(file_path);

  // Load image with automatic channel detection
  std::unique_ptr<unsigned char, decltype(&stbi_image_free)> image_data(
      stbi_load(path_str.c_str(), &width, &height, &channels, 0), stbi_image_free);

  if (!image_data) {
    // Check if file exists or if it's a format error
    return std::unexpected(Error(ErrorCode::LoadFailed));
  }

  if (width <= 0 || height <= 0) {
    return std::unexpected(Error(ErrorCode::InvalidDimensions));
  }

  // Convert image data to Color array
  const auto width_size = static_cast<std::size_t>(width);
  const auto height_size = static_cast<std::size_t>(height);
  const auto channels_size = static_cast<std::size_t>(channels);
  std::vector<Color> pixels;
  pixels.reserve(width_size * height_size);

  const auto pixel_count = width_size * height_size;

  for (std::size_t i = 0; i < pixel_count; ++i) {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;

    const auto base_index = i * channels_size;

    if (channels == 1) {
      // Grayscale
      r = g = b = image_data.get()[base_index];
    } else if (channels == 2) {
      // Grayscale + Alpha (ignore alpha)
      r = g = b = image_data.get()[base_index];
    } else if (channels == 3) {
      // RGB
      r = image_data.get()[base_index];
      g = image_data.get()[base_index + 1];
      b = image_data.get()[base_index + 2];
    } else if (channels == 4) {
      // RGBA (ignore alpha)
      r = image_data.get()[base_index];
      g = image_data.get()[base_index + 1];
      b = image_data.get()[base_index + 2];
    }

    pixels.push_back(rgb_to_color(r, g, b));
  }

  // Draw the bitmap
  draw_bitmap(x, y, pixels, static_cast<std::size_t>(width), static_cast<std::size_t>(height), target_width,
              target_height);

  return {};
}

auto Display::save_framebuffer_to_bmp(std::string_view filename) -> std::expected<void, Error> {
  // Get effective dimensions (accounting for orientation)
  const auto width = effective_width();
  const auto height = effective_height();

  // BMP rows must be padded to 4-byte boundary
  const auto row_size = ((width * 3 + 3) / 4) * 4;
  const auto pixel_data_size = row_size * height;

  // BMP file header (14 bytes)
  const std::uint32_t file_size = static_cast<std::uint32_t>(14 + 40 + pixel_data_size);
  std::vector<std::uint8_t> bmp_data;
  bmp_data.reserve(file_size);

  // File header
  bmp_data.push_back('B');                                         // Signature
  bmp_data.push_back('M');                                         // Signature
  bmp_data.push_back(file_size & 0xFF);                            // File size (little-endian)
  bmp_data.push_back((file_size >> 8) & 0xFF);                     //
  bmp_data.push_back((file_size >> 16) & 0xFF);                    //
  bmp_data.push_back((file_size >> 24) & 0xFF);                    //
  bmp_data.push_back(0);                                           // Reserved
  bmp_data.push_back(0);                                           // Reserved
  bmp_data.push_back(0);                                           // Reserved
  bmp_data.push_back(0);                                           // Reserved
  bmp_data.push_back(54);                                          // Pixel data offset (14 + 40)
  bmp_data.push_back(0);                                           //
  bmp_data.push_back(0);                                           //
  bmp_data.push_back(0);                                           //

  // DIB header (BITMAPINFOHEADER, 40 bytes)
  bmp_data.push_back(40);                                          // Header size
  bmp_data.push_back(0);                                           //
  bmp_data.push_back(0);                                           //
  bmp_data.push_back(0);                                           //
  bmp_data.push_back(width & 0xFF);                                // Width (little-endian)
  bmp_data.push_back((width >> 8) & 0xFF);                         //
  bmp_data.push_back((width >> 16) & 0xFF);                        //
  bmp_data.push_back((width >> 24) & 0xFF);                        //
  bmp_data.push_back(height & 0xFF);                               // Height (little-endian)
  bmp_data.push_back((height >> 8) & 0xFF);                        //
  bmp_data.push_back((height >> 16) & 0xFF);                       //
  bmp_data.push_back((height >> 24) & 0xFF);                       //
  bmp_data.push_back(1);                                           // Color planes
  bmp_data.push_back(0);                                           //
  bmp_data.push_back(24);                                          // Bits per pixel (24-bit RGB)
  bmp_data.push_back(0);                                           //
  bmp_data.push_back(0);                                           // Compression (0 = none)
  bmp_data.push_back(0);                                           //
  bmp_data.push_back(0);                                           //
  bmp_data.push_back(0);                                           //
  bmp_data.push_back(pixel_data_size & 0xFF);                      // Image size
  bmp_data.push_back((pixel_data_size >> 8) & 0xFF);               //
  bmp_data.push_back((pixel_data_size >> 16) & 0xFF);              //
  bmp_data.push_back((pixel_data_size >> 24) & 0xFF);              //
  bmp_data.push_back(0x13);                                        // X pixels per meter (~72 DPI)
  bmp_data.push_back(0x0B);                                        //
  bmp_data.push_back(0);                                           //
  bmp_data.push_back(0);                                           //
  bmp_data.push_back(0x13);                                        // Y pixels per meter (~72 DPI)
  bmp_data.push_back(0x0B);                                        //
  bmp_data.push_back(0);                                           //
  bmp_data.push_back(0);                                           //
  bmp_data.push_back(0);                                           // Colors in palette (0 = default)
  bmp_data.push_back(0);                                           //
  bmp_data.push_back(0);                                           //
  bmp_data.push_back(0);                                           //
  bmp_data.push_back(0);                                           // Important colors (0 = all)
  bmp_data.push_back(0);                                           //
  bmp_data.push_back(0);                                           //
  bmp_data.push_back(0);                                           //

  // Pixel data (BGR format, bottom-to-top)
  // BMP format stores rows bottom-to-top, so we iterate y in reverse
  for (std::size_t y = height; y > 0; --y) {
    const auto current_y = y - 1;
    
    for (std::size_t x = 0; x < width; ++x) {
      // Get pixel color from framebuffer (with orientation transform)
      const auto color = get_pixel(x, current_y);
      
      // Convert Color enum to RGB values
      std::uint8_t r, g, b;
      switch (color) {
        case Color::White:
          r = g = b = 255;
          break;
        case Color::Gray1:
          r = g = b = 170;  // Light gray
          break;
        case Color::Gray2:
          r = g = b = 85;   // Dark gray
          break;
        case Color::Black:
        default:
          r = g = b = 0;
          break;
      }
      
      // BMP uses BGR format
      bmp_data.push_back(b);
      bmp_data.push_back(g);
      bmp_data.push_back(r);
    }
    
    // Pad row to 4-byte boundary
    const auto padding = row_size - (width * 3);
    for (std::size_t p = 0; p < padding; ++p) {
      bmp_data.push_back(0);
    }
  }

  // Write to file
  std::FILE* file = std::fopen(filename.data(), "wb");
  if (!file) {
    return std::unexpected(Error{ErrorCode::FileNotFound, 
                                 std::string("Failed to create BMP file: ") + std::string(filename)});
  }

  const auto bytes_written = std::fwrite(bmp_data.data(), 1, bmp_data.size(), file);
  std::fclose(file);

  if (bytes_written != bmp_data.size()) {
    return std::unexpected(Error{ErrorCode::LoadFailed, 
                                 "Failed to write complete BMP file"});
  }

  return {};
}

} // namespace epaper
