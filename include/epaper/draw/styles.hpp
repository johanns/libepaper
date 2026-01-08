#pragma once

#include "epaper/display.hpp"
#include "epaper/font.hpp"

namespace epaper {

/**
 * @brief Reusable style specification for line drawing.
 *
 * Encapsulates color, width, and style for drawing lines.
 * Can be applied to LineBuilder via `with_style()`.
 *
 * @example
 * @code{.cpp}
 * LineStyleSpec thick_black{Color::Black, DotPixel::Pixel3x3, LineStyle::Solid};
 * display.draw(display.line().from({10, 10}).to({100, 10}).with_style(thick_black).build());
 * @endcode
 */
struct LineStyleSpec {
  Color color;     ///< Line color
  DotPixel width;  ///< Line width
  LineStyle style; ///< Line style (solid/dotted)

  /**
   * @brief Construct a line style with specified parameters.
   *
   * @param c Line color
   * @param w Line width (default: 1x1)
   * @param s Line style (default: Solid)
   */
  constexpr LineStyleSpec(Color c, DotPixel w = DotPixel::Pixel1x1, LineStyle s = LineStyle::Solid) noexcept
      : color(c), width(w), style(s) {}
};

/**
 * @brief Reusable style specification for shapes (rectangles, circles).
 *
 * Encapsulates color, border width, and fill mode for drawing shapes.
 * Can be applied to RectangleBuilder or CircleBuilder via `with_style()`.
 *
 * @example
 * @code{.cpp}
 * ShapeStyleSpec filled_black{Color::Black, DotPixel::Pixel1x1, DrawFill::Full};
 * display.draw(display.rectangle().top_left({10, 10}).bottom_right({50, 50}).with_style(filled_black).build());
 * @endcode
 */
struct ShapeStyleSpec {
  Color color;           ///< Shape color
  DotPixel border_width; ///< Border width
  DrawFill fill;         ///< Fill mode (empty/full)

  /**
   * @brief Construct a shape style with specified parameters.
   *
   * @param c Shape color
   * @param w Border width (default: 1x1)
   * @param f Fill mode (default: Empty)
   */
  constexpr ShapeStyleSpec(Color c, DotPixel w = DotPixel::Pixel1x1, DrawFill f = DrawFill::Empty) noexcept
      : color(c), border_width(w), fill(f) {}
};

/**
 * @brief Reusable style specification for text rendering.
 *
 * Encapsulates font, foreground color, and background color for drawing text.
 * Can be applied to TextBuilder via `with_style()`.
 *
 * @example
 * @code{.cpp}
 * TextStyleSpec header_style{&Font::font20(), Color::Black, Color::White};
 * display.draw(display.text("Header").at({10, 10}).with_style(header_style).build());
 * @endcode
 */
struct TextStyleSpec {
  const Font *font; ///< Font to use for rendering
  Color foreground; ///< Foreground (text) color
  Color background; ///< Background color

  /**
   * @brief Construct a text style with specified parameters.
   *
   * @param f Font to use
   * @param fg Foreground color
   * @param bg Background color
   */
  constexpr TextStyleSpec(const Font *f, Color fg, Color bg) noexcept : font(f), foreground(fg), background(bg) {}
};

} // namespace epaper
