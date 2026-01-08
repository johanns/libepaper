#pragma once

#include "epaper/display.hpp"
#include "epaper/font.hpp"
#include "epaper/geometry.hpp"
#include <cstdint>
#include <span>
#include <string>
#include <string_view>

namespace epaper {

/**
 * @brief Command for drawing a line between two points.
 *
 * Encapsulates all parameters needed to draw a line.
 * Constructed via LineBuilder.
 */
struct LineCommand {
  Point from;      ///< Starting point
  Point to;        ///< Ending point
  Color color;     ///< Line color
  DotPixel width;  ///< Line width
  LineStyle style; ///< Line style (solid/dotted)

  /**
   * @brief Construct a line command with specified parameters.
   *
   * @param from_pt Starting point
   * @param to_pt Ending point
   * @param c Line color (default: Black)
   * @param w Line width (default: 1x1)
   * @param s Line style (default: Solid)
   */
  constexpr LineCommand(Point from_pt, Point to_pt, Color c = Color::Black, DotPixel w = DotPixel::Pixel1x1,
                        LineStyle s = LineStyle::Solid) noexcept
      : from(from_pt), to(to_pt), color(c), width(w), style(s) {}
};

/**
 * @brief Command for drawing a rectangle.
 *
 * Encapsulates all parameters needed to draw a rectangle.
 * Constructed via RectangleBuilder.
 */
struct RectangleCommand {
  Point top_left;        ///< Top-left corner
  Point bottom_right;    ///< Bottom-right corner
  Color color;           ///< Rectangle color
  DotPixel border_width; ///< Border width
  DrawFill fill;         ///< Fill mode (empty/full)

  /**
   * @brief Construct a rectangle command with specified parameters.
   *
   * @param tl Top-left corner
   * @param br Bottom-right corner
   * @param c Rectangle color (default: Black)
   * @param w Border width (default: 1x1)
   * @param f Fill mode (default: Empty)
   */
  constexpr RectangleCommand(Point tl, Point br, Color c = Color::Black, DotPixel w = DotPixel::Pixel1x1,
                             DrawFill f = DrawFill::Empty) noexcept
      : top_left(tl), bottom_right(br), color(c), border_width(w), fill(f) {}
};

/**
 * @brief Command for drawing a circle.
 *
 * Encapsulates all parameters needed to draw a circle.
 * Constructed via CircleBuilder.
 */
struct CircleCommand {
  Point center;          ///< Center point
  std::size_t radius;    ///< Circle radius in pixels
  Color color;           ///< Circle color
  DotPixel border_width; ///< Border width
  DrawFill fill;         ///< Fill mode (empty/full)

  /**
   * @brief Construct a circle command with specified parameters.
   *
   * @param ctr Center point
   * @param r Circle radius
   * @param c Circle color (default: Black)
   * @param w Border width (default: 1x1)
   * @param f Fill mode (default: Empty)
   */
  constexpr CircleCommand(Point ctr, std::size_t r, Color c = Color::Black, DotPixel w = DotPixel::Pixel1x1,
                          DrawFill f = DrawFill::Empty) noexcept
      : center(ctr), radius(r), color(c), border_width(w), fill(f) {}
};

/**
 * @brief Command for drawing a point.
 *
 * Encapsulates all parameters needed to draw a point.
 * Constructed via PointBuilder.
 */
struct PointCommand {
  Point position;      ///< Point position
  Color color;         ///< Point color
  DotPixel pixel_size; ///< Point size

  /**
   * @brief Construct a point command with specified parameters.
   *
   * @param pos Point position
   * @param c Point color (default: Black)
   * @param s Point size (default: 1x1)
   */
  constexpr PointCommand(Point pos, Color c = Color::Black, DotPixel s = DotPixel::Pixel1x1) noexcept
      : position(pos), color(c), pixel_size(s) {}
};

/**
 * @brief Content type for text rendering.
 */
enum class TextContent {
  String, ///< String content
  Number, ///< Integer number
  Decimal ///< Decimal number
};

/**
 * @brief Command for drawing text.
 *
 * Encapsulates all parameters needed to draw text (string, number, or decimal).
 * Constructed via TextBuilder.
 */
struct TextCommand {
  Point position;              ///< Text position
  std::string text;            ///< Text content (or converted number)
  const Font *font;            ///< Font to use
  Color foreground;            ///< Foreground (text) color
  Color background;            ///< Background color
  TextContent content_type;    ///< Content type for internal use
  std::int32_t number;         ///< Number value (for Number type)
  double decimal;              ///< Decimal value (for Decimal type)
  std::uint8_t decimal_places; ///< Decimal places (for Decimal type)

  /**
   * @brief Construct a text command with string content.
   *
   * @param pos Text position
   * @param txt Text content
   * @param f Font to use
   * @param fg Foreground color (default: Black)
   * @param bg Background color (default: White)
   */
  TextCommand(Point pos, std::string_view txt, const Font *f, Color fg = Color::Black, Color bg = Color::White) noexcept
      : position(pos), text(txt), font(f), foreground(fg), background(bg), content_type(TextContent::String), number(0),
        decimal(0.0), decimal_places(0) {}

  /**
   * @brief Construct a text command with number content.
   *
   * @param pos Text position
   * @param num Number value
   * @param f Font to use
   * @param fg Foreground color (default: Black)
   * @param bg Background color (default: White)
   */
  TextCommand(Point pos, std::int32_t num, const Font *f, Color fg = Color::Black, Color bg = Color::White) noexcept
      : position(pos), text(), font(f), foreground(fg), background(bg), content_type(TextContent::Number), number(num),
        decimal(0.0), decimal_places(0) {}

  /**
   * @brief Construct a text command with decimal content.
   *
   * @param pos Text position
   * @param dec Decimal value
   * @param places Decimal places
   * @param f Font to use
   * @param fg Foreground color (default: Black)
   * @param bg Background color (default: White)
   */
  TextCommand(Point pos, double dec, std::uint8_t places, const Font *f, Color fg = Color::Black,
              Color bg = Color::White) noexcept
      : position(pos), text(), font(f), foreground(fg), background(bg), content_type(TextContent::Decimal), number(0),
        decimal(dec), decimal_places(places) {}
};

} // namespace epaper
