#pragma once

#include <cstdint>

namespace epaper {

/**
 * @brief RGB color representation.
 *
 * Represents a color using red, green, and blue components in the sRGB color space.
 * Each component ranges from 0 (no intensity) to 255 (full intensity).
 *
 * **Memory Layout:**
 * - Components stored in BGR order (b, g, r) for compatibility with common image formats
 * - Total size: 3 bytes
 * - Alignment: Default struct alignment
 *
 * **Color Space:**
 * - Uses standard sRGB color space (most common for displays and images)
 * - No gamma correction applied - linear RGB values
 * - Suitable for color matching and distance calculations
 *
 * **Conversion:**
 * - to_grayscale() uses ITU-R BT.601 standard luminance formula
 * - Weights: R=0.299, G=0.587, B=0.114 (human eye sensitivity)
 *
 * @example
 * ```cpp
 * // Predefined colors
 * RGB white{255, 255, 255};
 * RGB black{0, 0, 0};
 * RGB red{255, 0, 0};
 *
 * // Custom colors
 * RGB orange{255, 165, 0};
 * RGB navy{0, 0, 128};
 *
 * // Grayscale conversion
 * RGB purple{128, 0, 128};
 * std::uint8_t gray = purple.to_grayscale();  // ~38 (weighted average)
 *
 * // Color comparison
 * if (orange == RGB{255, 165, 0}) {
 *   // Colors match exactly
 * }
 * ```
 *
 * @see RGBA, ColorManager, colors namespace
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
    return static_cast<std::uint8_t>((0.299 * static_cast<double>(r)) + (0.587 * static_cast<double>(g)) +
                                     (0.114 * static_cast<double>(b)));
  }
};

/**
 * @brief RGBA color representation with alpha channel.
 *
 * Extends RGB with an alpha (transparency) component. Used for compositing
 * operations, image loading, and rendering with transparency.
 *
 * **Alpha Channel:**
 * - Alpha ranges from 0 (fully transparent) to 255 (fully opaque)
 * - 0 = invisible (0% opacity)
 * - 128 = semi-transparent (50% opacity)
 * - 255 = fully visible (100% opacity)
 *
 * **Memory Layout:**
 * - Components stored in ABGR order (a, b, g, r)
 * - Total size: 4 bytes
 * - Common format for PNG images and compositing
 *
 * **Alpha Blending:**
 * - ColorManager performs alpha blending when converting to display colors
 * - Blending formula: result = (src * alpha + bg * (255 - alpha)) / 255
 * - Background color determined by display mode or user specification
 *
 * @note E-paper displays do not support partial transparency at the hardware level.
 *       Alpha channel is used during conversion to determine final pixel color.
 *
 * @example
 * ```cpp
 * // Fully opaque colors
 * RGBA opaque_white{255, 255, 255, 255};
 * RGBA opaque_red{255, 0, 0, 255};
 *
 * // Transparent and semi-transparent
 * RGBA transparent{0, 0, 0, 0};        // Invisible
 * RGBA semi_red{255, 0, 0, 128};       // 50% transparent red
 *
 * // Convert from RGB (defaults to opaque)
 * RGB base_color{0, 128, 255};
 * RGBA with_alpha{base_color};  // Alpha = 255
 *
 * // Extract RGB for display
 * RGBA color{255, 128, 64, 200};
 * RGB display_color = color.to_rgb();  // Alpha discarded
 * ```
 *
 * @see RGB, ColorManager::convert_to_bw_with_alpha()
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
    return static_cast<std::uint8_t>((0.299 * static_cast<double>(r)) + (0.587 * static_cast<double>(g)) +
                                     (0.114 * static_cast<double>(b)));
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
