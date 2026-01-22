#pragma once

#include "epaper/core/types.hpp"
#include <concepts>
#include <cstddef>
#include <span>
#include <vector>

namespace epaper {

// Forward declarations
enum class DisplayMode : std::uint8_t;
enum class Color : std::uint8_t;
enum class Orientation : std::uint8_t;

/**
 * @brief Concept defining the interface for framebuffer implementations.
 *
 * This concept replaces the previous `Framebuffer` abstract base class,
 * enabling compile-time polymorphism via C++20/23 concepts. Eliminates
 * virtual function overhead while maintaining type safety and interface
 * enforcement.
 *
 * **Requirements:**
 * - Dimension queries: width(), height()
 * - Mode query: mode() returns associated DisplayMode
 * - Buffer access: data() and get_planes() for raw byte access
 * - Pixel operations: set_pixel(), get_pixel() with orientation support
 * - Bulk operations: clear()
 * - Mode compatibility: static supports_mode() for validation
 *
 * **Implementation Notes:**
 * - Coordinate transforms (orientation) are framebuffer's responsibility
 * - Pixel encoding (color to bit pattern) is mode-specific
 * - Buffer layout is implementation-defined (row-major typical)
 *
 * @tparam T Type to check for framebuffer interface compliance
 *
 * @example
 * ```cpp
 * // Verify framebuffer types satisfy concept
 * static_assert(FramebufferLike<MonoFramebuffer>);
 * static_assert(FramebufferLike<MultiPlaneFramebuffer<internal::PlaneCount::Two>>);
 *
 * // Use in generic template function
 * template <FramebufferLike FB>
 * auto draw_pattern(FB& fb) -> void {
 *   for (std::size_t y = 0; y < fb.height(); ++y) {
 *     for (std::size_t x = 0; x < fb.width(); ++x) {
 *       Color c = ((x + y) % 2) ? Color::Black : Color::White;
 *       fb.set_pixel(x, y, c, Orientation::Portrait0);
 *     }
 *   }
 * }
 * ```
 *
 * @see MonoFramebuffer, MultiPlaneFramebuffer, Display
 */
template <typename T>
concept FramebufferLike =
    requires(T fb, const T const_fb, std::size_t x, std::size_t y, Color color, Orientation orientation) {
      /**
       * @brief Get framebuffer width in pixels.
       * @return Width in pixels
       */
      { fb.width() } -> std::same_as<std::size_t>;

      /**
       * @brief Get framebuffer height in pixels.
       * @return Height in pixels
       */
      { fb.height() } -> std::same_as<std::size_t>;

      /**
       * @brief Get display mode this framebuffer supports.
       * @return DisplayMode
       */
      { fb.mode() } -> std::same_as<DisplayMode>;

      /**
       * @brief Get raw buffer data as byte span.
       * @return Span of const bytes
       */
      { const_fb.data() } -> std::same_as<std::span<const std::byte>>;

      /**
       * @brief Get all color planes (for multi-plane modes).
       * @return Vector of byte spans, one per plane
       */
      { const_fb.get_planes() } -> std::same_as<std::vector<std::span<const std::byte>>>;

      /**
       * @brief Set pixel at coordinates with orientation transform.
       * @param x X coordinate (logical)
       * @param y Y coordinate (logical)
       * @param color Color to set
       * @param orientation Display orientation for coordinate transform
       */
      { fb.set_pixel(x, y, color, orientation) } -> std::same_as<void>;

      /**
       * @brief Get pixel color at coordinates with orientation transform.
       * @param x X coordinate (logical)
       * @param y Y coordinate (logical)
       * @param orientation Display orientation for coordinate transform
       * @return Color at position
       */
      { const_fb.get_pixel(x, y, orientation) } -> std::same_as<Color>;

      /**
       * @brief Clear entire framebuffer to a color.
       * @param color Fill color
       */
      { fb.clear(color) } -> std::same_as<void>;
    };

} // namespace epaper
