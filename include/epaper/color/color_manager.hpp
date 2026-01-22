#pragma once

#include "epaper/color/color.hpp"
#include "epaper/color/device_color.hpp"
#include "epaper/core/types.hpp"
#include "epaper/drivers/driver.hpp"
#include <algorithm>
#include <cstdint>
#include <span>
#include <vector>

namespace epaper {

/**
 * @brief Color conversion and management.
 *
 * Converts high-level RGB/RGBA colors to device-specific formats based on
 * driver capabilities. Handles quantization, color matching, and alpha blending.
 *
 * **Conversion Algorithms:**
 * - BlackWhite: Luminance threshold (grayscale >= 128 → white)
 * - Grayscale4: 4-level quantization (0-255 → 0-3)
 * - BWR/BWY: Nearest color matching using Euclidean distance in RGB space
 * - Spectra6: 6-color nearest neighbor matching
 *
 * **Color Matching:**
 * - Uses Euclidean distance in RGB color space: d² = (r₁-r₂)² + (g₁-g₂)² + (b₁-b₂)²
 * - No perceptual color space (CIE Lab) - optimized for simplicity and speed
 * - Suitable for e-paper's limited color gamut
 *
 * **Alpha Blending:**
 * - Standard alpha compositing: C_out = C_fg × α + C_bg × (1 - α)
 * - Background defaults to white (typical e-paper substrate color)
 * - Performed before quantization/color matching
 *
 * **Dithering:**
 * - Not currently implemented (future enhancement)
 * - Simple threshold/nearest neighbor sufficient for most e-paper use cases
 * - Floyd-Steinberg dithering could improve gradient rendering
 *
 * @note All conversion functions are constexpr - can be evaluated at compile time
 *       for constant colors, improving performance.
 *
 * @example
 * ```cpp
 * // Convert RGB to display-specific format
 * RGB orange{255, 165, 0};
 *
 * // BlackWhite mode - luminance threshold
 * auto bw = ColorManager::convert_to_bw(orange);
 * // orange.to_grayscale() = 179, >= 128 → white
 *
 * // BWR mode - nearest color matching
 * auto bwr = ColorManager::convert_to_bwr(orange);
 * // Closest to Red (distance to Red < distance to Black/White)
 *
 * // RGBA with alpha blending
 * RGBA semi_transparent_red{255, 0, 0, 128};  // 50% alpha
 * auto bw_alpha = ColorManager::convert_to_bw(semi_transparent_red, colors::White);
 * // Blends to RGB{255, 128, 128}, then converts to white
 *
 * // Generic template-based conversion
 * auto gray = ColorManager::convert<DisplayMode::Grayscale4>(orange);
 * ```
 *
 * @see RGB, RGBA, DeviceColor, DisplayMode
 */
class ColorManager {
private:
  /**
   * @brief Calculate squared Euclidean distance between two colors in RGB space.
   *
   * Used for nearest-color matching in multi-color modes (BWR, BWY, Spectra6).
   * Returns squared distance to avoid expensive sqrt() operation - comparison
   * ordering is preserved.
   *
   * Formula: d² = (r₁-r₂)² + (g₁-g₂)² + (b₁-b₂)²
   *
   * @param c1 First color
   * @param c2 Second color
   * @return Squared distance (0 = identical colors, max ~195075 for black vs white)
   */
  [[nodiscard]] static constexpr auto distance_sq(const RGB &c1, const RGB &c2) noexcept -> int {
    int dr = static_cast<int>(c1.r) - static_cast<int>(c2.r);
    int dg = static_cast<int>(c1.g) - static_cast<int>(c2.g);
    int db = static_cast<int>(c1.b) - static_cast<int>(c2.b);
    return (dr * dr) + (dg * dg) + (db * db);
  }

public:
  ColorManager() = default;

