#pragma once

#include "epaper/drivers/capabilities.hpp"
#include "epaper/drivers/driver.hpp"
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace epaper {

/**
 * @brief Display orientation.
 *
 * Defines the rotation of the display relative to its default orientation.
 */
enum class Orientation : std::uint8_t {
  Portrait0 = 0,   ///< Default portrait (0°)
  Landscape90 = 1, ///< Clockwise 90° (landscape)
  Portrait180 = 2, ///< Upside down (180°)
  Landscape270 = 3 ///< Counter-clockwise 90° (270°)
};

/**
 * @brief Pure framebuffer management without color knowledge.
 *
 * Manages a raw byte buffer in device-specific format and handles
 * coordinate transformation for orientation. Does not perform any
 * color conversions - stores data exactly as the device expects it.
 *
 * @tparam depth Color depth of the device
 *
 * @example
 * @code{.cpp}
 * EPD27 driver{device};
 * Framebuffer<ColorDepth::Bits2> fb{driver, Orientation::Landscape90};
 * fb.clear_byte(0xFF); // Clear to all white
 * fb.set_raw_byte(10, 20, 0x00); // Set byte at position
 * @endcode
 */
template <ColorDepth depth> class Framebuffer {
public:
  /**
   * @brief Construct framebuffer for a driver.
   *
   * @param driver Display driver
   * @param orientation Display orientation
   */
  explicit Framebuffer(Driver &driver, Orientation orientation = Orientation::Portrait0)
      : driver_(&driver), physical_width_(driver.width()), physical_height_(driver.height()),
        orientation_(orientation) {
    buffer_.resize(driver.buffer_size());
  }

  /**
   * @brief Get physical buffer width (before orientation).
   *
   * @return Width in pixels
   */
  [[nodiscard]] auto physical_width() const noexcept -> std::size_t { return physical_width_; }

  /**
   * @brief Get physical buffer height (before orientation).
   *
   * @return Height in pixels
   */
  [[nodiscard]] auto physical_height() const noexcept -> std::size_t { return physical_height_; }

  /**
   * @brief Get effective width (after orientation transform).
   *
   * @return Width in pixels
   */
  [[nodiscard]] auto width() const noexcept -> std::size_t {
    return (orientation_ == Orientation::Landscape90 || orientation_ == Orientation::Landscape270) ? physical_height_
                                                                                                   : physical_width_;
  }

  /**
   * @brief Get effective height (after orientation transform).
   *
   * @return Height in pixels
   */
  [[nodiscard]] auto height() const noexcept -> std::size_t {
    return (orientation_ == Orientation::Landscape90 || orientation_ == Orientation::Landscape270) ? physical_width_
                                                                                                   : physical_height_;
  }

  /**
   * @brief Get current orientation.
   *
   * @return Orientation value
   */
  [[nodiscard]] auto orientation() const noexcept -> Orientation { return orientation_; }

  /**
   * @brief Get const view of raw buffer.
   *
   * @return Span of buffer bytes
   */
  [[nodiscard]] auto buffer() const noexcept -> std::span<const std::byte> { return buffer_; }

  /**
   * @brief Clear entire buffer to a specific byte value.
   *
   * @param value Byte value to fill buffer with
   */
  auto clear_byte(std::uint8_t value) -> void {
    for (auto &byte : buffer_) {
      byte = static_cast<std::byte>(value);
    }
  }

  /**
   * @brief Set a raw byte at physical buffer position.
   *
   * @param x X coordinate (physical)
   * @param y Y coordinate (physical)
   * @param value Byte value
   *
   * @note This operates on physical coordinates without orientation transform.
   */
  auto set_raw_byte(std::size_t x, std::size_t y, std::uint8_t value) -> void {
    if constexpr (depth == ColorDepth::Bits1) {
      // 1 bit per pixel (8 pixels per byte)
      const auto byte_index = (x / 8) + (y * ((physical_width_ + 7) / 8));
      if (byte_index < buffer_.size()) {
        buffer_[byte_index] = static_cast<std::byte>(value);
      }
    } else if constexpr (depth == ColorDepth::Bits2) {
      // 2 bits per pixel (4 pixels per byte)
      const auto byte_index = (x / 4) + (y * ((physical_width_ + 3) / 4));
      if (byte_index < buffer_.size()) {
        buffer_[byte_index] = static_cast<std::byte>(value);
      }
    } else if constexpr (depth == ColorDepth::Bits32) {
      // 32 bits per pixel (1 pixel = 4 bytes)
      const auto byte_index = (x + y * physical_width_) * 4;
      if (byte_index < buffer_.size()) {
        buffer_[byte_index] = static_cast<std::byte>(value);
      }
    }
  }

  /**
   * @brief Get raw byte at physical buffer position.
   *
   * @param x X coordinate (physical)
   * @param y Y coordinate (physical)
   * @return Byte value
   *
   * @note This operates on physical coordinates without orientation transform.
   */
  [[nodiscard]] auto get_raw_byte(std::size_t x, std::size_t y) const -> std::uint8_t {
    if constexpr (depth == ColorDepth::Bits1) {
      const auto byte_index = (x / 8) + (y * ((physical_width_ + 7) / 8));
      if (byte_index < buffer_.size()) {
        return static_cast<std::uint8_t>(buffer_[byte_index]);
      }
    } else if constexpr (depth == ColorDepth::Bits2) {
      const auto byte_index = (x / 4) + (y * ((physical_width_ + 3) / 4));
      if (byte_index < buffer_.size()) {
        return static_cast<std::uint8_t>(buffer_[byte_index]);
      }
    } else if constexpr (depth == ColorDepth::Bits32) {
      const auto byte_index = (x + y * physical_width_) * 4;
      if (byte_index < buffer_.size()) {
        return static_cast<std::uint8_t>(buffer_[byte_index]);
      }
    }
    return 0;
  }

  /**
   * @brief Transform logical coordinates to physical based on orientation.
   *
   * @param x Logical X coordinate
   * @param y Logical Y coordinate
   * @return Physical (x, y) coordinates
   */
  [[nodiscard]] auto transform_coordinates(std::size_t x, std::size_t y) const -> std::pair<std::size_t, std::size_t> {
    switch (orientation_) {
    case Orientation::Portrait0:
      return {x, y};
    case Orientation::Landscape90:
      return {physical_width_ - y - 1, x};
    case Orientation::Portrait180:
      return {physical_width_ - x - 1, physical_height_ - y - 1};
    case Orientation::Landscape270:
      return {y, physical_height_ - x - 1};
    }
    return {x, y};
  }

  /**
   * @brief Set pixel at logical coordinates (with orientation transform).
   *
   * @param x Logical X coordinate
   * @param y Logical Y coordinate
   * @param value Raw pixel value in device format
   *
   * @note The value should already be in device-specific format.
   */
  auto set_pixel(std::size_t x, std::size_t y, std::uint8_t value) -> void {
    const auto [phys_x, phys_y] = transform_coordinates(x, y);

    if constexpr (depth == ColorDepth::Bits1) {
      // 1 bit per pixel
      const auto byte_index = (phys_x / 8) + (phys_y * ((physical_width_ + 7) / 8));
      const auto bit_offset = phys_x % 8;
      if (byte_index < buffer_.size()) {
        const auto mask = static_cast<std::uint8_t>(0x80 >> bit_offset);
        auto current = static_cast<std::uint8_t>(buffer_[byte_index]);
        if (value != 0u) {
          current |= mask;
        } else {
          current &= ~mask;
        }
        buffer_[byte_index] = static_cast<std::byte>(current);
      }
    } else if constexpr (depth == ColorDepth::Bits2) {
      // 2 bits per pixel (4 pixels per byte)
      const auto byte_index = (phys_x / 4) + (phys_y * ((physical_width_ + 3) / 4));
      const auto pixel_offset = phys_x % 4;
      if (byte_index < buffer_.size()) {
        const auto shift = 6 - (pixel_offset * 2);
        const auto mask = static_cast<std::uint8_t>(0x03 << shift);
        auto current = static_cast<std::uint8_t>(buffer_[byte_index]);
        current = (current & ~mask) | ((value & 0x03) << shift);
        buffer_[byte_index] = static_cast<std::byte>(current);
      }
    } else if constexpr (depth == ColorDepth::Bits32) {
      // 32 bits per pixel
      const auto pixel_index = phys_x + (phys_y * physical_width_);
      const auto byte_index = pixel_index * 4;
      if (byte_index + 3 < buffer_.size()) {
        buffer_[byte_index] = static_cast<std::byte>(value);
      }
    }
  }

  /**
   * @brief Get pixel at logical coordinates (with orientation transform).
   *
   * @param x Logical X coordinate
   * @param y Logical Y coordinate
   * @return Raw pixel value in device format
   */
  [[nodiscard]] auto get_pixel(std::size_t x, std::size_t y) const -> std::uint8_t {
    const auto [phys_x, phys_y] = transform_coordinates(x, y);

    if constexpr (depth == ColorDepth::Bits1) {
      const auto byte_index = (phys_x / 8) + (phys_y * ((physical_width_ + 7) / 8));
      const auto bit_offset = phys_x % 8;
      if (byte_index < buffer_.size()) {
        const auto current = static_cast<std::uint8_t>(buffer_[byte_index]);
        const auto mask = static_cast<std::uint8_t>(0x80 >> bit_offset);
        return ((current & mask) != 0) ? 1 : 0;
      }
    } else if constexpr (depth == ColorDepth::Bits2) {
      const auto byte_index = (phys_x / 4) + (phys_y * ((physical_width_ + 3) / 4));
      const auto pixel_offset = phys_x % 4;
      if (byte_index < buffer_.size()) {
        const auto shift = 6 - (pixel_offset * 2);
        const auto current = static_cast<std::uint8_t>(buffer_[byte_index]);
        return (current >> shift) & 0x03;
      }
    } else if constexpr (depth == ColorDepth::Bits32) {
      const auto pixel_index = phys_x + (phys_y * physical_width_);
      const auto byte_index = pixel_index * 4;
      if (byte_index + 3 < buffer_.size()) {
        return static_cast<std::uint8_t>(buffer_[byte_index]);
      }
    }
    return 0;
  }

private:
  Driver *driver_;
  std::vector<std::byte> buffer_;
  std::size_t physical_width_;
  std::size_t physical_height_;
  Orientation orientation_;
};

} // namespace epaper
