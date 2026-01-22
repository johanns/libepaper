#pragma once

#include <cstddef>

namespace epaper {

/**
 * @brief A 2D point in display coordinate space.
 *
 * Represents a position with x (horizontal) and y (vertical) coordinates.
 * Coordinates are in logical space (accounting for display orientation).
 *
 * The origin (0, 0) is at the top-left corner. X increases rightward,
 * Y increases downward. Orientation transforms are handled internally
 * by the framebuffer.
 *
 * @note Point uses std::size_t to prevent negative coordinates and
 *       ensure type safety with display dimensions.
 *
 * @example
 * ```cpp
 * // Create points using aggregate initialization
 * Point origin{0, 0};
 * Point top_right{display.width() - 1, 0};
 * Point center{display.width() / 2, display.height() / 2};
 *
 * // Translate points
 * Point offset{10, 20};
 * Point translated = origin.translate(offset); // {10, 20}
 *
 * // Use in drawing operations
 * display.line(origin, center).color(Color::Black).draw();
 * ```
 *
 * @see Size, Display::set_pixel(), Graphics::draw_line()
 */
struct Point {
  std::size_t x; ///< X coordinate (horizontal position)
  std::size_t y; ///< Y coordinate (vertical position)

  /**
   * @brief Default constructor initializes to origin (0, 0).
   */
  constexpr Point() noexcept : x(0), y(0) {}

  /**
   * @brief Construct a point at the specified coordinates.
   *
   * @param x_coord X coordinate
   * @param y_coord Y coordinate
   */
  constexpr Point(std::size_t x_coord, std::size_t y_coord) noexcept : x(x_coord), y(y_coord) {}

  /**
   * @brief Translate this point by an offset.
   *
   * @param offset The offset to add to this point
   * @return New point with offset applied
   */
  [[nodiscard]] constexpr auto translate(const Point &offset) const noexcept -> Point {
    return Point{x + offset.x, y + offset.y};
  }

  /**
   * @brief Equality comparison.
   */
  [[nodiscard]] constexpr auto operator==(const Point &other) const noexcept -> bool {
    return x == other.x && y == other.y;
  }

  /**
   * @brief Inequality comparison.
   */
  [[nodiscard]] constexpr auto operator!=(const Point &other) const noexcept -> bool { return !(*this == other); }
};

/**
 * @brief A 2D size representing width and height.
 *
 * Used for specifying dimensions of rectangles, bitmaps, and other 2D objects.
 * Width and height are always positive values in pixels.
 *
 * @note Size is separate from Point to enforce type safety - you cannot
 *       accidentally use a size where a position is expected.
 *
 * @example
 * ```cpp
 * // Query display dimensions
 * Size screen_size{display.width(), display.height()};
 * auto pixels = screen_size.area(); // Total pixel count
 *
 * // Bitmap dimensions
 * Size bitmap_size{64, 48};
 * std::vector<uint8_t> bitmap_data(bitmap_size.area());
 *
 * // Rectangle bounds checking
 * Size rect_size{100, 50};
 * if (rect_size.width <= display.width() && rect_size.height <= display.height()) {
 *   display.rectangle({0, 0}, {rect_size.width, rect_size.height}).draw();
 * }
 * ```
 *
 * @see Point, Display::width(), Display::height()
 */
struct Size {
  std::size_t width;  ///< Width in pixels
  std::size_t height; ///< Height in pixels

  /**
   * @brief Default constructor initializes to zero size.
   */
  constexpr Size() noexcept : width(0), height(0) {}

  /**
   * @brief Construct a size with the specified dimensions.
   *
   * @param w Width in pixels
   * @param h Height in pixels
   */
  constexpr Size(std::size_t w, std::size_t h) noexcept : width(w), height(h) {}

  /**
   * @brief Calculate the area (width × height).
   *
   * @return Total area in pixels
   */
  [[nodiscard]] constexpr auto area() const noexcept -> std::size_t { return width * height; }

  /**
   * @brief Equality comparison.
   */
  [[nodiscard]] constexpr auto operator==(const Size &other) const noexcept -> bool {
    return width == other.width && height == other.height;
  }

  /**
   * @brief Inequality comparison.
   */
  [[nodiscard]] constexpr auto operator!=(const Size &other) const noexcept -> bool { return !(*this == other); }
};

} // namespace epaper
