#pragma once

/**
 * @file capabilities.hpp
 * @brief Compile-time driver capability traits system.
 *
 * Provides template specialization-based mechanism for querying driver
 * capabilities at compile time. Used for static validation, optimization,
 * and feature detection.
 *
 * **Design Pattern:**
 * ```
 * Trait Class Pattern (Compile-Time Introspection)
 * ├─ Specialization: Each driver specializes driver_traits<Driver>
 * ├─ Constexpr Fields: All capabilities are compile-time constants
 * ├─ Zero Runtime Cost: All queries resolved at compile time
 * └─ Validation: DriverTraits concept enforces complete specialization
 * ```
 *
 * **Capabilities Provided:**
 * - `max_mode`: Highest DisplayMode supported (e.g., DisplayMode::BWR)
 * - `supports_grayscale`: Can render 4-level grayscale
 * - `supports_partial_refresh`: Can update regions without full refresh
 * - `supports_power_control`: Can enter/exit sleep modes
 * - `supports_wake_from_sleep`: Can wake without full re-init
 * - `max_width`, `max_height`: Physical display dimensions
 *
 * **Specialization Template:**
 * ```cpp
 * template <>
 * struct driver_traits<MyDriver> {
 *   static constexpr DisplayMode max_mode = DisplayMode::BWR;
 *   static constexpr bool supports_grayscale = true;
 *   static constexpr bool supports_partial_refresh = false;
 *   static constexpr bool supports_power_control = true;
 *   static constexpr bool supports_wake_from_sleep = true;
 *   static constexpr std::size_t max_width = 264;
 *   static constexpr std::size_t max_height = 176;
 * };
 * ```
 *
 * **Usage Patterns:**
 * ```cpp
 * // Compile-time capability check
 * if constexpr (driver_traits<EPD27>::supports_grayscale) {
 *   // Use Grayscale4 mode
 * }
 *
 * // Static assertion for required features
 * static_assert(driver_traits<MyDriver>::supports_power_control,
 *               "This application requires sleep mode support");
 *
 * // Mode validation
 * bool can_use_bwr = supports_display_mode<EPD27>(DisplayMode::BWR);
 * ```
 *
 * **Mode Support Logic:**
 * - BlackWhite: Always supported (baseline)
 * - Grayscale4: Requires `supports_grayscale == true`
 * - Color modes (BWR/BWY/Spectra6): Requires exact match with `max_mode`
 * - Rationale: Color modes are hardware-specific, not hierarchical
 *
 * @see Driver, DisplayMode, supports_display_mode()
 */

#include "epaper/drivers/driver.hpp"
#include <concepts>
#include <cstddef>

namespace epaper {

/**
 * @brief Driver capabilities trait template.
 *
 * Provides compile-time information about driver capabilities.
 * **Must be specialized for each driver implementation.**
 *
 * **Required Fields (all must be present in specialization):**
 * - `max_mode`: Highest DisplayMode supported
 * - `supports_grayscale`: true if Grayscale4 mode available
 * - `supports_partial_refresh`: true if partial updates supported
 * - `supports_power_control`: true if sleep()/wake() implemented
 * - `supports_wake_from_sleep`: true if wake() restores state
 * - `max_width`: Physical display width in pixels
 * - `max_height`: Physical display height in pixels
 *
 * **Specialization Example:**
 * ```cpp
 * // For EPD 2.7" BWR display
 * template <>
 * struct driver_traits<EPD27> {
 *   static constexpr DisplayMode max_mode = DisplayMode::BWR;
 *   static constexpr bool supports_grayscale = false;
 *   static constexpr bool supports_partial_refresh = false;
 *   static constexpr bool supports_power_control = true;
 *   static constexpr bool supports_wake_from_sleep = true;
 *   static constexpr std::size_t max_width = 264;
 *   static constexpr std::size_t max_height = 176;
 * };
 * ```
 *
 * @tparam Driver The driver type to query capabilities for
 *
 * @note Base template is intentionally incomplete to force specialization.
 *       Attempting to use driver_traits<T> without specialization will fail
 *       DriverTraits concept check.
 *
 * @see DriverTraits, supports_display_mode()
 */
template <typename Driver> struct driver_traits;
// Base template is empty/undefined to force specialization
// Or we can leave it undefined so it fails instantiation if not specialized.

/**
 * @brief Concept to check if a type is a valid driver with capabilities.
 *
 * @tparam T Type to check
 */
template <typename T>
concept DriverTraits = requires {
  requires std::convertible_to<decltype(driver_traits<T>::max_mode), DisplayMode>;
  requires std::convertible_to<decltype(driver_traits<T>::supports_grayscale), bool>;
  requires std::convertible_to<decltype(driver_traits<T>::supports_partial_refresh), bool>;
  requires std::convertible_to<decltype(driver_traits<T>::supports_power_control), bool>;
  requires std::convertible_to<decltype(driver_traits<T>::supports_wake_from_sleep), bool>;
  requires std::convertible_to<decltype(driver_traits<T>::max_width), std::size_t>;
  requires std::convertible_to<decltype(driver_traits<T>::max_height), std::size_t>;
};

/**
 * @brief Check if a driver supports a specific display mode at compile-time.
 *
 * Uses driver_traits specialization to determine mode compatibility.
 *
 * **Support Logic:**
 * - **BlackWhite**: Always supported (baseline mode for all e-paper)
 * - **Grayscale4**: Requires `driver_traits<Driver>::supports_grayscale == true`
 * - **BWR/BWY/Spectra6**: Requires exact match with `driver_traits<Driver>::max_mode`
 *
 * **Why Exact Match for Color Modes?**
 * Color modes are hardware-specific and mutually exclusive:
 * - BWR (Red) ≠ BWY (Yellow) - different ink formulations
 * - Spectra6 ≠ BWR - incompatible command sets
 * Using `<=` comparison would falsely suggest BWR(2) < Spectra6(4) implies compatibility.
 *
 * **Rationale:**
 * - Prevents runtime mode validation errors
 * - Enables compile-time specialization (e.g., `if constexpr`)
 * - Clear error messages at template instantiation
 *
 * @tparam Driver Driver type to query (must satisfy DriverTraits)
 * @param mode Display mode to check
 * @return true if driver supports the mode, false otherwise
 *
 * @example
 * ```cpp
 * // Compile-time mode validation
 * if constexpr (supports_display_mode<EPD27>(DisplayMode::BWR)) {
 *   auto display = create_display<EPD27>(device, DisplayMode::BWR);
 * } else {
 *   auto display = create_display<EPD27>(device, DisplayMode::BlackWhite);
 * }
 *
 * // Static assertion for required mode
 * static_assert(supports_display_mode<MyDriver>(DisplayMode::Grayscale4),
 *               "This app requires grayscale support");
 * ```
 *
 * @see driver_traits, DisplayMode, create_display()
 */
template <typename Driver>
  requires DriverTraits<Driver>
[[nodiscard]] constexpr auto supports_display_mode(DisplayMode mode) noexcept -> bool {
  // Always support single-bit BW
  if (mode == DisplayMode::BlackWhite) {
    return true;
  }

  // Grayscale support is explicit
  if (mode == DisplayMode::Grayscale4) {
    return driver_traits<Driver>::supports_grayscale;
  }

  // For color modes (BWR, BWY, Spectra6), strict matching is required.
  // Inequality (<=) is unsafe because modes like BWR(2) and BWY(3) are mutually exclusive,
  // not hierarchical. A Spectra6(5) driver might not support legacy BWR(2) commands.
  return mode == driver_traits<Driver>::max_mode;
}

} // namespace epaper
