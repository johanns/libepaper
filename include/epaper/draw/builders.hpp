#pragma once

/**
 * @file builders.hpp
 * @brief Fluent builder pattern for constructing drawing commands.
 *
 * Provides type-safe, expressive builders for all drawing command types.
 * Each builder follows the fluent interface pattern (method chaining) for
 * readable, self-documenting code.
 *
 * **Design Pattern:**
 * ```
 * Builder Pattern (Fluent Interface)
 * ├─ Default Construction: Provides sensible defaults
 * ├─ Method Chaining: Each setter returns *this for chaining
 * ├─ Type Safety: All parameters statically typed (no stringly-typed APIs)
 * └─ Finalization: build() produces immutable command struct
 * ```
 *
 * **Benefits:**
 * - **Readability**: `line().from({0,0}).to({100,100}).color(Black)` reads like English
 * - **Discoverability**: IDE autocomplete guides users through available options
 * - **Flexibility**: Optional parameters with sensible defaults
 * - **Immutability**: Built commands are const, preventing accidental modification
 * - **Style Reuse**: `with_style()` applies predefined style bundles
 *
 * **Usage Patterns:**
 * ```cpp
 * // Direct builder usage
 * auto line_cmd = LineBuilder()
 *   .from({10, 10})
 *   .to({100, 100})
 *   .color(Color::Black)
 *   .build();
 * display.draw(line_cmd);
 *
 * // Using Display::line() factory (preferred)
 * display.draw(
 *   display.line()
 *     .from({10, 10})
 *     .to({100, 100})
 *     .color(Color::Black)
 *     .build()
 * );
 *
 * // Style reuse for consistent appearance
 * LineStyleSpec thick_black{Color::Black, DotPixel::Pixel3x3};
 * display.draw(display.line().from(a).to(b).with_style(thick_black).build());
 * display.draw(display.line().from(c).to(d).with_style(thick_black).build());
 * ```
 *
 * **Implementation Details:**
 * - All builders are value types (cheap to copy, no heap allocation)
 * - Private members hold intermediate state
 * - `build()` marked `[[nodiscard]]` to prevent unused command objects
 * - Setters are `noexcept` (no allocation, no exceptions)
 *
 * @see commands.hpp, styles.hpp, Display::line(), Display::rectangle()
 */

#include "epaper/core/geometry.hpp"
#include "epaper/draw/commands.hpp"
#include "epaper/draw/styles.hpp"
#include "epaper/graphics/font.hpp"
#include <cstdint>
#include <string>
#include <string_view>

namespace epaper {

/**
 * @brief Fluent builder for constructing LineCommand.
 *
 * Provides a fluent interface for specifying line drawing parameters.
 * Call `build()` to produce a LineCommand that can be passed to `Display::draw()`.
 *
 * **Method Overloads:**
 * - Coordinate setters accept both Point and (x, y) pairs for flexibility
 * - All parameters optional with sensible defaults (Black, 1x1 width, Solid style)
 *
 * **Defaults:**
 * - from: (0, 0)
 * - to: (0, 0)
 * - color: Black
 * - width: Pixel1x1
 * - style: Solid
 *
 * @example
 * ```cpp
 * // Minimal usage (uses defaults)
 * auto cmd1 = LineBuilder().from({10, 10}).to({100, 100}).build();
 *
 * // Full specification
 * auto cmd2 = LineBuilder()
 *     .from({10, 10})
 *     .to({100, 100})
 *     .color(Color::Black)
 *     .width(DotPixel::Pixel2x2)
 *     .style(LineStyle::Dotted)
 *     .build();
 * display.draw(cmd2);
 *
 * // Using coordinate pairs instead of Point
 * auto cmd3 = LineBuilder().from(0, 0).to(100, 100).build();
 *
 * // Applying reusable style
 * LineStyleSpec dashed_red{Color::Red, DotPixel::Pixel1x1, LineStyle::Dotted};
 * auto cmd4 = LineBuilder().from(a).to(b).with_style(dashed_red).build();
 * ```
 *
 * @see LineCommand, LineStyleSpec, Display::draw()
 */
class LineBuilder {
public:
  /**
   * @brief Default constructor creates a builder with default values.
   */
  LineBuilder() noexcept = default;

