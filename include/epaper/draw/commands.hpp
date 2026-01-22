#pragma once

/**
 * @file commands.hpp
 * @brief Immutable command structures for drawing operations.
 *
 * Defines value types representing drawing commands. Each command is a plain
 * struct (no methods beyond constructors) that encapsulates all parameters
 * needed to perform a drawing operation.
 *
 * **Design Pattern:**
 * ```
 * Command Pattern (Data-Oriented)
 * ├─ Immutable: All members const or effectively const after construction
 * ├─ Value Semantics: Cheap to copy, no ownership issues
 * ├─ Serializable: POD-like structs (could be written to binary format)
 * └─ Decoupled: Commands independent of execution (Display handles rendering)
 * ```
 *
 * **Rationale:**
 * - **Separation of Concerns**: Command creation (builders) vs execution (Display)
 * - **Testability**: Commands can be constructed and validated without hardware
 * - **Serialization**: Commands could be logged, queued, or replayed
 * - **Type Safety**: Each command type carries exactly the parameters it needs
 *
 * **Lifetime:**
 * - Commands typically stack-allocated and short-lived
 * - Builder creates command, immediately passed to Display::draw(), then destroyed
 * - For deferred rendering, commands can be stored in std::vector or queue
 *
 * **Thread Safety:**
 * - Commands are const after construction (inherently thread-safe to read)
 * - Font pointers must remain valid during command lifetime (borrowed, not owned)
 *
 * @example
 * ```cpp
 * // Direct construction (rare - prefer builders)
 * LineCommand cmd1{{10,10}, {100,100}, Color::Black, DotPixel::Pixel1x1, LineStyle::Solid};
 *
 * // Typical usage via builder
 * auto cmd2 = LineBuilder().from({10,10}).to({100,100}).build();
 * display.draw(cmd2);
 *
 * // Deferred rendering (command queue)
 * std::vector<LineCommand> lines;
 * lines.push_back(LineBuilder().from(a).to(b).build());
 * lines.push_back(LineBuilder().from(c).to(d).build());
 * for (const auto& cmd : lines) {
 *   display.draw(cmd);
 * }
 * ```
 *
 * @see builders.hpp, Display::draw(), std::variant for command polymorphism
 */

#include "epaper/core/geometry.hpp"
#include "epaper/core/types.hpp"
#include "epaper/graphics/font.hpp"
#include <cstdint>
#include <string>
#include <string_view>

