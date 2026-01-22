#pragma once

#include "epaper/core/framebuffer_concepts.hpp"
#include "epaper/core/geometry.hpp"
#include "epaper/core/types.hpp"
#include "epaper/graphics/font.hpp"
#include <cmath>
#include <cstddef>
#include <string_view>

namespace epaper {

/**
 * @brief Graphics drawing primitives.
 *
 * Stateless drawing functions that operate on any FramebufferLike type using
 * C++20/23 concepts for compile-time polymorphism. Implements standard computer
 * graphics algorithms optimized for integer-only e-paper displays.
 *
 * **Algorithms Implemented:**
 * - Lines: Bresenham's line algorithm (integer-only, no floating point)
 * - Circles: Midpoint circle algorithm (Bresenham's circle)
 * - Rectangles: Edge decomposition into lines
 * - Text: Bitmap font rasterization
 * - Bitmaps: Nearest-neighbor scaling
 *
 * **Performance Characteristics:**
 * - All algorithms use integer arithmetic only (no FPU required)
 * - Bresenham avoids division in inner loops
 * - Suitable for embedded systems with limited CPU
 *
 * **Design Philosophy:**
 * - Stateless: No internal state, all parameters explicit
 * - Generic: Works with any FramebufferLike implementation
 * - Header-only: Template instantiation at compile time
 * - Orientation-aware: Delegates coordinate transforms to framebuffer
 *
 * @example
 * ```cpp
 * MonoFramebuffer fb{176, 264, DisplayMode::BlackWhite};
 *
 * // Draw line using Bresenham's algorithm
 * Graphics::draw_line(fb, {0, 0}, {100, 100},
 *                     LineStyle::Solid, Color::Black,
 *                     Orientation::Portrait0);
 *
 * // Draw filled circle using midpoint algorithm
 * Graphics::draw_circle(fb, {88, 132}, 50,
 *                       LineStyle::Solid, Color::Black,
 *                       DrawFill::Full, Orientation::Portrait0);
 *
 * // Draw rectangle
 * Graphics::draw_rectangle(fb, {10, 10}, {60, 60},
 *                          LineStyle::Solid, Color::Black,
 *                          DrawFill::Empty, Orientation::Portrait0);
 *
 * // Render text
 * Font font = Font::load("font.bdf");
 * Graphics::draw_text(fb, {10, 70}, "Hello!", font,
 *                     Color::Black, Color::White,
 *                     Orientation::Portrait0);
 * ```
 *
 * @see FramebufferLike, Font, Point
 */
class Graphics {
public:
  /**
   * @brief Draw a line on the framebuffer.
   *
   * @tparam FB FramebufferLike type
   * @param fb Target framebuffer
   * @param start Start point
   * @param end End point
   * @param style Line style (Solid, Dotted, etc.)
   * @param color Line color
   * @param orientation Coordinate orientation
   */
  template <FramebufferLike FB>
  static auto draw_line(FB &fb, Point start, Point end, LineStyle style, Color color, Orientation orientation) -> void;

  /**
   * @brief Draw a rectangle on the framebuffer.
   *
   * @tparam FB FramebufferLike type
   * @param fb Target framebuffer
   * @param top_left Top-left corner
   * @param bottom_right Bottom-right corner
   * @param style Border style
   * @param color Rectangle color
   * @param fill Fill mode
   * @param orientation Coordinate orientation
   */
  template <FramebufferLike FB>
  static auto draw_rectangle(FB &fb, Point top_left, Point bottom_right, LineStyle style, Color color, DrawFill fill,
                             Orientation orientation) -> void;

  /**
   * @brief Draw a circle on the framebuffer.
   *
   * @tparam FB FramebufferLike type
   * @param fb Target framebuffer
   * @param center Center point
   * @param radius Radius in pixels
   * @param style Border style
   * @param color Circle color
   * @param fill Fill mode
   * @param orientation Coordinate orientation
   */
  template <FramebufferLike FB>
  static auto draw_circle(FB &fb, Point center, std::size_t radius, LineStyle style, Color color, DrawFill fill,
                          Orientation orientation) -> void;

  /**
   * @brief Draw text string on the framebuffer.
   *
   * @tparam FB FramebufferLike type
   * @param fb Target framebuffer
   * @param pos Top-left position of text
   * @param text Text content
   * @param font Font to use
   * @param foreground Text color
   * @param background Background color (Color::None for transparent)
   * @param orientation Coordinate orientation
   */
  template <FramebufferLike FB>
  static auto draw_text(FB &fb, Point pos, std::string_view text, const Font &font, Color foreground, Color background,
                        Orientation orientation) -> void;

