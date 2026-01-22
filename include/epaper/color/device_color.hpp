#pragma once

#include "epaper/color/color.hpp" // Added include for RGB
#include "epaper/drivers/driver.hpp"
#include <cstdint>

namespace epaper {

/**
 * @brief Device-specific color representation (generic template).
 *
 * Stores color in the format required by the target device. The actual
 * representation depends on the device's display mode - this generic template
 * provides a fallback, but mode-specific specializations are used in practice.
 *
 * **Specializations:**
 * - DisplayMode::BlackWhite → bool (1 bit)
 * - DisplayMode::Grayscale4 → std::uint8_t level (2 bits, 0-3)
 * - DisplayMode::BWR/BWY → TriColor enum (Black/White/Red or Yellow)
 * - DisplayMode::Spectra6 → Spectra6Color enum (6 colors)
 *
 * @tparam Mode Display mode of the device
 *
 * @see ColorManager::convert(), DisplayMode
 */
template <DisplayMode Mode> struct DeviceColor {
  std::uint8_t value; ///< Raw color value

  constexpr explicit DeviceColor(std::uint8_t val = 0) : value(val) {}

  constexpr auto operator==(const DeviceColor &other) const noexcept -> bool = default;
};

/**
 * @brief Specialization for 1-bit (black and white) devices.
 *
 * **Bit Layout:**
 * - Single bit per pixel
 * - 1 (true) = white, 0 (false) = black
 * - 8 pixels packed per byte in framebuffer
 *
 * **Memory Representation:**
 * - Stored as bool for type safety and clarity
 * - to_byte() converts to 0x00 (black) or 0xFF (white) for bulk operations
 *
 * @example
 * ```cpp
 * DeviceColor<DisplayMode::BlackWhite> white{true};
 * DeviceColor<DisplayMode::BlackWhite> black{false};
 *
 * std::uint8_t byte_val = white.to_byte();  // 0xFF
 * RGB rgb = white.to_rgb();                 // {255, 255, 255}
 * ```
 */
template <> struct DeviceColor<DisplayMode::BlackWhite> {
  bool is_white; ///< true = white, false = black

  constexpr explicit DeviceColor(bool white = true) : is_white(white) {}

  constexpr auto operator==(const DeviceColor &other) const noexcept -> bool = default;

  [[nodiscard]] constexpr auto to_byte() const noexcept -> std::uint8_t { return is_white ? 0xFF : 0x00; }
  [[nodiscard]] constexpr auto to_rgb() const noexcept -> RGB { return is_white ? colors::White : colors::Black; }
};

/**
 * @brief Specialization for 2-bit (4-level grayscale) devices.
 *
 * **Bit Layout:**
 * - 2 bits per pixel (4 discrete levels)
 * - Level encoding: 00=black, 01=dark gray, 10=light gray, 11=white
 * - 4 pixels packed per byte in framebuffer
 *
 * **Quantization Levels:**
 * ```
 * Level | Binary | Byte Value | RGB Equivalent
 * ------|--------|------------|---------------
 *   0   |   00   |    0x00    | (0, 0, 0) Black
 *   1   |   01   |    0x40    | (64, 64, 64) Dark Gray
 *   2   |   10   |    0x80    | (192, 192, 192) Light Gray
 *   3   |   11   |    0xC0    | (255, 255, 255) White
 * ```
 *
 * **Memory Efficiency:**
 * - 4 pixels per byte reduces memory by 75% vs 8-bit grayscale
 * - Sufficient for e-paper anti-aliasing and smooth gradients
 *
 * @example
 * ```cpp
 * DeviceColor<DisplayMode::Grayscale4> black{0};
 * DeviceColor<DisplayMode::Grayscale4> dark_gray{1};
 * DeviceColor<DisplayMode::Grayscale4> light_gray{2};
 * DeviceColor<DisplayMode::Grayscale4> white{3};
 *
 * std::uint8_t byte = dark_gray.to_byte();  // 0x40
 * RGB rgb = dark_gray.to_rgb();             // colors::DarkGray
 * ```
 */
template <> struct DeviceColor<DisplayMode::Grayscale4> {
  std::uint8_t level; ///< Grayscale level (0-3, where 3 is white)

  constexpr explicit DeviceColor(std::uint8_t gray_level = 3) : level(gray_level & 0x03) {}

  constexpr auto operator==(const DeviceColor &other) const noexcept -> bool = default;

  [[nodiscard]] constexpr auto to_byte() const noexcept -> std::uint8_t {
    // Map 2-bit level to 8-bit value
    // 0 -> 0x00 (black), 1 -> 0x40, 2 -> 0x80, 3 -> 0xC0 (white)
    return static_cast<std::uint8_t>(level << 6);
  }

  [[nodiscard]] constexpr auto to_rgb() const noexcept -> RGB {
    switch (level) {
    case 0:
      return colors::Black;
    case 1:
      return colors::DarkGray;
    case 2:
      return colors::LightGray;
    default:
      return colors::White;
    }
  }
};

/**
 * @brief Color values for 3-color displays (Black/White/Red or Black/White/Yellow).
 *
 * Used by BWR and BWY display modes which employ dual-plane architecture:
 * - Plane 0: Black/White data (1 bpp)
 * - Plane 1: Accent color data (1 bpp)
 *
 * **Dual-Plane Encoding:**
 * ```
 * Color   | Plane 0 | Plane 1 | TriColor Value
 * --------|---------|---------|---------------
 * Black   |    1    |    1    | Black (0)
 * White   |    0    |    1    | White (1)
 * Red/Yel |    0    |    0    | Third (2)
 * ```
 *
 * @note The Third value represents Red for BWR mode and Yellow for BWY mode.
 */
enum class TriColor : std::uint8_t {
  Black = 0,
  White = 1,
  Third = 2 ///< Red for BWR, Yellow for BWY
};

/**
 * @brief Specialization for Black/White/Red displays.
 *
 * **Dual-Plane Architecture:**
 * - Plane 0 (Black/White): 1 bpp buffer for base monochrome image
 * - Plane 1 (Red): 1 bpp buffer for red accent pixels
 * - Both planes transferred separately to display controller
 *
 * **Bit Extraction:**
 * - get_bw_bit(): Returns Plane 0 value (false=black, true=white/red)
 * - get_color_bit(): Returns Plane 1 value (false=red, true=white/black)
 *
 * **Typical Usage:**
 * ```cpp
 * DeviceColor<DisplayMode::BWR> red{TriColor::Third};
 * bool plane0 = red.get_bw_bit();    // false (not black)
 * bool plane1 = red.get_color_bit(); // false (is red)
 *
 * // Set framebuffer planes
 * framebuffer.set_plane_0(x, y, plane0);
 * framebuffer.set_plane_1(x, y, plane1);
 * ```
 *
 * @see MultiPlaneFramebuffer, DisplayMode::BWR
 */
template <> struct DeviceColor<DisplayMode::BWR> {
  TriColor color; ///< Color value (Black, White, or Red)

  constexpr explicit DeviceColor(TriColor c = TriColor::White) : color(c) {}

  constexpr auto operator==(const DeviceColor &other) const noexcept -> bool = default;

  [[nodiscard]] constexpr auto to_byte() const noexcept -> std::uint8_t { return static_cast<std::uint8_t>(color); }

  [[nodiscard]] constexpr auto get_bw_bit() const noexcept -> bool { return color != TriColor::Black; }
  [[nodiscard]] constexpr auto get_color_bit() const noexcept -> bool { return color != TriColor::Third; }

  [[nodiscard]] constexpr auto to_rgb() const noexcept -> RGB {
    switch (color) {
    case TriColor::Black:
      return colors::Black;
    case TriColor::Third:
      return colors::Red;
    default:
      return colors::White;
    }
  }
};

/**
 * @brief Specialization for Black/White/Yellow displays.
 */
template <> struct DeviceColor<DisplayMode::BWY> {
  TriColor color; ///< Color value (Black, White, or Yellow)

  constexpr explicit DeviceColor(TriColor c = TriColor::White) : color(c) {}

  constexpr auto operator==(const DeviceColor &other) const noexcept -> bool = default;

  [[nodiscard]] constexpr auto to_byte() const noexcept -> std::uint8_t { return static_cast<std::uint8_t>(color); }

  [[nodiscard]] constexpr auto get_bw_bit() const noexcept -> bool { return color != TriColor::Black; }
  [[nodiscard]] constexpr auto get_color_bit() const noexcept -> bool { return color != TriColor::Third; }

  [[nodiscard]] constexpr auto to_rgb() const noexcept -> RGB {
    switch (color) {
    case TriColor::Black:
      return colors::Black;
    case TriColor::Third:
      return colors::Yellow;
    default:
      return colors::White;
    }
  }
};

/**
 * @brief Color values for Spectra 6 displays.
 *
 * **6-Color Palette:**
 * - Supports full primary and secondary colors in RGB space
 * - Each color encoded as 3-bit value (0-5, values 6-7 unused)
 * - Pixels packed tightly in framebuffer (may span byte boundaries)
 *
 * **Bit Encoding:**
 * ```
 * Color  | Value | Binary | RGB Equivalent
 * -------|-------|--------|---------------
 * Black  |   0   |  000   | (0, 0, 0)
 * White  |   1   |  001   | (255, 255, 255)
 * Red    |   2   |  010   | (255, 0, 0)
 * Yellow |   3   |  011   | (255, 255, 0)
 * Blue   |   4   |  100   | (0, 0, 255)
 * Green  |   5   |  101   | (0, 255, 0)
 * ```
 *
 * @note Spectra6 uses single-plane 3bpp encoding, unlike BWR/BWY dual-plane.
 */
enum class Spectra6Color : std::uint8_t { Black = 0, White = 1, Red = 2, Yellow = 3, Blue = 4, Green = 5 };

/**
 * @brief Specialization for Spectra 6 (6-color) displays.
 */
template <> struct DeviceColor<DisplayMode::Spectra6> {
  Spectra6Color color; ///< Color value (one of 6 colors)

  constexpr explicit DeviceColor(Spectra6Color c = Spectra6Color::White) : color(c) {}

  constexpr auto operator==(const DeviceColor &other) const noexcept -> bool = default;

  [[nodiscard]] constexpr auto to_byte() const noexcept -> std::uint8_t { return static_cast<std::uint8_t>(color); }

  [[nodiscard]] constexpr auto to_uint8() const noexcept -> std::uint8_t { return static_cast<std::uint8_t>(color); }

  [[nodiscard]] constexpr auto to_rgb() const noexcept -> RGB {
    switch (color) {
    case Spectra6Color::Black:
      return colors::Black;
    case Spectra6Color::Red:
      return colors::Red;
    case Spectra6Color::Green:
      return colors::Green;
    case Spectra6Color::Blue:
      return colors::Blue;
    case Spectra6Color::Yellow:
      return colors::Yellow;
    default:
      return colors::White;
    }
  }
};

} // namespace epaper