  /**
   * @brief Set the starting point of the line.
   *
   * @param pt Starting point
   * @return Reference to this builder for chaining
   */
  auto from(Point pt) noexcept -> LineBuilder & {
    from_ = pt;
    return *this;
  }

  /**
   * @brief Set the starting point of the line using coordinates.
   *
   * @param x Starting X coordinate
   * @param y Starting Y coordinate
   * @return Reference to this builder for chaining
   */
  auto from(std::size_t x, std::size_t y) noexcept -> LineBuilder & {
    from_ = Point{x, y};
    return *this;
  }

  /**
   * @brief Set the ending point of the line.
   *
   * @param pt Ending point
   * @return Reference to this builder for chaining
   */
  auto to(Point pt) noexcept -> LineBuilder & {
    to_ = pt;
    return *this;
  }

  /**
   * @brief Set the ending point of the line using coordinates.
   *
   * @param x Ending X coordinate
   * @param y Ending Y coordinate
   * @return Reference to this builder for chaining
   */
  auto to(std::size_t x, std::size_t y) noexcept -> LineBuilder & {
    to_ = Point{x, y};
    return *this;
  }

  /**
   * @brief Set the line color.
   *
   * @param c Line color
   * @return Reference to this builder for chaining
   */
  auto color(Color c) noexcept -> LineBuilder & {
    color_ = c;
    return *this;
  }

  /**
   * @brief Set the line width.
   *
   * @param w Line width
   * @return Reference to this builder for chaining
   */
  auto width(DotPixel w) noexcept -> LineBuilder & {
    width_ = w;
    return *this;
  }

  /**
   * @brief Set the line style.
   *
   * @param s Line style (solid/dotted)
   * @return Reference to this builder for chaining
   */
  auto style(LineStyle s) noexcept -> LineBuilder & {
    style_ = s;
    return *this;
  }

  /**
   * @brief Apply a reusable line style.
   *
   * @param line_style Style specification to apply
   * @return Reference to this builder for chaining
   */
  auto with_style(const LineStyleSpec &line_style) noexcept -> LineBuilder & {
    color_ = line_style.color;
    width_ = line_style.width;
    style_ = line_style.style;
    return *this;
  }

