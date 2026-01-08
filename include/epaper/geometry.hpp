#pragma once

#include <cstddef>

namespace epaper {

/**
 * @brief A 2D point in display coordinate space.
 *
 * Represents a position with x (horizontal) and y (vertical) coordinates.
 * Coordinates are in logical space (accounting for display orientation).
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
