#pragma once

#include "epaper/drivers/capabilities.hpp"
#include <cstdint>

namespace epaper {

/**
 * @brief Device-specific color representation.
 *
 * Stores color in the format required by the target device.
 * The actual representation depends on the device's color depth.
 *
 * @tparam depth Color depth of the device
 */
template <ColorDepth depth> struct DeviceColor {
  std::uint8_t value; ///< Raw color value

  constexpr explicit DeviceColor(std::uint8_t val = 0) : value(val) {}

  constexpr auto operator==(const DeviceColor &other) const noexcept -> bool = default;
};

/**
 * @brief Specialization for 1-bit (black and white) devices.
 */
template <> struct DeviceColor<ColorDepth::Bits1> {
  bool is_white; ///< true = white, false = black

  constexpr explicit DeviceColor(bool white = true) : is_white(white) {}

  constexpr auto operator==(const DeviceColor &other) const noexcept -> bool = default;

  [[nodiscard]] constexpr auto to_byte() const noexcept -> std::uint8_t { return is_white ? 0xFF : 0x00; }
};

/**
 * @brief Specialization for 2-bit (4-level grayscale) devices.
 */
template <> struct DeviceColor<ColorDepth::Bits2> {
  std::uint8_t level; ///< Grayscale level (0-3, where 3 is white)

  constexpr explicit DeviceColor(std::uint8_t gray_level = 3) : level(gray_level & 0x03) {}

  constexpr auto operator==(const DeviceColor &other) const noexcept -> bool = default;

  [[nodiscard]] constexpr auto to_byte() const noexcept -> std::uint8_t {
    // Map 2-bit level to 8-bit value
    // 0 -> 0x00 (black), 1 -> 0x40, 2 -> 0x80, 3 -> 0xC0 (white)
    return static_cast<std::uint8_t>(level << 6);
  }
};

/**
 * @brief Specialization for 32-bit (true color with alpha) devices.
 */
template <> struct DeviceColor<ColorDepth::Bits32> {
  std::uint8_t a; ///< Alpha component
  std::uint8_t b; ///< Blue component
  std::uint8_t g; ///< Green component
  std::uint8_t r; ///< Red component

  constexpr DeviceColor(std::uint8_t red = 0, std::uint8_t green = 0, std::uint8_t blue = 0, std::uint8_t alpha = 255)
      : a(alpha), b(blue), g(green), r(red) {}

  constexpr auto operator==(const DeviceColor &other) const noexcept -> bool = default;
};

} // namespace epaper