namespace epaper {

/**
 * @brief Command for drawing a line between two points.
 *
 * Encapsulates all parameters needed to draw a line using Bresenham's algorithm.
 * Constructed via LineBuilder for type-safe, expressive creation.
 *
 * **Member Semantics:**
 * - `from`, `to`: Line endpoints in logical coordinate space (pre-orientation)
 * - `color`: Pixel color (mode-dependent rendering via ColorManager)
 * - `width`: Line thickness (DotPixel::Pixel1x1 = 1px, Pixel2x2 = 2px, etc.)
 * - `style`: Solid (all pixels) or Dotted (alternating pixels)
 *
 * **Implementation Notes:**
 * - Width > 1px implemented by drawing parallel offset lines
 * - Dotted style: draws pixel at even step indices only (step % 2 == 0)
 * - Coordinates validated/clipped during rendering, not at construction
 *
 * @see LineBuilder, Graphics::draw_line()
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
 * Encapsulates all parameters needed to draw a rectangle (axis-aligned).
 * Constructed via RectangleBuilder.
 *
 * **Member Semantics:**
 * - `top_left`: Top-left corner coordinate
 * - `bottom_right`: Bottom-right corner coordinate
 * - `color`: Rectangle color (border and fill use same color)
 * - `border_width`: Thickness of rectangle outline
 * - `fill`: Empty (outline only) or Full (solid fill)
 *
 * **Coordinate Constraints:**
 * - Assumes `bottom_right.x >= top_left.x` and `bottom_right.y >= top_left.y`
 * - Inverted coordinates (e.g., top_left.x > bottom_right.x) produce undefined results
 * - Zero-area rectangles (top_left == bottom_right) render as single point
 *
 * **Fill Behavior:**
 * - DrawFill::Empty: 4 border lines only (interior transparent)
 * - DrawFill::Full: Border + interior filled with horizontal scan lines
 *
 * @see RectangleBuilder, Graphics::draw_rectangle()
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
 * Encapsulates all parameters needed to draw a circle using midpoint algorithm.
 * Constructed via CircleBuilder.
 *
 * **Member Semantics:**
 * - `center`: Circle center point in logical coordinates
 * - `radius`: Circle radius in pixels (always positive)
 * - `color`: Circle color (border and fill use same color)
 * - `border_width`: Thickness of circle outline (currently unimplemented - always 1px)
 * - `fill`: Empty (outline only) or Full (solid fill)
 *
 * **Algorithm:**
 * - Midpoint circle algorithm (Bresenham's circle) - integer-only, no FPU
 * - Draws one octant, mirrors to remaining 7 using symmetry
 * - Fill implemented as horizontal scan lines between symmetric points
 *
 * **Coordinate Clipping:**
 * - No bounds checking at construction - circles may extend beyond display edges
 * - Framebuffer set_pixel() performs clipping during rendering
 *
 * @see CircleBuilder, Graphics::draw_circle()
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
 * @brief Command for drawing a point (pixel or multi-pixel dot).
 *
 * Encapsulates all parameters needed to draw a point.
 * Constructed via PointBuilder.
 *
 * **Member Semantics:**
 * - `position`: Point location in logical coordinates
 * - `color`: Pixel color
 * - `pixel_size`: Size of the "dot" (Pixel1x1 to Pixel8x8)
 *
 * **Pixel Size Rendering:**
 * - Pixel1x1: Single pixel at `position`
 * - Pixel2x2: 2x2 square centered at `position` (4 pixels total)
 * - Pixel3x3: 3x3 square centered at `position` (9 pixels total)
 * - ... and so on up to Pixel8x8
 *
 * **Centering Behavior:**
 * - For even sizes (2x2, 4x4, etc.), center is top-left of 2x2 center quad
 * - For odd sizes (1x1, 3x3, etc.), center is exact middle pixel
 *
 * @see PointBuilder, DotPixel
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
 *
 * Discriminant for TextCommand's internal union-like behavior.
 * Determines which fields are active and how `text` is populated.
 */
enum class TextContent {
  String, ///< Direct string content (text field used as-is)
  Number, ///< Integer number (number field converted to text via sprintf)
  Decimal ///< Decimal number (decimal field formatted with decimal_places precision)
};

/**
 * @brief Command for drawing text.
 *
 * Encapsulates all parameters needed to draw text (string, number, or decimal).
 * Constructed via TextBuilder.
 *
 * **Member Semantics:**
 * - `position`: Text baseline position (top-left of first character cell)
 * - `text`: String content or converted number/decimal (read-only after construction)
 * - `font`: Pointer to loaded font (borrowed - must outlive command)
 * - `foreground`: Text ink color (1 bits in font bitmap)
 * - `background`: Paper color (0 bits in font bitmap)
 * - `content_type`: Internal discriminant for TextBuilder (user ignores this)
 * - `number`, `decimal`, `decimal_places`: Internal storage for numeric types
 *
 * **Font Requirements:**
 * - Font must be loaded via Font::load() before command creation
 * - Font determines character dimensions (fixed-width bitmap)
 * - Missing characters render as blank space (no error)
 *
 * **Text Rendering:**
 * - Characters rendered left-to-right, no line wrapping
 * - Cursor advances by font.metrics().width for each character
 * - Text may extend beyond display edges (clipped at framebuffer level)
 *
 * **Numeric Formatting:**
 * - Number: sprintf("%d", number) → "-123", "0", "999"
 * - Decimal: sprintf("%.*f", decimal_places, decimal) → "3.14", "2.718"
 * - Formatted string stored in `text` field during TextBuilder::build()
 *
 * @warning Font pointer must remain valid for command lifetime. Do not pass
 *          temporary Font objects (use Font::load() and store result).
 *
 * @see TextBuilder, Font, Graphics::draw_text()
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
      : position(pos), font(f), foreground(fg), background(bg), content_type(TextContent::Number), number(num),
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
      : position(pos), font(f), foreground(fg), background(bg), content_type(TextContent::Decimal), number(0),
        decimal(dec), decimal_places(places) {}
};

} // namespace epaper
