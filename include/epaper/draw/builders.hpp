#pragma once

#include "epaper/display.hpp"
#include "epaper/draw/commands.hpp"
#include "epaper/draw/styles.hpp"
#include "epaper/font.hpp"
#include "epaper/geometry.hpp"
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
 * @example
 * @code{.cpp}
 * auto cmd = LineBuilder()
 *     .from({10, 10})
 *     .to({100, 100})
 *     .color(Color::Black)
 *     .width(DotPixel::Pixel2x2)
 *     .style(LineStyle::Dotted)
 *     .build();
 * display.draw(cmd);
 * @endcode
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
 * @example
 * @code{.cpp}
 * // Using corners
 * auto cmd1 = RectangleBuilder()
 *     .top_left({10, 10})
 *     .bottom_right({100, 50})
 *     .color(Color::Black)
 *     .fill(DrawFill::Empty)
 *     .build();
 *
 * // Using position + size
 * auto cmd2 = RectangleBuilder()
 *     .at({10, 10})
 *     .size({90, 40})
 *     .color(Color::Black)
 *     .fill(DrawFill::Full)
 *     .build();
 * @endcode
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
   * @return Reference to this builder for chaining
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
 * @example
 * @code{.cpp}
 * auto cmd = CircleBuilder()
 *     .center({50, 50})
 *     .radius(25)
 *     .color(Color::Black)
 *     .fill(DrawFill::Full)
 *     .build();
 * display.draw(cmd);
 * @endcode
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
 * Provides a fluent interface for specifying point drawing parameters.
 *
 * @example
 * @code{.cpp}
 * auto cmd = PointBuilder()
 *     .at({10, 10})
 *     .color(Color::Black)
 *     .size(DotPixel::Pixel3x3)
 *     .build();
 * display.draw(cmd);
 * @endcode
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
 * Supports strings, integers, and decimal numbers.
 *
 * @example
 * @code{.cpp}
 * // String
 * auto cmd1 = TextBuilder("Hello")
 *     .at({10, 20})
 *     .font(&Font::font16())
 *     .foreground(Color::Black)
 *     .background(Color::White)
 *     .build();
 *
 * // Number
 * auto cmd2 = TextBuilder()
 *     .number(42)
 *     .at({10, 40})
 *     .font(&Font::font16())
 *     .build();
 *
 * // Decimal
 * auto cmd3 = TextBuilder()
 *     .decimal(3.14159, 2)
 *     .at({10, 60})
 *     .font(&Font::font16())
 *     .build();
 * @endcode
 */
class TextBuilder {
public:
  /**
   * @brief Default constructor creates a builder with empty text.
   */
  TextBuilder() noexcept : text_(), content_type_(TextContent::String), number_(0), decimal_(0.0), decimal_places_(0) {}

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
