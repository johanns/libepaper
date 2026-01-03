#pragma once

#include <cstddef>
#include <cstdint>

namespace epaper {

/**
 * @brief Color depth enumeration for display capabilities.
 *
 * Defines the bit depth per pixel that a display can support.
 */
enum class ColorDepth : std::uint8_t {
  Bits1 = 1,   ///< 1-bit (black and white)
  Bits2 = 2,   ///< 2-bit (4-level grayscale)
  Bits4 = 4,   ///< 4-bit (16-level grayscale)
  Bits8 = 8,   ///< 8-bit (256 colors)
  Bits16 = 16, ///< 16-bit color
  Bits24 = 24, ///< 24-bit true color
  Bits32 = 32  ///< 32-bit true color with alpha
};

/**
 * @brief Driver capabilities trait template.
 *
 * Provides compile-time information about driver capabilities.
 * Specialize this template for each driver implementation.
 *
 * @tparam Driver The driver type to query capabilities for
 */
template <typename Driver> struct driver_capabilities {
  static constexpr ColorDepth color_depth = ColorDepth::Bits1;
  static constexpr bool supports_grayscale = false;
  static constexpr bool supports_partial_refresh = false;
  static constexpr bool supports_power_control = false;
  static constexpr bool supports_wake_from_sleep = false;
  static constexpr std::size_t max_width = 0;
  static constexpr std::size_t max_height = 0;
};

/**
 * @brief Concept to check if a type is a valid driver with capabilities.
 *
 * @tparam T Type to check
 */
template <typename T>
concept DriverWithCapabilities = requires {
  requires std::convertible_to<decltype(driver_capabilities<T>::color_depth), ColorDepth>;
  requires std::convertible_to<decltype(driver_capabilities<T>::supports_grayscale), bool>;
  requires std::convertible_to<decltype(driver_capabilities<T>::supports_partial_refresh), bool>;
  requires std::convertible_to<decltype(driver_capabilities<T>::supports_power_control), bool>;
  requires std::convertible_to<decltype(driver_capabilities<T>::supports_wake_from_sleep), bool>;
  requires std::convertible_to<decltype(driver_capabilities<T>::max_width), std::size_t>;
  requires std::convertible_to<decltype(driver_capabilities<T>::max_height), std::size_t>;
};

} // namespace epaper
