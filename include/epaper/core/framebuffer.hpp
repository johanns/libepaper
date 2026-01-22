#pragma once

#include "epaper/core/framebuffer_concepts.hpp"
#include "epaper/core/types.hpp"
#include "epaper/drivers/driver.hpp"
#include "epaper/internal/internal.hpp"
#include <array>
#include <cstddef>
#include <span>
#include <vector>

namespace epaper {

/**
 * @brief Single-plane framebuffer supporting mono, grayscale, or packed color.
 *
 * Supports display modes that use a single plane (e.g., BlackWhite, Grayscale4,
 * Spectra6). The concrete encoding is selected by the runtime `DisplayMode`.
 *
 * **Buffer Layout by Mode:**
 * - BlackWhite (1 bpp): 8 pixels per byte, MSB first, 1=white, 0=black
 * - Grayscale4 (2 bpp): 4 pixels per byte, 2 bits each (00=black, 11=white)
 * - Spectra6 (3 bpp): Packed 3-bit values, custom color mapping
 *
 * **Memory Efficiency:**
 * - Row-major layout with stride alignment
 * - Stride may exceed width/8 for byte alignment
 * - Total buffer size = stride * height
 *
 * @note Pixel encoding and bit ordering are hardware-specific.
 *       Coordinate transforms handle orientation automatically.
 *
 * @example
 * ```cpp
 * // Create mono framebuffer for 2.7" display
 * MonoFramebuffer fb{176, 264, DisplayMode::BlackWhite};
 *
 * // Set pixels (orientation transforms applied internally)
 * fb.set_pixel(0, 0, Color::Black, Orientation::Portrait0);
 *
 * // Clear to white
 * fb.clear(Color::White);
 *
 * // Access raw buffer for driver transfer
 * auto buffer = fb.data();  // std::span<const std::byte>
 * driver.display(buffer);
 * ```
 *
 * @see DisplayMode, FramebufferLike, MultiPlaneFramebuffer
 */
class MonoFramebuffer {
public:
  /**
   * @brief Construct a mono framebuffer.
   *
   * @param width Framebuffer width in pixels
   * @param height Framebuffer height in pixels
   * @param mode Display mode (must be single-plane)
   */
  MonoFramebuffer(std::size_t width, std::size_t height, DisplayMode mode);

  /** @brief Get framebuffer width in pixels. */
  [[nodiscard]] auto width() const -> std::size_t;
  /** @brief Get framebuffer height in pixels. */
  [[nodiscard]] auto height() const -> std::size_t;
  /** @brief Get display mode represented by this framebuffer. */
  [[nodiscard]] auto mode() const -> DisplayMode;
  /** @brief Get raw buffer as a byte span. */
  [[nodiscard]] auto data() const -> std::span<const std::byte>;
  /** @brief Get plane spans (single plane for mono). */
  [[nodiscard]] auto get_planes() const -> std::vector<std::span<const std::byte>>;
  /**
   * @brief Set a pixel with orientation transform.
   * @param x X coordinate (logical)
   * @param y Y coordinate (logical)
   * @param color Color to set
   * @param orientation Display orientation
   */
  auto set_pixel(std::size_t x, std::size_t y, Color color, Orientation orientation) -> void;
  /**
   * @brief Get a pixel color with orientation transform.
   * @param x X coordinate (logical)
   * @param y Y coordinate (logical)
   * @param orientation Display orientation
   * @return Color at position
   */
  [[nodiscard]] auto get_pixel(std::size_t x, std::size_t y, Orientation orientation) const -> Color;
  /**
   * @brief Clear the framebuffer to a color.
   * @param color Fill color
   */
  auto clear(Color color) -> void;

