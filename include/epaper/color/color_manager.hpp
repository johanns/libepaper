#pragma once

#include "epaper/color/color.hpp"
#include "epaper/color/device_color.hpp"
#include "epaper/drivers/capabilities.hpp"
#include <cstdint>

namespace epaper {

/**
 * @brief Color conversion and management.
 *
 * Converts high-level RGB/RGBA colors to device-specific formats
 * based on driver capabilities. Handles quantization, dithering,
 * and alpha blending as needed.
 */
class ColorManager {
public:
  ColorManager() = default;

  /**
   * @brief Convert RGB color to 1-bit black/white.
   *
   * @param color RGB color to convert
   * @return Device color in 1-bit format
   */
  [[nodiscard]] static constexpr auto convert_to_bw(const RGB &color) noexcept -> DeviceColor<ColorDepth::Bits1> {
    const auto gray = color.to_grayscale();
    return DeviceColor<ColorDepth::Bits1>{gray >= 128};
  }

  /**
   * @brief Convert RGB color to 2-bit grayscale (4 levels).
   *
   * @param color RGB color to convert
   * @return Device color in 2-bit format
   */
  [[nodiscard]] static constexpr auto convert_to_gray2(const RGB &color) noexcept -> DeviceColor<ColorDepth::Bits2> {
    const auto gray = color.to_grayscale();
    // Map 0-255 to 0-3
    // 0-63 -> 0 (black), 64-127 -> 1, 128-191 -> 2, 192-255 -> 3 (white)
    const std::uint8_t level = gray >> 6; // Divide by 64
    return DeviceColor<ColorDepth::Bits2>{level};
  }

  /**
   * @brief Convert RGB color to 32-bit true color.
   *
   * @param color RGB color to convert
   * @return Device color in 32-bit format
   */
  [[nodiscard]] static constexpr auto convert_to_rgb32(const RGB &color) noexcept -> DeviceColor<ColorDepth::Bits32> {
    return DeviceColor<ColorDepth::Bits32>{color.r, color.g, color.b, 255};
  }

  /**
   * @brief Convert RGBA color to 1-bit black/white with alpha blending.
   *
   * @param color RGBA color to convert
   * @param background Background color for alpha blending
   * @return Device color in 1-bit format
   */
  [[nodiscard]] static constexpr auto convert_to_bw(const RGBA &color, const RGB &background = colors::White) noexcept
      -> DeviceColor<ColorDepth::Bits1> {
    const auto blended = blend_alpha(color, background);
    return convert_to_bw(blended);
  }

  /**
   * @brief Convert RGBA color to 2-bit grayscale with alpha blending.
   *
   * @param color RGBA color to convert
   * @param background Background color for alpha blending
   * @return Device color in 2-bit format
   */
  [[nodiscard]] static constexpr auto convert_to_gray2(const RGBA &color,
                                                       const RGB &background = colors::White) noexcept
      -> DeviceColor<ColorDepth::Bits2> {
    const auto blended = blend_alpha(color, background);
    return convert_to_gray2(blended);
  }

  /**
   * @brief Convert RGBA color to 32-bit true color.
   *
   * @param color RGBA color to convert
   * @return Device color in 32-bit format
   */
  [[nodiscard]] static constexpr auto convert_to_rgb32(const RGBA &color) noexcept -> DeviceColor<ColorDepth::Bits32> {
    return DeviceColor<ColorDepth::Bits32>{color.r, color.g, color.b, color.a};
  }

  /**
   * @brief Generic conversion based on color depth template parameter.
   *
   * @tparam depth Target color depth
   * @param color RGB color to convert
   * @return Device color in specified format
   */
  template <ColorDepth depth>
  [[nodiscard]] static constexpr auto convert(const RGB &color) noexcept -> DeviceColor<depth> {
    if constexpr (depth == ColorDepth::Bits1) {
      return convert_to_bw(color);
    } else if constexpr (depth == ColorDepth::Bits2) {
      return convert_to_gray2(color);
    } else if constexpr (depth == ColorDepth::Bits32) {
      return convert_to_rgb32(color);
    } else {
      return DeviceColor<depth>{0};
    }
  }

  /**
   * @brief Generic conversion based on color depth template parameter with alpha.
   *
   * @tparam depth Target color depth
   * @param color RGBA color to convert
   * @param background Background color for alpha blending
   * @return Device color in specified format
   */
  template <ColorDepth depth>
  [[nodiscard]] static constexpr auto convert(const RGBA &color, const RGB &background = colors::White) noexcept
      -> DeviceColor<depth> {
    if constexpr (depth == ColorDepth::Bits1) {
      return convert_to_bw(color, background);
    } else if constexpr (depth == ColorDepth::Bits2) {
      return convert_to_gray2(color, background);
    } else if constexpr (depth == ColorDepth::Bits32) {
      return convert_to_rgb32(color);
    } else {
      return DeviceColor<depth>{0};
    }
  }

private:
  /**
   * @brief Blend RGBA color with background using alpha.
   *
   * @param color RGBA color with alpha
   * @param background Background RGB color
   * @return Blended RGB color
   */
  [[nodiscard]] static constexpr auto blend_alpha(const RGBA &color, const RGB &background) noexcept -> RGB {
    const auto alpha = static_cast<double>(color.a) / 255.0;
    const auto inv_alpha = 1.0 - alpha;

    const auto r = static_cast<std::uint8_t>(color.r * alpha + background.r * inv_alpha);
    const auto g = static_cast<std::uint8_t>(color.g * alpha + background.g * inv_alpha);
    const auto b = static_cast<std::uint8_t>(color.b * alpha + background.b * inv_alpha);

    return RGB{r, g, b};
  }
};

} // namespace epaper
