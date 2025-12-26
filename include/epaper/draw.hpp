#pragma once

#include "epaper/font.hpp"
#include "epaper/screen.hpp"
#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <string_view>

namespace epaper {

// Bitmap error types
enum class BitmapError { FileNotFound, InvalidFormat, LoadFailed, InvalidDimensions };

// Drawing styles
enum class DotPixel : std::uint8_t {
  Pixel1x1 = 1,
  Pixel2x2 = 2,
  Pixel3x3 = 3,
  Pixel4x4 = 4,
  Pixel5x5 = 5,
  Pixel6x6 = 6,
  Pixel7x7 = 7,
  Pixel8x8 = 8
};

enum class LineStyle { Solid, Dotted };

enum class DrawFill { Empty, Full };

// High-level drawing API
class Draw {
public:
  explicit Draw(Screen &screen);

  // Point drawing
  auto draw_point(std::size_t x, std::size_t y, Color color, DotPixel pixel_size = DotPixel::Pixel1x1) -> void;

  // Line drawing
  auto draw_line(std::size_t x_start, std::size_t y_start, std::size_t x_end, std::size_t y_end, Color color,
                 DotPixel line_width = DotPixel::Pixel1x1, LineStyle style = LineStyle::Solid) -> void;

  // Rectangle drawing
  auto draw_rectangle(std::size_t x_start, std::size_t y_start, std::size_t x_end, std::size_t y_end, Color color,
                      DotPixel line_width = DotPixel::Pixel1x1, DrawFill fill = DrawFill::Empty) -> void;

  // Circle drawing
  auto draw_circle(std::size_t x_center, std::size_t y_center, std::size_t radius, Color color,
                   DotPixel line_width = DotPixel::Pixel1x1, DrawFill fill = DrawFill::Empty) -> void;

  // Text drawing
  auto draw_char(std::size_t x, std::size_t y, char character, const Font &font, Color foreground, Color background)
      -> void;

  auto draw_string(std::size_t x, std::size_t y, std::string_view text, const Font &font, Color foreground,
                   Color background) -> void;

  // Number drawing
  auto draw_number(std::size_t x, std::size_t y, std::int32_t number, const Font &font, Color foreground,
                   Color background) -> void;

  auto draw_decimal(std::size_t x, std::size_t y, double number, std::uint8_t decimal_places, const Font &font,
                    Color foreground, Color background) -> void;

  // Bitmap drawing
  auto draw_bitmap(std::size_t x, std::size_t y, std::span<const Color> pixels, std::size_t bitmap_width,
                   std::size_t bitmap_height, std::size_t target_width = 0, std::size_t target_height = 0) -> void;

  auto draw_bitmap_from_file(std::size_t x, std::size_t y, std::string_view file_path, std::size_t target_width = 0,
                             std::size_t target_height = 0) -> std::expected<void, BitmapError>;

private:
  // Helper: convert RGB/RGBA to Color enum
  auto rgb_to_color(std::uint8_t r, std::uint8_t g, std::uint8_t b, DisplayMode mode) -> Color;
  // Helper: draw horizontal line
  auto draw_horizontal_line(std::size_t x_start, std::size_t x_end, std::size_t y, Color color, DotPixel width) -> void;

  // Helper: draw vertical line
  auto draw_vertical_line(std::size_t x, std::size_t y_start, std::size_t y_end, Color color, DotPixel width) -> void;

  Screen &screen_;
};

} // namespace epaper