  /**
   * @brief Build the final LineCommand.
   *
   * @return LineCommand ready to be passed to Display::draw()
   */
  [[nodiscard]] auto build() const noexcept -> LineCommand { return LineCommand{from_, to_, color_, width_, style_}; }

private:
  Point from_{0, 0};
  Point to_{0, 0};
  Color color_ = Color::Black;
  DotPixel width_ = DotPixel::Pixel1x1;
  LineStyle style_ = LineStyle::Solid;
};

/**
 * @brief Fluent builder for constructing RectangleCommand.
 *
 * Provides a fluent interface for specifying rectangle drawing parameters.
 * Supports both corner-based and position+size-based specification.
 *
 * **Specification Modes:**
 * 1. **Corner-based**: `top_left()` + `bottom_right()` - direct corner coordinates
 * 2. **Position+Size**: `at()` + `size()` - position plus dimensions
 *
 * **Interaction Note:**
 * - `size()` calculates `bottom_right` from current `top_left`, so order matters:
 *   - Correct: `.at({10,10}).size({80,60})` → bottom_right = (90, 70)
 *   - Incorrect: `.size({80,60}).at({10,10})` → bottom_right = (80, 60) [wrong]
 *
 * **Defaults:**
 * - top_left: (0, 0)
 * - bottom_right: (0, 0)
 * - color: Black
 * - border_width: Pixel1x1
 * - fill: Empty
 *
 * @example
 * ```cpp
 * // Using corners
 * auto cmd1 = RectangleBuilder()
 *     .top_left({10, 10})
 *     .bottom_right({100, 50})
 *     .color(Color::Black)
 *     .fill(DrawFill::Empty)
 *     .build();
 *
 * // Using position + size (more intuitive for UI layouts)
 * auto cmd2 = RectangleBuilder()
 *     .at({10, 10})        // Top-left position
 *     .size({90, 40})      // Width x Height
 *     .color(Color::Black)
 *     .fill(DrawFill::Full)
 *     .build();
 *
 * // Filled rectangle with thick border
 * auto cmd3 = RectangleBuilder()
 *     .at({10, 10})
 *     .size(100, 60)
 *     .color(Color::Red)
 *     .border_width(DotPixel::Pixel3x3)
 *     .fill(DrawFill::Full)
 *     .build();
 *
 * // Applying reusable style
 * ShapeStyleSpec filled_black{Color::Black, DotPixel::Pixel1x1, DrawFill::Full};
 * auto cmd4 = RectangleBuilder().at({10,10}).size({80,60}).with_style(filled_black).build();
 * ```
 *
 * @see RectangleCommand, ShapeStyleSpec, Display::draw()
 */
class RectangleBuilder {
public:
  /**
   * @brief Default constructor creates a builder with default values.
   */
  RectangleBuilder() noexcept = default;

  /**
   * @brief Set the top-left corner of the rectangle.
   *
   * @param pt Top-left corner
   * @return Reference to this builder for chaining
   */
  auto top_left(Point pt) noexcept -> RectangleBuilder & {
    top_left_ = pt;
    return *this;
  }

  /**
   * @brief Set the top-left corner using coordinates.
   *
   * @param x Top-left X coordinate
   * @param y Top-left Y coordinate
   * @return Reference to this builder for chainingsrc include/epaper
   */
  auto top_left(std::size_t x, std::size_t y) noexcept -> RectangleBuilder & {
    top_left_ = Point{x, y};
    return *this;
  }

  /**
   * @brief Set the bottom-right corner of the rectangle.
   *
   * @param pt Bottom-right corner
   * @return Reference to this builder for chaining
   */
  auto bottom_right(Point pt) noexcept -> RectangleBuilder & {
    bottom_right_ = pt;
    return *this;
  }

  /**
   * @brief Set the bottom-right corner using coordinates.
   *
   * @param x Bottom-right X coordinate
   * @param y Bottom-right Y coordinate
   * @return Reference to this builder for chaining
   */
  auto bottom_right(std::size_t x, std::size_t y) noexcept -> RectangleBuilder & {
    bottom_right_ = Point{x, y};
    return *this;
  }

  /**
   * @brief Set the position (top-left corner) of the rectangle.
   *
   * @param pt Position point
   * @return Reference to this builder for chaining
   */
  auto at(Point pt) noexcept -> RectangleBuilder & {
    top_left_ = pt;
    return *this;
  }

  /**
   * @brief Set the position using coordinates.
   *
   * @param x X coordinate
   * @param y Y coordinate
   * @return Reference to this builder for chaining
   */
  auto at(std::size_t x, std::size_t y) noexcept -> RectangleBuilder & {
    top_left_ = Point{x, y};
    return *this;
  }

  /**
   * @brief Set the size of the rectangle.
   *
   * This method calculates the bottom-right corner based on the current
   * top-left position and the specified size.
   *
   * @param sz Size of the rectangle
   * @return Reference to this builder for chaining
   */
  auto size(Size sz) noexcept -> RectangleBuilder & {
    bottom_right_ = Point{top_left_.x + sz.width, top_left_.y + sz.height};
    return *this;
  }