  /**
   * @brief Check if the mode is compatible with mono framebuffer.
   * @param mode Display mode
   * @return true if the mode is single-plane
   */
  [[nodiscard]] static constexpr auto supports_mode(DisplayMode mode) noexcept -> bool {
    return num_planes(mode) == internal::PLANE_COUNT_ONE;
  }

private:
  std::size_t width_;
  std::size_t height_;
  std::size_t stride_{0};
  DisplayMode mode_;
  std::vector<std::byte> buffer_;
};

/**
 * @brief Multi-plane framebuffer for color modes.
 *
 * Plane count is a compile-time constant for strict type-level guarantees.
 * Used for tri-color displays (BWR, BWY) which require separate buffers
 * for base (black/white) and accent (red/yellow) colors.
 *
 * **Plane Organization:**
 * - Plane 0: Black/White data (1 bpp)
 * - Plane 1: Red/Yellow data (1 bpp)
 * - Pixel color determined by combination of both planes
 *
 * **Color Encoding (BWR example):**
 * - (0, 0) → White
 * - (1, 0) → Black
 * - (0, 1) → Red
 * - (1, 1) → Undefined (typically avoided)
 *
 * **Type Safety:**
 * - Template parameter enforces correct plane count at compile time
 * - supports_mode() validates mode/plane count compatibility
 *
 * @tparam PlaneCount Number of planes (internal::PlaneCount enum)
 *
 * @example
 * ```cpp
 * // Create two-plane framebuffer for BWR display
 * MultiPlaneFramebuffer<internal::PlaneCount::Two> fb{176, 264, DisplayMode::BWR};
 *
 * // Set pixels (automatically distributed across planes)
 * fb.set_pixel(10, 20, Color::Red, Orientation::Portrait0);
 * fb.set_pixel(30, 40, Color::Black, Orientation::Portrait0);
 *
 * // Access planes for driver transfer
 * auto planes = fb.get_planes();  // std::vector<std::span<const std::byte>>
 * driver.display_planes(planes);  // Driver handles multi-plane protocol
 * ```
 *
 * @see MonoFramebuffer, DisplayMode::BWR, DisplayMode::BWY, TwoPlaneFramebuffer
 */
template <internal::PlaneCount PlaneCount> class MultiPlaneFramebuffer {
public:
  /**
   * @brief Construct a multi-plane framebuffer.
   *
   * @param width Framebuffer width in pixels
   * @param height Framebuffer height in pixels
   * @param mode Display mode (must match plane count)
   */
  MultiPlaneFramebuffer(std::size_t width, std::size_t height, DisplayMode mode);

  /** @brief Get framebuffer width in pixels. */
  [[nodiscard]] auto width() const -> std::size_t;
  /** @brief Get framebuffer height in pixels. */
  [[nodiscard]] auto height() const -> std::size_t;
  /** @brief Get display mode represented by this framebuffer. */
  [[nodiscard]] auto mode() const -> DisplayMode;
  /** @brief Get raw buffer as a byte span (first plane). */
  [[nodiscard]] auto data() const -> std::span<const std::byte>;
  /** @brief Get plane spans (one per plane). */
  [[nodiscard]] auto get_planes() const -> std::vector<std::span<const std::byte>>;
  /**
   * @brief Set a pixel with orientation transform.
   * @param x X coordinate (logical)
   * @param y Y coordinate (logical)
   * @param color Color to set
   * @param orientation Display orientation
   */
  auto set_pixel(std::size_t x, std::size_t y, Color color, Orientation orientation) -> void;
  /**
   * @brief Get a pixel color with orientation transform.
   * @param x X coordinate (logical)
   * @param y Y coordinate (logical)
   * @param orientation Display orientation
   * @return Color at position
   */
  [[nodiscard]] auto get_pixel(std::size_t x, std::size_t y, Orientation orientation) const -> Color;
  /**
   * @brief Clear the framebuffer to a color.
   * @param color Fill color
   */
  auto clear(Color color) -> void;

  /**
   * @brief Check if the mode is compatible with this plane count.
   * @param mode Display mode
   * @return true if the plane count matches
   */
  [[nodiscard]] static constexpr auto supports_mode(DisplayMode mode) noexcept -> bool {
    return num_planes(mode) == internal::plane_count_value(PlaneCount);
  }

private:
  static constexpr std::size_t PLANE_COUNT = internal::plane_count_value(PlaneCount);
  std::size_t width_;
  std::size_t height_;
  std::size_t stride_;
  DisplayMode mode_;
  std::array<std::vector<std::byte>, PLANE_COUNT> planes_;
};

/** @brief Convenience alias for a two-plane color framebuffer. */
using TwoPlaneFramebuffer = MultiPlaneFramebuffer<internal::PlaneCount::Two>;

static_assert(FramebufferLike<MonoFramebuffer>);
static_assert(FramebufferLike<TwoPlaneFramebuffer>);

} // namespace epaper
