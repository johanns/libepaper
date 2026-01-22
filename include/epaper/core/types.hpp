#pragma once

#include <cstdint>

namespace epaper {

/**
 * @brief Color representation for e-paper displays.
 *
 * Defines logical colors supported across different display modes.
 * Not all colors are available on all displays - availability depends on
 * the DisplayMode (see Display::available_colors()).
 *
 * @note Color values are not device-native encodings - they are converted
 *       to display-specific bit patterns by ColorManager.
 *
 * @example
 * ```cpp
 * // Basic color usage
 * display.set_pixel(10, 20, Color::Black);
 * display.clear(Color::White);
 *
 * // Check available colors for current mode
 * auto colors = display.available_colors();
 * if (std::find(colors.begin(), colors.end(), Color::Red) != colors.end()) {
 *   display.draw_line({0, 0}, {100, 100}, Color::Red);
 * }
 * ```
 *
 * @see ColorManager, DisplayMode, Display::available_colors()
 */
enum class Color : std::uint8_t {
  White = 0xFF,  ///< White (or lightest gray in grayscale modes)
  Black = 0x00,  ///< Black (or darkest gray in grayscale modes)
  Gray1 = 0x80,  ///< First gray level - lighter (Grayscale4 mode only)
  Gray2 = 0x40,  ///< Second gray level - darker (Grayscale4 mode only)
  Red = 0x01,    ///< Red (BWR and Spectra6 modes)
  Yellow = 0x02, ///< Yellow (BWY and Spectra6 modes)
  Blue = 0x03,   ///< Blue (Spectra6 mode only)
  Green = 0x04   ///< Green (Spectra6 mode only)
};

/**
 * @brief Display orientation modes.
 *
 * Controls coordinate system rotation for all drawing operations.
 * Physical pixels are transformed according to the orientation setting.
 *
 * Orientation affects Display::width() and Display::height() - portrait
 * modes return (physical_width, physical_height) while landscape modes
 * return (physical_height, physical_width).
 *
 * @example
 * ```cpp
 * // Create display in landscape mode
 * auto result = create_display<EPD27>(DisplayMode::BlackWhite, Orientation::Landscape90);
 * if (result) {
 *   auto display = std::move(*result);
 *   // width() and height() are swapped in landscape
 *   display.draw_text({0, 0}, "Rotated text", font, Color::Black, Color::White);
 * }
 * ```
 *
 * @see Display::orientation(), Display::width(), Display::height()
 */
enum class Orientation : std::uint8_t {
  Portrait0 = 0,   ///< Default portrait orientation (0° rotation)
  Landscape90 = 1, ///< 90° clockwise rotation (landscape mode)
  Portrait180 = 2, ///< 180° rotation (upside down portrait)
  Landscape270 = 3 ///< 270° clockwise / 90° counter-clockwise (landscape mode)
};

/**
 * @brief Drawing styles for points and lines - defines pen width.
 *
 * Specifies the pixel size for drawing operations. Each value represents
 * an NxN pixel block. Used by LineBuilder, PointBuilder, and shape drawing
 * commands to control stroke width.
 *
 * @note Larger pixel sizes create thicker lines and borders but may reduce
 *       fine detail. DotPixel::Pixel1x1 provides the finest resolution.
 *
 * @example
 * ```cpp
 * // Draw thin and thick lines
 * display.line({0, 0}, {100, 0}).width(DotPixel::Pixel1x1).color(Color::Black).draw();
 * display.line({0, 10}, {100, 10}).width(DotPixel::Pixel3x3).color(Color::Black).draw();
 * ```
 *
 * @see LineBuilder::width(), PointBuilder::width()
 */
enum class DotPixel : std::uint8_t {
  Pixel1x1 = 1, ///< Single pixel (finest resolution)
  Pixel2x2 = 2, ///< 2x2 pixel block
  Pixel3x3 = 3, ///< 3x3 pixel block
  Pixel4x4 = 4, ///< 4x4 pixel block
  Pixel5x5 = 5, ///< 5x5 pixel block
  Pixel6x6 = 6, ///< 6x6 pixel block
  Pixel7x7 = 7, ///< 7x7 pixel block
  Pixel8x8 = 8  ///< 8x8 pixel block (thickest)
};

/**
 * @brief Line drawing styles - defines line pattern.
 *
 * Specifies whether lines are drawn as continuous strokes or dotted patterns.
 * Applies to lines, rectangles, circles, and other shape outlines.
 *
 * @example
 * ```cpp
 * // Solid border rectangle
 * display.rectangle({10, 10}, {50, 50}).style(LineStyle::Solid).draw();
 *
 * // Dotted outline circle
 * display.circle({100, 100}, 30).style(LineStyle::Dotted).draw();
 * ```
 *
 * @see LineBuilder::style(), RectangleBuilder, CircleBuilder
 */
enum class LineStyle {
  Solid, ///< Continuous solid line
  Dotted ///< Dotted line pattern (alternating pixels)
};

/**
 * @brief Fill modes for shapes - controls interior rendering.
 *
 * Determines whether shapes (rectangles, circles) are drawn as outlines
 * or filled solid regions.
 *
 * @note DrawFill::Empty draws only the shape's border using the specified
 *       line style and width. DrawFill::Full fills the entire interior.
 *
 * @example
 * ```cpp
 * // Hollow rectangle
 * display.rectangle({10, 10}, {50, 50})
 *   .fill(DrawFill::Empty)
 *   .color(Color::Black)
 *   .draw();
 *
 * // Filled circle
 * display.circle({100, 100}, 30)
 *   .fill(DrawFill::Full)
 *   .color(Color::Red)
 *   .draw();
 * ```
 *
 * @see RectangleBuilder::fill(), CircleBuilder::fill()
 */
enum class DrawFill {
  Empty, ///< Draw outline only (border with no fill)
  Full   ///< Draw filled shape (solid interior)
};

} // namespace epaper