  /**
   * @brief Set the size using width and height.
   *
   * @param w Width
   * @param h Height
   * @return Reference to this builder for chaining
   */
  auto size(std::size_t w, std::size_t h) noexcept -> RectangleBuilder & {
    bottom_right_ = Point{top_left_.x + w, top_left_.y + h};
    return *this;
  }

  /**
   * @brief Set the rectangle color.
   *
   * @param c Rectangle color
   * @return Reference to this builder for chaining
   */
  auto color(Color c) noexcept -> RectangleBuilder & {
    color_ = c;
    return *this;
  }

  /**
   * @brief Set the border width.
   *
   * @param w Border width
   * @return Reference to this builder for chaining
   */
  auto border_width(DotPixel w) noexcept -> RectangleBuilder & {
    border_width_ = w;
    return *this;
  }

  /**
   * @brief Set the fill mode.
   *
   * @param f Fill mode (Empty or Full)
   * @return Reference to this builder for chaining
   */
  auto fill(DrawFill f) noexcept -> RectangleBuilder & {
    fill_ = f;
    return *this;
  }

  /**
   * @brief Apply a reusable shape style.
   *
   * @param shape_style Style specification to apply
   * @return Reference to this builder for chaining
   */
  auto with_style(const ShapeStyleSpec &shape_style) noexcept -> RectangleBuilder & {
    color_ = shape_style.color;
    border_width_ = shape_style.border_width;
    fill_ = shape_style.fill;
    return *this;
  }

  /**
   * @brief Build the final RectangleCommand.
   *
   * @return RectangleCommand ready to be passed to Display::draw()
   */
  [[nodiscard]] auto build() const noexcept -> RectangleCommand {
    return RectangleCommand{top_left_, bottom_right_, color_, border_width_, fill_};
  }

private:
  Point top_left_{0, 0};
  Point bottom_right_{0, 0};
  Color color_ = Color::Black;
  DotPixel border_width_ = DotPixel::Pixel1x1;
  DrawFill fill_ = DrawFill::Empty;
};

/**
 * @brief Fluent builder for constructing CircleCommand.
 *
 * Provides a fluent interface for specifying circle drawing parameters.
 *
 * **Defaults:**
 * - center: (0, 0)
 * - radius: 0
 * - color: Black
 * - border_width: Pixel1x1
 * - fill: Empty
 *
 * **Coordinate System:**
 * - Center coordinates are in logical pixel space (before orientation transform)
 * - Radius is in pixels (always positive)
 * - Circle may be clipped at display edges (no bounds checking in builder)
 *
 * @example
 * ```cpp
 * // Basic empty circle
 * auto cmd1 = CircleBuilder()
 *     .center({50, 50})
 *     .radius(25)
 *     .color(Color::Black)
 *     .build();
 *
 * // Filled circle (solid dot)
 * auto cmd2 = CircleBuilder()
 *     .center({100, 100})
 *     .radius(10)
 *     .color(Color::Red)
 *     .fill(DrawFill::Full)
 *     .build();
 * display.draw(cmd2);
 *
 * // Using coordinate pairs
 * auto cmd3 = CircleBuilder().center(75, 75).radius(30).build();
 *
 * // Applying reusable style
 * ShapeStyleSpec filled_red{Color::Red, DotPixel::Pixel1x1, DrawFill::Full};
 * auto cmd4 = CircleBuilder().center({50,50}).radius(20).with_style(filled_red).build();
 * ```
 *
 * @see CircleCommand, ShapeStyleSpec, Display::draw()
 */
class CircleBuilder {
public:
  /**
   * @brief Default constructor creates a builder with default values.
   */
  CircleBuilder() noexcept = default;

  /**
   * @brief Set the center point of the circle.
   *
   * @param pt Center point
   * @return Reference to this builder for chaining
   */
  auto center(Point pt) noexcept -> CircleBuilder & {
    center_ = pt;
    return *this;
  }

