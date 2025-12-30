#pragma once

#include <cstdint>

namespace epaper {

/**
 * @brief RGB color representation.
 *
 * Represents a color using red, green, and blue components.
 * Each component ranges from 0 (no intensity) to 255 (full intensity).
 *
 * @example
 * @code{.cpp}
 * RGB white{255, 255, 255};
 * RGB black{0, 0, 0};
 * RGB red{255, 0, 0};
 * @endcode
 */
struct RGB {
  std::uint8_t b; ///< Blue component (0-255)
  std::uint8_t g; ///< Green component (0-255)
  std::uint8_t r; ///< Red component (0-255)

  /**
   * @brief Default constructor initializes to black.
   */
  constexpr RGB() : b(0), g(0), r(0) {}

  /**
   * @brief Construct RGB color from components.
   *
   * @param red Red component (0-255)
   * @param green Green component (0-255)
   * @param blue Blue component (0-255)
   */
  constexpr RGB(std::uint8_t red, std::uint8_t green, std::uint8_t blue) : b(blue), g(green), r(red) {}

  /**
   * @brief Equality comparison.
   */
  constexpr auto operator==(const RGB &other) const noexcept -> bool = default;

  /**
   * @brief Convert to grayscale using standard luminance formula.
   *
   * @return Grayscale value (0-255)
   */
  [[nodiscard]] constexpr auto to_grayscale() const noexcept -> std::uint8_t {
    return static_cast<std::uint8_t>(0.299 * static_cast<double>(r) + 0.587 * static_cast<double>(g) +
                                     0.114 * static_cast<double>(b));
  }
};

/**
 * @brief RGBA color representation with alpha channel.
 *
 * Extends RGB with an alpha (transparency) component.
 * Alpha ranges from 0 (fully transparent) to 255 (fully opaque).
 *
 * @example
 * @code{.cpp}
 * RGBA opaque_white{255, 255, 255, 255};
 * RGBA transparent_black{0, 0, 0, 0};
 * RGBA semi_transparent_red{255, 0, 0, 128};
 * @endcode
 */
struct RGBA {
  std::uint8_t a; ///< Alpha component (0-255, 0=transparent, 255=opaque)
  std::uint8_t b; ///< Blue component (0-255)
  std::uint8_t g; ///< Green component (0-255)
  std::uint8_t r; ///< Red component (0-255)

  /**
   * @brief Default constructor initializes to transparent black.
   */
  constexpr RGBA() : a(255), b(0), g(0), r(0) {}

  /**
   * @brief Construct RGBA color from components.
   *
   * @param red Red component (0-255)
   * @param green Green component (0-255)
   * @param blue Blue component (0-255)
   * @param alpha Alpha component (0-255, default 255=opaque)
   */
  constexpr RGBA(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha = 255)
      : a(alpha), b(blue), g(green), r(red) {}

  /**
   * @brief Construct RGBA from RGB (fully opaque).
   *
   * @param rgb RGB color
   */
  constexpr explicit RGBA(const RGB &rgb) : a(255), b(rgb.b), g(rgb.g), r(rgb.r) {}

  /**
   * @brief Equality comparison.
   */
  constexpr auto operator==(const RGBA &other) const noexcept -> bool = default;

  /**
   * @brief Convert to RGB (discards alpha channel).
   *
   * @return RGB color
   */
  [[nodiscard]] constexpr auto to_rgb() const noexcept -> RGB { return RGB{r, g, b}; }

  /**
   * @brief Convert to grayscale using standard luminance formula.
   *
   * @return Grayscale value (0-255)
   */
  [[nodiscard]] constexpr auto to_grayscale() const noexcept -> std::uint8_t {
    return static_cast<std::uint8_t>(0.299 * static_cast<double>(r) + 0.587 * static_cast<double>(g) +
                                     0.114 * static_cast<double>(b));
  }
};

// Predefined colors
namespace colors {
constexpr RGB Black{0, 0, 0};
constexpr RGB White{255, 255, 255};
constexpr RGB Red{255, 0, 0};
constexpr RGB Green{0, 255, 0};
constexpr RGB Blue{0, 0, 255};
constexpr RGB Yellow{255, 255, 0};
constexpr RGB Cyan{0, 255, 255};
constexpr RGB Magenta{255, 0, 255};
constexpr RGB Gray{128, 128, 128};
constexpr RGB DarkGray{64, 64, 64};
constexpr RGB LightGray{192, 192, 192};
} // namespace colors

} // namespace epaper
