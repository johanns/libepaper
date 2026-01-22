#pragma once

#include "epaper/core/types.hpp"
#include <cstddef>
#include <cstdint>
#include <utility>

/**
 * @file internal.hpp
 * @brief Internal utilities for libepaper implementation details.
 *
 * This header contains internal implementation utilities that are not part
 * of the public API. These functions and types are used within the library
 * implementation and may change without notice.
 *
 * **Usage Constraint:** Do not include this header directly in user code.
 * These utilities are for library internals only.
 */

namespace epaper::internal {

/// @brief Number of planes for single-plane displays (BW, Gray4, Spectra6)
inline constexpr std::size_t PLANE_COUNT_ONE = 1;

/// @brief Number of planes for dual-plane displays (BWR, BWY)
inline constexpr std::size_t PLANE_COUNT_TWO = 2;

/// @brief Number of planes for triple-plane displays (reserved for future modes)
inline constexpr std::size_t PLANE_COUNT_THREE = 3;

/**
 * @brief Type-safe enumeration for framebuffer plane counts.
 *
 * Used internally to represent the number of color planes in a framebuffer.
 * Different display modes require different numbers of planes:
 * - **One:** Monochrome modes (BlackWhite, Grayscale4, Spectra6)
 * - **Two:** Dual-plane color modes (BWR, BWY) where base + accent planes
 * - **Three:** Reserved for future multi-plane color modes
 *
 * @see MultiPlaneFramebuffer
 */
enum class PlaneCount : std::uint8_t {
  One,  ///< Single plane (monochrome or packed multi-color)
  Two,  ///< Dual plane (base + accent color)
  Three ///< Triple plane (reserved for future use)
};

/**
 * @brief Convert PlaneCount enum to numeric value.
 *
 * Provides compile-time conversion from type-safe PlaneCount to std::size_t
 * for array sizing and iteration.
 *
 * @param count The plane count to convert
 * @return Numeric plane count (1, 2, or 3)
 *
 * @note This function is constexpr and will be evaluated at compile time
 *       when passed a constant expression.
 *
 * @example
 * constexpr auto num_planes = plane_count_value(PlaneCount::Two); // 2
 */
[[nodiscard]] constexpr auto plane_count_value(PlaneCount count) noexcept -> std::size_t {
  switch (count) {
  case PlaneCount::One:
    return PLANE_COUNT_ONE;
  case PlaneCount::Two:
    return PLANE_COUNT_TWO;
  case PlaneCount::Three:
    return PLANE_COUNT_THREE;
  }
  std::unreachable();
}

/**
 * @brief Transform logical coordinates to physical display coordinates.
 *
 * Applies orientation-based coordinate transformation to map logical
 * (x, y) positions to physical display addresses. This enables rotation
 * without modifying the framebuffer layout in memory.
 *
 * **Transformation Matrix:**
 * - **Portrait0:** Identity (x, y) → (x, y)
 * - **Landscape90:** 90° CW rotation (x, y) → (width-1-y, x)
 * - **Portrait180:** 180° rotation (x, y) → (width-1-x, height-1-y)
 * - **Landscape270:** 270° CW / 90° CCW (x, y) → (y, height-1-x)
 *
 * **Algorithm:**
 * Each orientation applies an affine transformation that preserves
 * the rectangular grid structure while rotating the coordinate frame.
 * Width and height parameters represent the *untransformed* display
 * dimensions (physical dimensions, not logical dimensions after rotation).
 *
 * @param x Logical x-coordinate (0 to width-1 for Portrait, 0 to height-1 for Landscape)
 * @param y Logical y-coordinate (0 to height-1 for Portrait, 0 to width-1 for Landscape)
 * @param width Physical display width in pixels (untransformed)
 * @param height Physical display height in pixels (untransformed)
 * @param orientation Desired display orientation
 * @return Physical (x', y') coordinates in display memory
 *
 * @note The default case returns (x, y) for compatibility, though all
 *       Orientation enum values are explicitly handled.
 *
 * @example
 * // 176x264 display rotated 90° clockwise
 * auto [px, py] = transform_coordinates(0, 0, 176, 264, Orientation::Landscape90);
 * // Returns (175, 0) - top-left becomes top-right
 *
 * @see Orientation
 */
inline auto transform_coordinates(std::size_t x, std::size_t y, std::size_t width, std::size_t height,
                                  Orientation orientation) -> std::pair<std::size_t, std::size_t> {
  switch (orientation) {
  case Orientation::Portrait0:
    // Identity transformation - no rotation applied
    // Logical coordinate system matches physical display layout
    return {x, y};

  case Orientation::Landscape90:
    // 90° clockwise rotation:
    // - Original top-left (0,0) → new top-right (width-1, 0)
    // - Original top-right (width-1, 0) → new bottom-right (width-1, height-1)
    // Transformation matrix: [x'] = [0  -1] [x] + [width-1]
    //                        [y']   [1   0] [y]   [0]
    // Simplified: x' = width-1-y, y' = x
    return {width - 1 - y, x};

  case Orientation::Portrait180:
    // 180° rotation:
    // - Original top-left (0,0) → new bottom-right (width-1, height-1)
    // - Original center stays at center
    // Transformation matrix: [x'] = [-1  0] [x] + [width-1]
    //                        [y']   [0  -1] [y]   [height-1]
    // Simplified: x' = width-1-x, y' = height-1-y
    return {width - 1 - x, height - 1 - y};

  case Orientation::Landscape270:
    // 270° clockwise (90° counter-clockwise) rotation:
    // - Original top-left (0,0) → new bottom-left (0, height-1)
    // - Original bottom-right → new top-right
    // Transformation matrix: [x'] = [0   1] [x] + [0]
    //                        [y']   [-1  0] [y]   [height-1]
    // Simplified: x' = y, y' = height-1-x
    return {y, height - 1 - x};

  default:
    // Fallback for any unexpected orientation values
    // Returns identity transformation for safety
    return {x, y};
  }
}
} // namespace epaper::internal