  /**
   * @brief Set the center point using coordinates.
   *
   * @param x Center X coordinate
   * @param y Center Y coordinate
   * @return Reference to this builder for chaining
   */
  auto center(std::size_t x, std::size_t y) noexcept -> CircleBuilder & {
    center_ = Point{x, y};
    return *this;
  }

  /**
   * @brief Set the circle radius.
   *
   * @param r Radius in pixels
   * @return Reference to this builder for chaining
   */
  auto radius(std::size_t r) noexcept -> CircleBuilder & {
    radius_ = r;
    return *this;
  }

  /**
   * @brief Set the circle color.
   *
   * @param c Circle color
   * @return Reference to this builder for chaining
   */
  auto color(Color c) noexcept -> CircleBuilder & {
    color_ = c;
    return *this;
  }

  /**
   * @brief Set the border width.
   *
   * @param w Border width
   * @return Reference to this builder for chaining
   */
  auto border_width(DotPixel w) noexcept -> CircleBuilder & {
    border_width_ = w;
    return *this;
  }

  /**
   * @brief Set the fill mode.
   *
   * @param f Fill mode (Empty or Full)
   * @return Reference to this builder for chaining
   */
  auto fill(DrawFill f) noexcept -> CircleBuilder & {
    fill_ = f;
    return *this;
  }

  /**
   * @brief Apply a reusable shape style.
   *
   * @param shape_style Style specification to apply
   * @return Reference to this builder for chaining
   */
  auto with_style(const ShapeStyleSpec &shape_style) noexcept -> CircleBuilder & {
    color_ = shape_style.color;
    border_width_ = shape_style.border_width;
    fill_ = shape_style.fill;
    return *this;
  }

  /**
   * @brief Build the final CircleCommand.
   *
   * @return CircleCommand ready to be passed to Display::draw()
   */
  [[nodiscard]] auto build() const noexcept -> CircleCommand {
    return CircleCommand{center_, radius_, color_, border_width_, fill_};
  }

private:
  Point center_{0, 0};
  std::size_t radius_ = 0;
  Color color_ = Color::Black;
  DotPixel border_width_ = DotPixel::Pixel1x1;
  DrawFill fill_ = DrawFill::Empty;
};

/**
 * @brief Fluent builder for constructing PointCommand.
 *
 * Provides a fluent interface for specifying point (pixel) drawing parameters.
 *
 * **Use Cases:**
 * - Plotting individual pixels (e.g., scatter plots, data points)
 * - Drawing custom shapes pixel-by-pixel
 * - Creating larger "dots" using DotPixel size (e.g., Pixel3x3 for markers)
 *
 * **DotPixel Sizes:**
 * - Pixel1x1: Single pixel
 * - Pixel2x2: 2x2 square (4 pixels total)
 * - Pixel3x3: 3x3 square (9 pixels total)
 * - ... up to Pixel8x8: 8x8 square (64 pixels total)
 *
 * **Defaults:**
 * - position: (0, 0)
 * - color: Black
 * - pixel_size: Pixel1x1
 *
 * @example
 * ```cpp
 * // Single pixel
 * auto cmd1 = PointBuilder().at({10, 10}).color(Color::Black).build();
 * display.draw(cmd1);
 *
 * // Large marker (3x3)
 * auto cmd2 = PointBuilder()
 *     .at({50, 50})
 *     .color(Color::Red)
 *     .size(DotPixel::Pixel3x3)
 *     .build();
 *
 * // Using coordinate pairs
 * auto cmd3 = PointBuilder().at(100, 100).color(Color::Black).build();
 *
 * // Plot multiple data points
 * std::vector<Point> data_points = {{10,20}, {15,25}, {20,18}};
 * for (const auto& pt : data_points) {
 *   display.draw(PointBuilder().at(pt).color(Color::Black).build());
 * }
 * ```
 *
 * @see PointCommand, DotPixel, Display::draw()
 */
class PointBuilder {
public:
  /**
   * @brief Default constructor creates a builder with default values.
   */
  PointBuilder() noexcept = default;

