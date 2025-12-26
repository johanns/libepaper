#include "epaper/draw.hpp"
#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace epaper {

Draw::Draw(Screen &screen) : screen_(screen) {}

auto Draw::draw_point(std::size_t x, std::size_t y, Color color, DotPixel pixel_size) -> void {
  const auto size = static_cast<std::size_t>(pixel_size);

  for (std::size_t dy = 0; dy < size; ++dy) {
    for (std::size_t dx = 0; dx < size; ++dx) {
      screen_.set_pixel(x + dx, y + dy, color);
    }
  }
}

auto Draw::draw_horizontal_line(std::size_t x_start, std::size_t x_end, std::size_t y, Color color, DotPixel width)
    -> void {
  if (x_start > x_end) {
    std::swap(x_start, x_end);
  }

  const auto line_width = static_cast<std::size_t>(width);
  for (std::size_t x = x_start; x <= x_end; ++x) {
    for (std::size_t w = 0; w < line_width; ++w) {
      screen_.set_pixel(x, y + w, color);
    }
  }
}

auto Draw::draw_vertical_line(std::size_t x, std::size_t y_start, std::size_t y_end, Color color, DotPixel width)
    -> void {
  if (y_start > y_end) {
    std::swap(y_start, y_end);
  }

  const auto line_width = static_cast<std::size_t>(width);
  for (std::size_t y = y_start; y <= y_end; ++y) {
    for (std::size_t w = 0; w < line_width; ++w) {
      screen_.set_pixel(x + w, y, color);
    }
  }
}

auto Draw::draw_line(std::size_t x_start, std::size_t y_start, std::size_t x_end, std::size_t y_end, Color color,
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

auto Draw::draw_rectangle(std::size_t x_start, std::size_t y_start, std::size_t x_end, std::size_t y_end, Color color,
                          DotPixel line_width, DrawFill fill) -> void {
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
        screen_.set_pixel(x, y, color);
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

auto Draw::draw_circle(std::size_t x_center, std::size_t y_center, std::size_t radius, Color color, DotPixel line_width,
                       DrawFill fill) -> void {
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

auto Draw::draw_char(std::size_t x, std::size_t y, char character, const Font &font, Color foreground, Color background)
    -> void {
  if (character < 0x20 || character > 0x7E) {
    return; // Only support printable ASCII
  }

  const auto char_data = font.char_data(character);
  const auto char_width = font.width();
  const auto char_height = font.height();
  const auto width_bytes = (char_width % 8 == 0) ? (char_width / 8) : (char_width / 8 + 1);

  for (std::uint16_t row = 0; row < char_height; ++row) {
    for (std::uint16_t col = 0; col < char_width; ++col) {
      const auto byte_index = row * width_bytes + col / 8;
      const auto bit_index = 7 - (col % 8);

      if (byte_index < char_data.size()) {
        const bool pixel_set = (char_data[byte_index] >> bit_index) & 0x01;
        const auto color = pixel_set ? foreground : background;
        screen_.set_pixel(x + col, y + row, color);
      }
    }
  }
}

auto Draw::draw_string(std::size_t x, std::size_t y, std::string_view text, const Font &font, Color foreground,
                       Color background) -> void {
  std::size_t current_x = x;

  for (const char c : text) {
    if (c == '\n') {
      current_x = x;
      y += font.height();
    } else if (c == '\r') {
      current_x = x;
    } else {
      draw_char(current_x, y, c, font, foreground, background);
      current_x += font.width();
    }
  }
}

auto Draw::draw_number(std::size_t x, std::size_t y, std::int32_t number, const Font &font, Color foreground,
                       Color background) -> void {
  const auto text = std::to_string(number);
  draw_string(x, y, text, font, foreground, background);
}

auto Draw::draw_decimal(std::size_t x, std::size_t y, double number, std::uint8_t decimal_places, const Font &font,
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

auto Draw::rgb_to_color(std::uint8_t r, std::uint8_t g, std::uint8_t b, DisplayMode mode) -> Color {
  // Convert RGB to grayscale using standard luminance formula
  const auto gray = static_cast<std::uint8_t>(0.299 * static_cast<double>(r) + 0.587 * static_cast<double>(g) +
                                              0.114 * static_cast<double>(b));

  if (mode == DisplayMode::BlackWhite) {
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

auto Draw::draw_bitmap(std::size_t x, std::size_t y, std::span<const Color> pixels, std::size_t bitmap_width,
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
      screen_.set_pixel(x + dst_x, y + dst_y, color);
    }
  }
}

auto Draw::draw_bitmap_from_file(std::size_t x, std::size_t y, std::string_view file_path, std::size_t target_width,
                                 std::size_t target_height) -> std::expected<void, BitmapError> {
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
    return std::unexpected(BitmapError::LoadFailed);
  }

  if (width <= 0 || height <= 0) {
    return std::unexpected(BitmapError::InvalidDimensions);
  }

  // Convert image data to Color array
  std::vector<Color> pixels;
  pixels.reserve(static_cast<std::size_t>(width) * static_cast<std::size_t>(height));

  const auto display_mode = screen_.mode();
  const auto pixel_count = static_cast<std::size_t>(width) * static_cast<std::size_t>(height);

  for (std::size_t i = 0; i < pixel_count; ++i) {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;

    const auto base_index = i * static_cast<std::size_t>(channels);

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

    pixels.push_back(rgb_to_color(r, g, b, display_mode));
  }

  // Draw the bitmap
  draw_bitmap(x, y, pixels, static_cast<std::size_t>(width), static_cast<std::size_t>(height), target_width,
              target_height);

  return {};
}

} // namespace epaper
