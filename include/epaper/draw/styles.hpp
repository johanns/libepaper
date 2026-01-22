#pragma once

/**
 * @file styles.hpp
 * @brief Reusable style specifications for drawing commands.
 *
 * Provides style bundle structs that encapsulate related visual properties.
 * Styles can be defined once and applied to multiple commands via `with_style()`
 * for consistent visual appearance.
 *
 * **Design Pattern:**
 * ```
 * Style Bundle Pattern
 * ├─ Encapsulation: Group related visual properties (color, width, fill)
 * ├─ Reusability: Define once, apply to many commands
 * ├─ Consistency: Ensure related elements share visual style
 * └─ Maintainability: Change style in one place, affects all uses
 * ```
 *
 * **Benefits:**
 * - **DRY Principle**: Don't repeat color/width/fill across similar commands
 * - **Theming**: Define visual themes (e.g., "accent", "warning", "disabled")
 * - **Readability**: Named styles communicate intent (e.g., `thick_border` vs `DotPixel::Pixel3x3`)
 * - **Compile-Time**: All constexpr - zero runtime overhead
 *
 * **Usage Patterns:**
 * ```cpp
 * // Define reusable styles
 * constexpr LineStyleSpec grid_lines{Color::Gray1, DotPixel::Pixel1x1, LineStyle::Dotted};
 * constexpr ShapeStyleSpec highlight{Color::Red, DotPixel::Pixel2x2, DrawFill::Empty};
 * constexpr TextStyleSpec header{&font20, Color::Black, Color::White};
 *
 * // Apply to multiple commands
 * display.draw(display.line().from({0,0}).to({100,0}).with_style(grid_lines).build());
 * display.draw(display.line().from({0,10}).to({100,10}).with_style(grid_lines).build());
 * display.draw(display.rectangle().at({10,10}).size({80,60}).with_style(highlight).build());
 * display.draw(display.text("Title").at({10,10}).with_style(header).build());
 * ```
 *
 * **Style Composition:**
 * - Styles can be stored in config files/structs and loaded at runtime
 * - Individual properties can be overridden after `with_style()`:
 *   ```cpp
 *   display.line().from(a).to(b).with_style(grid_lines).color(Color::Red).build();
 *   //                                                   ^^^^^^^^^^^^^^^^^^^ override
 *   ```
 *
 * @see builders.hpp, LineBuilder::with_style(), RectangleBuilder::with_style()
 */

#include "epaper/core/types.hpp"
#include "epaper/graphics/font.hpp"

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