  /**
   * @brief Set the position of the point.
   *
   * @param pt Point position
   * @return Reference to this builder for chaining
   */
  auto at(Point pt) noexcept -> PointBuilder & {
    position_ = pt;
    return *this;
  }

  /**
   * @brief Set the position using coordinates.
   *
   * @param x X coordinate
   * @param y Y coordinate
   * @return Reference to this builder for chaining
   */
  auto at(std::size_t x, std::size_t y) noexcept -> PointBuilder & {
    position_ = Point{x, y};
    return *this;
  }

  /**
   * @brief Set the point color.
   *
   * @param c Point color
   * @return Reference to this builder for chaining
   */
  auto color(Color c) noexcept -> PointBuilder & {
    color_ = c;
    return *this;
  }

  /**
   * @brief Set the point size.
   *
   * @param s Point size
   * @return Reference to this builder for chaining
   */
  auto size(DotPixel s) noexcept -> PointBuilder & {
    pixel_size_ = s;
    return *this;
  }

  /**
   * @brief Build the final PointCommand.
   *
   * @return PointCommand ready to be passed to Display::draw()
   */
  [[nodiscard]] auto build() const noexcept -> PointCommand { return PointCommand{position_, color_, pixel_size_}; }

private:
  Point position_{0, 0};
  Color color_ = Color::Black;
  DotPixel pixel_size_ = DotPixel::Pixel1x1;
};

/**
 * @brief Fluent builder for constructing TextCommand.
 *
 * Provides a fluent interface for specifying text rendering parameters.
 * Supports strings, integers, and decimal numbers with automatic formatting.
 *
 * **Content Types:**
 * - **String**: Direct text via constructor or `text()` method
 * - **Number**: Integer (int32_t) via `number()` - auto-converted to string
 * - **Decimal**: Floating-point (double) via `decimal(value, places)` - formatted with fixed precision
 *
 * **Automatic Formatting:**
 * - Numbers: sprintf("%d") → "-42", "1234"
 * - Decimals: sprintf("%.*f", places) → "3.14", "2.718"
 *
 * **Font Requirements:**
 * - Font must be loaded before rendering (see Font::load())
 * - Font determines character dimensions (fixed-width bitmap fonts)
 * - Null font pointer → undefined behavior (no safety check in builder)
 *
 * **Defaults:**
 * - position: (0, 0)
 * - text: "" (empty)
 * - font: nullptr (MUST be set before build())
 * - foreground: Black
 * - background: White
 *
 * @example
 * ```cpp
 * // String text
 * auto font = Font::load("font.bdf").value();
 * auto cmd1 = TextBuilder("Hello")
 *     .at({10, 20})
 *     .font(&font)
 *     .foreground(Color::Black)
 *     .background(Color::White)
 *     .build();
 * display.draw(cmd1);
 *
 * // Integer number
 * int temperature = 42;
 * auto cmd2 = TextBuilder()
 *     .number(temperature)
 *     .at({10, 40})
 *     .font(&font)
 *     .build();
 *
 * // Decimal number with 2 decimal places
 * double pi = 3.14159;
 * auto cmd3 = TextBuilder()
 *     .decimal(pi, 2)  // Renders as "3.14"
 *     .at({10, 60})
 *     .font(&font)
 *     .build();
 *
 * // Using Display::text() factory (preferred)
 * display.draw(
 *   display.text("Status")
 *     .at({10, 10})
 *     .font(&font)
 *     .foreground(Color::Black)
 *     .build()
 * );
 *
 * // Applying reusable style
 * TextStyleSpec header_style{&font, Color::Black, Color::White};
 * auto cmd4 = TextBuilder("Title").at({10,10}).with_style(header_style).build();
 * ```
 *
 * @see TextCommand, TextStyleSpec, Font, Display::draw()
 */
class TextBuilder {
public:
  /**
   * @brief Default constructor creates a builder with empty text.
   */
  TextBuilder() noexcept : content_type_(TextContent::String), number_(0), decimal_(0.0), decimal_places_(0) {}