  /**
   * @brief Convert Color enum to RGB.
   *
   * @param color Color enum value
   * @return Corresponding RGB color
   */
  [[nodiscard]] static constexpr auto to_rgb(Color color) noexcept -> RGB {
    switch (color) {
    case Color::Black:
      return colors::Black;
    case Color::White:
      return colors::White;
    case Color::Red:
      return colors::Red;
    case Color::Green:
      return colors::Green;
    case Color::Blue:
      return colors::Blue;
    case Color::Yellow:
      return colors::Yellow;
    case Color::Gray1:
      return colors::LightGray;
    case Color::Gray2:
      return colors::DarkGray;
    default:
      return colors::White;
    }
  }

  /**
   * @brief Convert RGB color to 1-bit black/white.
   *
   * @param color RGB color to convert
   * @return Device color in 1-bit format
   */
  [[nodiscard]] static constexpr auto convert_to_bw(const RGB &color) noexcept -> DeviceColor<DisplayMode::BlackWhite> {
    const auto gray = color.to_grayscale();
    return DeviceColor<DisplayMode::BlackWhite>{gray >= 128};
  }

  /**
   * @brief Convert RGB color to 2-bit grayscale (4 levels).
   *
   * Maps 0-255 luminance range to 4 discrete gray levels:
   * - Level 0 (00): 0-63   → Black
   * - Level 1 (01): 64-127 → Dark gray
   * - Level 2 (10): 128-191 → Light gray
   * - Level 3 (11): 192-255 → White
   *
   * Uses ITU-R BT.601 luminance formula via RGB::to_grayscale().
   *
   * @param color RGB color to convert
   * @return Device color in 2-bit format (level 0-3)
   *
   * @example
   * ```cpp
   * RGB dark_gray{64, 64, 64};   // luminance ~64
   * auto result = ColorManager::convert_to_gray4(dark_gray);
   * // result.level == 1 (dark gray)
   *
   * RGB medium{150, 150, 150};   // luminance ~150
   * auto result2 = ColorManager::convert_to_gray4(medium);
   * // result2.level == 2 (light gray)
   * ```
   */
  [[nodiscard]] static constexpr auto convert_to_gray4(const RGB &color) noexcept
      -> DeviceColor<DisplayMode::Grayscale4> {
    const auto gray = color.to_grayscale();
    // Map 0-255 to 0-3
    // 0-63 -> 0 (black), 64-127 -> 1, 128-191 -> 2, 192-255 -> 3 (white)
    const std::uint8_t level = gray >> 6; // Divide by 64
    return DeviceColor<DisplayMode::Grayscale4>{level};
  }

  [[nodiscard]] static constexpr auto convert_to_bwr(const RGB &color) noexcept -> DeviceColor<DisplayMode::BWR> {
    // Current options: Black, White, Red
    const int d_black = distance_sq(color, colors::Black);
    const int d_white = distance_sq(color, colors::White);
    const int d_red = distance_sq(color, colors::Red);

    if (d_red < d_black && d_red < d_white) {
      return DeviceColor<DisplayMode::BWR>{TriColor::Third}; // Red
    }
    if (d_black < d_white) {
      return DeviceColor<DisplayMode::BWR>{TriColor::Black};
    }
    return DeviceColor<DisplayMode::BWR>{TriColor::White};
  }

  [[nodiscard]] static constexpr auto convert_to_bwy(const RGB &color) noexcept -> DeviceColor<DisplayMode::BWY> {
    // Current options: Black, White, Yellow
    const int d_black = distance_sq(color, colors::Black);
    const int d_white = distance_sq(color, colors::White);
    const int d_yellow = distance_sq(color, colors::Yellow);

    if (d_yellow < d_black && d_yellow < d_white) {
      return DeviceColor<DisplayMode::BWY>{TriColor::Third}; // Yellow
    }
    if (d_black < d_white) {
      return DeviceColor<DisplayMode::BWY>{TriColor::Black};
    }
    return DeviceColor<DisplayMode::BWY>{TriColor::White};
  }

