#pragma once

#include <cstddef>
#include <cstdint>

namespace epaper {

/**
 * @brief Display mode enumeration.
 *
 * Defines the color/grayscale mode a display can operate in.
 * Each mode implies specific bits-per-pixel, color capability, and
 * hardware buffer requirements.
 *
 * **Mode Characteristics:**
 * - BlackWhite: 1 bpp, 2 colors, single buffer
 * - Grayscale4: 2 bpp, 4 gray levels, single buffer
 * - BWR/BWY: 2 bpp, 3 colors, dual-buffer (black/white + red/yellow)
 * - Spectra6: 3 bpp, 6 colors, single 3-bit buffer
 *
 * **Mode Selection Guidelines:**
 * - Use BlackWhite for fastest refresh and maximum contrast
 * - Use Grayscale4 for anti-aliased text or smooth gradients
 * - Use BWR/BWY for highlights, warnings, or accent colors
 * - Use Spectra6 for colorful diagrams or illustrations (slower refresh)
 *
 * @note Not all modes are supported by all drivers. Check driver_traits<Driver>::max_mode
 *       and driver_traits<Driver>::supports_grayscale before creating display.
 *
 * @example
 * ```cpp
 * // Query mode capabilities
 * DisplayMode mode = DisplayMode::BWR;
 * auto bpp = bits_per_pixel(mode);      // Returns 2
 * bool has_color = is_color_mode(mode); // Returns true
 * auto planes = num_planes(mode);       // Returns 2
 *
 * // Mode-specific display creation
 * if (driver_traits<EPD27>::max_mode >= DisplayMode::BWR) {
 *   auto display = create_display<EPD27>(device, DisplayMode::BWR);
 * }
 * ```
 *
 * @see bits_per_pixel(), is_color_mode(), num_planes(), driver_traits
 */
enum class DisplayMode : std::uint8_t {
  BlackWhite, ///< 1-bit black and white (2 colors)
  Grayscale4, ///< 2-bit 4-level grayscale
  BWR,        ///< Black, White, Red (3 colors, typically 2-bit)
  BWY,        ///< Black, White, Yellow (3 colors, typically 2-bit)
  Spectra6    ///< 6-color: Black, White, Red, Yellow, Blue, Green (3-bit)
};

/**
 * @brief Get bits per pixel for a display mode.
 *
 * @param mode Display mode
 * @return Bits per pixel required for this mode
 */
[[nodiscard]] constexpr auto bits_per_pixel(DisplayMode mode) noexcept -> std::uint8_t {
  switch (mode) {
  case DisplayMode::BlackWhite:
    return 1;
  case DisplayMode::Grayscale4:
  case DisplayMode::BWR:
  case DisplayMode::BWY:
    return 2;
  case DisplayMode::Spectra6:
    return 3;
  }
  return 1; // Default fallback
}

/**
 * @brief Check if mode supports color (non-grayscale).
 *
 * @param mode Display mode
 * @return true if mode supports color
 */
[[nodiscard]] constexpr auto is_color_mode(DisplayMode mode) noexcept -> bool {
  switch (mode) {
  case DisplayMode::BWR:
  case DisplayMode::BWY:
  case DisplayMode::Spectra6:
    return true;
  default:
    return false;
  }
}

/**
 * @brief Get number of color planes required for a display mode.
 *
 * @param mode Display mode
 * @return Number of planes (1 for monochrome/grayscale, 2+ for color)
 */
[[nodiscard]] constexpr auto num_planes(DisplayMode mode) noexcept -> std::size_t {
  switch (mode) {
  case DisplayMode::BlackWhite:
  case DisplayMode::Grayscale4:
    return 1;
  case DisplayMode::BWR:
  case DisplayMode::BWY:
    return 2;
  case DisplayMode::Spectra6:
    return 1; // Spectra6 uses single 3-bit buffer
  }
  return 1;
}

} // namespace epaper