  /**
   * @brief Draw a bitmap on the framebuffer.
   *
   * @tparam FB FramebufferLike type
   * @param fb Target framebuffer
   * @param pos Top-left position
   * @param data Raw bitmap data
   * @param w Source width
   * @param h Source height
   * @param target_w Target width (scaling)
   * @param target_h Target height (scaling)
   * @param orientation Coordinate orientation
   */
  template <FramebufferLike FB>
  static auto draw_bitmap(FB &fb, Point pos, std::span<const std::uint8_t> data, std::size_t w, std::size_t h,
                          std::size_t target_w, std::size_t target_h, Orientation orientation) -> void;
};

// ========== Template Implementations ==========

/**
 * @brief Bresenham's line algorithm implementation.
 *
 * Classic integer-only line drawing algorithm. Efficiently determines which
 * pixels to illuminate on a raster display to approximate a straight line
 * between two points.
 *
 * **Algorithm Properties:**
 * - No floating point arithmetic required
 * - No division in inner loop (only initialization)
 * - Symmetric - same pixels regardless of direction
 * - Works for all octants (0°-360°)
 *
 * **Dotted Line Support:**
 * - Alternates pixels on/off based on step count
 * - Pattern: pixel, skip, pixel, skip, ...
 *
 * Reference: Bresenham, J. E. (1965). "Algorithm for computer control of a
 * digital plotter". IBM Systems Journal. 4 (1): 25–30.
 */
template <FramebufferLike FB>
auto Graphics::draw_line(FB &fb, Point start, Point end, LineStyle style, Color color, Orientation orientation)
    -> void {
  // Initialize Bresenham variables
  int x0 = static_cast<int>(start.x);
  int y0 = static_cast<int>(start.y);
  int x1 = static_cast<int>(end.x);
  int y1 = static_cast<int>(end.y);

  int dx = std::abs(x1 - x0);  // Absolute delta X
  int dy = std::abs(y1 - y0);  // Absolute delta Y
  int sx = (x0 < x1) ? 1 : -1; // Step direction X
  int sy = (y0 < y1) ? 1 : -1; // Step direction Y
  int err = dx - dy;           // Error accumulator

  int step_count = 0;
  while (true) {
    // Draw pixel only if solid line, or if dotted and on even step
    if (style == LineStyle::Solid || (step_count % 2 == 0)) {
      fb.set_pixel(static_cast<std::size_t>(x0), static_cast<std::size_t>(y0), color, orientation);
    }

    // Reached endpoint - line complete
    if (x0 == x1 && y0 == y1) {
      break;
    }

    // Bresenham error adjustment and step
    int e2 = 2 * err;
    if (e2 > -dy) {
      err -= dy;
      x0 += sx; // Step in X direction
    }
    if (e2 < dx) {
      err += dx;
      y0 += sy; // Step in Y direction
    }
    ++step_count; // Track step for dotted pattern
  }
}

/**
 * @brief Rectangle drawing via edge decomposition.
 *
 * Draws rectangle as 4 separate lines (top, right, bottom, left edges).
 * For filled rectangles, draws horizontal scan lines between left and right edges.
 *
 * **Algorithm:**
 * 1. Draw 4 border lines
 * 2. If fill requested, draw horizontal lines for interior
 *
 * **Optimization Note:**
 * For filled rectangles, could use faster set_pixel loops instead of draw_line,
 * but current approach reuses line drawing logic for consistency.
 */
template <FramebufferLike FB>
auto Graphics::draw_rectangle(FB &fb, Point top_left, Point bottom_right, LineStyle style, Color color, DrawFill fill,
                              Orientation orientation) -> void {
  // Draw four edges
  draw_line(fb, top_left, {bottom_right.x, top_left.y}, style, color, orientation);     // Top edge
  draw_line(fb, {bottom_right.x, top_left.y}, bottom_right, style, color, orientation); // Right edge
  draw_line(fb, bottom_right, {top_left.x, bottom_right.y}, style, color, orientation); // Bottom edge
  draw_line(fb, {top_left.x, bottom_right.y}, top_left, style, color, orientation);     // Left edge

  // Fill interior with horizontal scan lines (skips edge pixels to avoid overdraw)
  if (fill == DrawFill::Full) {
    for (std::size_t y = top_left.y + 1; y < bottom_right.y; ++y) {
      draw_line(fb, {top_left.x + 1, y}, {bottom_right.x - 1, y}, style, color, orientation);
    }
  }
}

/**
 * @brief Midpoint circle algorithm (Bresenham's circle).
 *
 * Efficient integer-only circle drawing using 8-way symmetry.
 * Calculates one octant and mirrors to draw all 8 octants simultaneously.
 *
 * **Algorithm Properties:**
 * - No floating point (no trigonometry, no sqrt)
 * - No multiplication in inner loop
 * - 8-fold symmetry reduces computations by 87.5%
 * - Incremental error calculation
 *
 * **Octant Symmetry:**
 * Computing (x, y) for one octant gives 8 points:
 * (±x, ±y) and (±y, ±x)
 *
 * **Fill Implementation:**
 * For filled circles, draws horizontal scan lines between symmetric points
 * in each octant, creating solid interior.
 *
 * Reference: Derived from Bresenham's line algorithm, adapted for circles.
 */
template <FramebufferLike FB>
auto Graphics::draw_circle(FB &fb, Point center, std::size_t radius, LineStyle /*style*/, Color color, DrawFill fill,
                           Orientation orientation) -> void {
  int x = static_cast<int>(radius);
  int y = 0;
  int err = 0; // Error accumulator for midpoint decision

  // Lambda to plot all 8 octants using symmetry
  // Given (x,y) relative to center, plot (±x,±y) and (±y,±x)
  auto plot_points = [&](int cx, int cy, int px, int py) {
    // Quadrant I and II (y-axis symmetry)
    fb.set_pixel(static_cast<std::size_t>(cx + px), static_cast<std::size_t>(cy + py), color, orientation);
    fb.set_pixel(static_cast<std::size_t>(cx - px), static_cast<std::size_t>(cy + py), color, orientation);
    // Quadrant III and IV (x-axis symmetry)
    fb.set_pixel(static_cast<std::size_t>(cx + px), static_cast<std::size_t>(cy - py), color, orientation);
    fb.set_pixel(static_cast<std::size_t>(cx - px), static_cast<std::size_t>(cy - py), color, orientation);
    // 45-degree rotated points (octant reflection)
    fb.set_pixel(static_cast<std::size_t>(cx + py), static_cast<std::size_t>(cy + px), color, orientation);
    fb.set_pixel(static_cast<std::size_t>(cx - py), static_cast<std::size_t>(cy + px), color, orientation);
    fb.set_pixel(static_cast<std::size_t>(cx + py), static_cast<std::size_t>(cy - px), color, orientation);
    fb.set_pixel(static_cast<std::size_t>(cx - py), static_cast<std::size_t>(cy - px), color, orientation);
  };

  // Lambda to fill horizontal scan lines for solid circle
  // For each Y level (py), draw horizontal line from -px to +px
  // Also fill the 90-degree rotated spans (using py as X range)
  auto fill_horizontal_line = [&](int cx, int cy, int px, int py) {
    if (fill == DrawFill::Full) {
      // Fill horizontal spans at y = ±py
      // Range: x ∈ [-px, +px] (covers full width at this Y level)
      for (int fx = -px; fx <= px; ++fx) {
        fb.set_pixel(static_cast<std::size_t>(cx + fx), static_cast<std::size_t>(cy + py), color, orientation);
        fb.set_pixel(static_cast<std::size_t>(cx + fx), static_cast<std::size_t>(cy - py), color, orientation);
      }
      // Fill horizontal spans at y = ±px (rotated 90 degrees)
      // Range: x ∈ [-py, +py] (covers narrower width at steeper Y)
      for (int fx = -py; fx <= py; ++fx) {
        fb.set_pixel(static_cast<std::size_t>(cx + fx), static_cast<std::size_t>(cy + px), color, orientation);
        fb.set_pixel(static_cast<std::size_t>(cx + fx), static_cast<std::size_t>(cy - px), color, orientation);
      }
    }
  };

  // Midpoint circle algorithm main loop
  // Start at (radius, 0) and walk counterclockwise to 45° line (x == y)
  // Only compute one octant; symmetry gives remaining 7 octants
  while (x >= y) {
    plot_points(static_cast<int>(center.x), static_cast<int>(center.y), x, y);
    fill_horizontal_line(static_cast<int>(center.x), static_cast<int>(center.y), x, y);

    // Decision parameter for next pixel
    // err represents distance from ideal circle: err ≈ x² + y² - r²
    if (err <= 0) {
      y += 1;           // Move up (increasing Y)
      err += 2 * y + 1; // Update error: Δerr = (y+1)² - y² = 2y + 1
    }
    if (err > 0) {
      x -= 1;           // Move left (decreasing X)
      err -= 2 * x + 1; // Update error: Δerr = x² - (x-1)² = 2x - 1
    }
  }
}

template <FramebufferLike FB>
auto Graphics::draw_text(FB &fb, Point pos, std::string_view text, const Font &font, Color foreground, Color background,
                         Orientation orientation) -> void {
  std::size_t cursor_x = pos.x; // Current X position for next character
  std::size_t cursor_y = pos.y; // Baseline Y position

  // Font metrics define character cell dimensions
  const auto &metrics = font.metrics();
  const auto font_width = metrics.width;   // Character width in pixels
  const auto font_height = metrics.height; // Character height in pixels

  // Calculate bytes per row for character bitmap (MSB-first packing)
  // Example: 12-pixel width needs 2 bytes (12/8 rounded up)
  const auto width_bytes = (font_width % 8 == 0) ? (font_width / 8) : ((font_width / 8) + 1);

  // Render each character sequentially
  for (char c : text) {
    // Fetch character bitmap (may be empty for unsupported chars)
    auto bitmap = font.char_data(c);
    if (bitmap.empty()) {
      continue; // Skip unsupported characters
    }

    // Rasterize character bitmap row-by-row, pixel-by-pixel
    for (std::size_t j = 0; j < font_height; ++j) {
      for (std::size_t i = 0; i < font_width; ++i) {
        // Calculate byte index for MSB-first bitmap layout
        // Bitmap format: [row0_byte0, row0_byte1, ..., row1_byte0, ...]
        std::size_t byte_idx = (j * width_bytes) + (i / 8);
        if (byte_idx >= bitmap.size()) {
          break; // Malformed font data - abort this character
        }

        // Extract bit from bitmap byte (MSB-first: bit 7 = leftmost pixel)
        std::uint8_t byte = bitmap[byte_idx];
        bool is_set = (byte & (0x80 >> (i % 8))) != 0;

        // Map bitmap bit to foreground/background color
        // 1 = foreground (ink), 0 = background (paper)
        Color pixel_color = is_set ? foreground : background;
        fb.set_pixel(cursor_x + i, cursor_y + j, pixel_color, orientation);
      }
    }

    cursor_x += font_width; // Advance to next character cell
  }
}

template <FramebufferLike FB>
auto Graphics::draw_bitmap(FB &fb, Point pos, std::span<const std::uint8_t> data, std::size_t w, std::size_t h,
                           std::size_t target_w, std::size_t target_h, Orientation orientation) -> void {
  // Determine target dimensions (default to source size if not specified)
  std::size_t tw = (target_w > 0) ? target_w : w;
  std::size_t th = (target_h > 0) ? target_h : h;

  // Calculate scaling factors (nearest-neighbor sampling)
  // scale_x/y represent source pixels per target pixel
  // Example: source=200px, target=100px → scale=2.0 (sample every 2nd pixel)
  float scale_x = static_cast<float>(w) / static_cast<float>(tw);
  float scale_y = static_cast<float>(h) / static_cast<float>(th);

  // Iterate over target (output) dimensions
  for (std::size_t y = 0; y < th; ++y) {
    for (std::size_t x = 0; x < tw; ++x) {
      // Nearest-neighbor resampling: map target coordinate to source coordinate
      // Truncates to nearest integer (no interpolation)
      auto src_x = static_cast<std::size_t>(static_cast<float>(x) * scale_x);
      auto src_y = static_cast<std::size_t>(static_cast<float>(y) * scale_y);

      // Calculate linear index in source bitmap (row-major layout)
      std::size_t idx = (src_y * w) + src_x;

      if (idx < data.size()) {
        // Simple binary threshold: 0=Black, non-zero=White
        // TODO: Support grayscale/color bitmaps via ColorManager
        Color color = (data[idx] == 0) ? Color::Black : Color::White;
        fb.set_pixel(pos.x + x, pos.y + y, color, orientation);
      }
    }
  }
}

} // namespace epaper