  /**
   * @brief Construct a text builder with string content.
   *
   * @param txt Text content
   */
  explicit TextBuilder(std::string_view txt) noexcept
      : text_(txt), content_type_(TextContent::String), number_(0), decimal_(0.0), decimal_places_(0) {}

  /**
   * @brief Set the text content.
   *
   * @param txt Text string
   * @return Reference to this builder for chaining
   */
  auto text(std::string_view txt) noexcept -> TextBuilder & {
    text_ = txt;
    content_type_ = TextContent::String;
    return *this;
  }

  /**
   * @brief Set the content to an integer number.
   *
   * @param num Integer value
   * @return Reference to this builder for chaining
   */
  auto number(std::int32_t num) noexcept -> TextBuilder & {
    number_ = num;
    content_type_ = TextContent::Number;
    return *this;
  }

  /**
   * @brief Set the content to a decimal number.
   *
   * @param dec Decimal value
   * @param places Number of decimal places
   * @return Reference to this builder for chaining
   */
  auto decimal(double dec, std::uint8_t places) noexcept -> TextBuilder & {
    decimal_ = dec;
    decimal_places_ = places;
    content_type_ = TextContent::Decimal;
    return *this;
  }

  /**
   * @brief Set the text position.
   *
   * @param pt Position point
   * @return Reference to this builder for chaining
   */
  auto at(Point pt) noexcept -> TextBuilder & {
    position_ = pt;
    return *this;
  }

  /**
   * @brief Set the text position using coordinates.
   *
   * @param x X coordinate
   * @param y Y coordinate
   * @return Reference to this builder for chaining
   */
  auto at(std::size_t x, std::size_t y) noexcept -> TextBuilder & {
    position_ = Point{x, y};
    return *this;
  }

  /**
   * @brief Set the font.
   *
   * @param f Font to use
   * @return Reference to this builder for chaining
   */
  auto font(const Font *f) noexcept -> TextBuilder & {
    font_ = f;
    return *this;
  }

  /**
   * @brief Set the foreground (text) color.
   *
   * @param c Foreground color
   * @return Reference to this builder for chaining
   */
  auto foreground(Color c) noexcept -> TextBuilder & {
    foreground_ = c;
    return *this;
  }

  /**
   * @brief Set the background color.
   *
   * @param c Background color
   * @return Reference to this builder for chaining
   */
  auto background(Color c) noexcept -> TextBuilder & {
    background_ = c;
    return *this;
  }

  /**
   * @brief Apply a reusable text style.
   *
   * @param text_style Style specification to apply
   * @return Reference to this builder for chaining
   */
  auto with_style(const TextStyleSpec &text_style) noexcept -> TextBuilder & {
    font_ = text_style.font;
    foreground_ = text_style.foreground;
    background_ = text_style.background;
    return *this;
  }

  /**
   * @brief Build the final TextCommand.
   *
   * @return TextCommand ready to be passed to Display::draw()
   */
  [[nodiscard]] auto build() const -> TextCommand {
    switch (content_type_) {
    case TextContent::String:
      return TextCommand{position_, text_, font_, foreground_, background_};
    case TextContent::Number:
      return TextCommand{position_, number_, font_, foreground_, background_};
    case TextContent::Decimal:
      return TextCommand{position_, decimal_, decimal_places_, font_, foreground_, background_};
    }
    // Should never reach here, but return string version as fallback
    return TextCommand{position_, text_, font_, foreground_, background_};
  }

private:
  Point position_{0, 0};
  std::string text_;
  const Font *font_ = nullptr;
  Color foreground_ = Color::Black;
  Color background_ = Color::White;
  TextContent content_type_;
  std::int32_t number_;
  double decimal_;
  std::uint8_t decimal_places_;
};

} // namespace epaper