  [[nodiscard]] static constexpr auto convert_to_spectra6(const RGB &color) noexcept
      -> DeviceColor<DisplayMode::Spectra6> {
    // Colors: Black, White, Red, Green, Blue, Yellow
    const int d_black = distance_sq(color, colors::Black);
    const int d_white = distance_sq(color, colors::White);
    const int d_red = distance_sq(color, colors::Red);
    const int d_green = distance_sq(color, colors::Green);
    const int d_blue = distance_sq(color, colors::Blue);
    const int d_yellow = distance_sq(color, colors::Yellow);

    int min_d = d_black;
    auto result = Spectra6Color::Black;

    if (d_white < min_d) {
      min_d = d_white;
      result = Spectra6Color::White;
    }
    if (d_red < min_d) {
      min_d = d_red;
      result = Spectra6Color::Red;
    }
    if (d_green < min_d) {
      min_d = d_green;
      result = Spectra6Color::Green;
    }
    if (d_blue < min_d) {
      min_d = d_blue;
      result = Spectra6Color::Blue;
    }
    if (d_yellow < min_d) {
      min_d = d_yellow;
      result = Spectra6Color::Yellow;
    }
    return DeviceColor<DisplayMode::Spectra6>{result};
  }

  /**
   * @brief Convert RGBA color to 1-bit black/white with alpha blending.
   *
   * @param color RGBA color to convert
   * @param background Background color for alpha blending
   * @return Device color in 1-bit format
   */
  [[nodiscard]] static constexpr auto convert_to_bw(const RGBA &color, const RGB &background = colors::White) noexcept
      -> DeviceColor<DisplayMode::BlackWhite> {
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
  [[nodiscard]] static constexpr auto convert_to_gray4(const RGBA &color,
                                                       const RGB &background = colors::White) noexcept
      -> DeviceColor<DisplayMode::Grayscale4> {
    const auto blended = blend_alpha(color, background);
    return convert_to_gray4(blended);
  }

  /**
   * @brief Generic conversion based on display mode template parameter.
   *
   * @tparam Mode Target display mode
   * @param color RGB color to convert
   * @return Device color in specified format
   */
  template <DisplayMode Mode>
  [[nodiscard]] static constexpr auto convert(const RGB &color) noexcept -> DeviceColor<Mode> {
    if constexpr (Mode == DisplayMode::BlackWhite) {
      return convert_to_bw(color);
    } else if constexpr (Mode == DisplayMode::Grayscale4) {
      return convert_to_gray4(color);
    } else if constexpr (Mode == DisplayMode::BWR) {
      return convert_to_bwr(color);
    } else if constexpr (Mode == DisplayMode::BWY) {
      return convert_to_bwy(color);
    } else if constexpr (Mode == DisplayMode::Spectra6) {
      return convert_to_spectra6(color);
    } else {
      return DeviceColor<Mode>{};
    }
  }

  /**
   * @brief Generic conversion based on display mode template parameter with alpha.
   *
   * @tparam Mode Target display mode
   * @param color RGBA color to convert
   * @param background Background color for alpha blending
   * @return Device color in specified format
   */
  template <DisplayMode Mode>
  [[nodiscard]] static constexpr auto convert(const RGBA &color, const RGB &background = colors::White) noexcept
      -> DeviceColor<Mode> {
    if constexpr (Mode == DisplayMode::BlackWhite) {
      return convert_to_bw(color, background);
    } else if constexpr (Mode == DisplayMode::Grayscale4) {
      return convert_to_gray4(color, background);
    } else {
      // For color modes, blend and default
      const auto blended = blend_alpha(color, background);
      return convert<Mode>(blended);
    }
  }

  /**
   * @brief Dither an RGB image to device colors using Floyd-Steinberg.
   *
   * @tparam Mode Target display mode
   * @tparam SetPixelFunc Function type for setting pixels (x, y, DeviceColor<Mode>)
   * @param rgb_data Packed RGB data (R, G, B, R, G, B...)
   * @param width Image width
   * @param height Image height
   * @param set_pixel Callback to set quantized pixel
   */
  template <DisplayMode Mode, typename SetPixelFunc>
  static void dither_image(std::span<const std::uint8_t> rgb_data, std::size_t width, std::size_t height,
                           SetPixelFunc set_pixel) {
    if (rgb_data.size() < width * height * 3) {
      return;
    }

    // Floyd-Steinberg Error Diffusion Dithering
    // Algorithm: Quantize each pixel and distribute quantization error to
    // neighboring unprocessed pixels using weighted error diffusion kernel.
    //
    // Error diffusion kernel (fractions of quantization error):
    //              X   7/16
    //      3/16  5/16  1/16
    //
    // Where X is current pixel. Error propagates right and down only,
    // allowing single-pass left-to-right, top-to-bottom processing.

    struct RGBError {
      int r, g, b;
    };
    std::vector<RGBError> pixels(width * height);

    // Initialize working buffer with RGB values as signed integers
    // to allow negative intermediate values during error propagation
    for (std::size_t i = 0; i < width * height; ++i) {
      pixels[i] = {static_cast<int>(rgb_data[i * 3]), static_cast<int>(rgb_data[(i * 3) + 1]),
                   static_cast<int>(rgb_data[(i * 3) + 2])};
    }

    // Clamp values to [0, 255] after error accumulation
    auto clamp = [](int v) -> std::uint8_t { return static_cast<std::uint8_t>(std::max(0, std::min(255, v))); };

    // Process pixels in raster scan order (left-to-right, top-to-bottom)
    for (std::size_t y = 0; y < height; ++y) {
      for (std::size_t x = 0; x < width; ++x) {
        std::size_t i = (y * width) + x;
        RGB current{clamp(pixels[i].r), clamp(pixels[i].g), clamp(pixels[i].b)};

        // Step 1: Quantize current pixel to nearest device color
        auto dev_color = convert<Mode>(current);
        set_pixel(x, y, dev_color);

        // Step 2: Calculate quantization error (original - quantized)
        RGB quantized = dev_color.to_rgb();
        int er = static_cast<int>(current.r) - static_cast<int>(quantized.r);
        int eg = static_cast<int>(current.g) - static_cast<int>(quantized.g);
        int eb = static_cast<int>(current.b) - static_cast<int>(quantized.b);

        // Step 3: Distribute quantization error to neighboring pixels
        // Floyd-Steinberg error diffusion weights:
        //   Right neighbor (x+1, y):     7/16 of error
        //   Bottom-left (x-1, y+1):      3/16 of error
        //   Bottom (x, y+1):             5/16 of error
        //   Bottom-right (x+1, y+1):     1/16 of error
        auto add_error = [&](std::size_t idx, double factor) {
          pixels[idx].r += static_cast<int>(er * factor);
          pixels[idx].g += static_cast<int>(eg * factor);
          pixels[idx].b += static_cast<int>(eb * factor);
        };

        // Diffuse to right neighbor (7/16)
        if (x + 1 < width) {
          add_error((y * width) + (x + 1), 7.0 / 16.0);
        }

        // Diffuse to next row (if not at bottom edge)
        if (y + 1 < height) {
          // Bottom-left diagonal (3/16)
          if (x > 0) {
            add_error(((y + 1) * width) + (x - 1), 3.0 / 16.0);
          }
          // Directly below (5/16)
          add_error(((y + 1) * width) + x, 5.0 / 16.0);
          // Bottom-right diagonal (1/16)
          if (x + 1 < width) {
            add_error(((y + 1) * width) + (x + 1), 1.0 / 16.0);
          }
        }
      }
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

    const auto r = static_cast<std::uint8_t>((color.r * alpha) + (background.r * inv_alpha));
    const auto g = static_cast<std::uint8_t>((color.g * alpha) + (background.g * inv_alpha));
    const auto b = static_cast<std::uint8_t>((color.b * alpha) + (background.b * inv_alpha));

    return RGB{r, g, b};
  }
};

} // namespace epaper
