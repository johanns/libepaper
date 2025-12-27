#pragma once

#include "epaper/drivers/driver.hpp"
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace epaper {

// Color representation for e-paper displays
enum class Color : std::uint8_t {
  White = 0xFF, // White (or lightest gray)
  Black = 0x00, // Black (or darkest gray)
  Gray1 = 0x80, // First gray level (lighter)
  Gray2 = 0x40  // Second gray level (darker)
};

// Display orientation
enum class Orientation : std::uint8_t {
  Portrait0 = 0,   // Default portrait (0°)
  Landscape90 = 1, // Clockwise 90° (landscape)
  Portrait180 = 2, // Upside down (180°)
  Landscape270 = 3 // Counter-clockwise 90° (270°)
};

// Screen framebuffer management
class Screen {
public:
  explicit Screen(Driver &driver, Orientation orientation = Orientation::Portrait0, bool auto_sleep = true);

  // Get screen dimensions (physical buffer dimensions)
  [[nodiscard]] auto width() const noexcept -> std::size_t { return width_; }
  [[nodiscard]] auto height() const noexcept -> std::size_t { return height_; }
  [[nodiscard]] auto mode() const noexcept -> DisplayMode { return mode_; }

  // Get effective dimensions (accounting for rotation)
  [[nodiscard]] auto effective_width() const noexcept -> std::size_t;
  [[nodiscard]] auto effective_height() const noexcept -> std::size_t;

  // Get current orientation
  [[nodiscard]] auto orientation() const noexcept -> Orientation { return orientation_; }

  // Auto-sleep configuration (prevents screen burn-in per WaveShare recommendation)
  [[nodiscard]] auto auto_sleep_enabled() const noexcept -> bool { return auto_sleep_enabled_; }
  auto set_auto_sleep(bool enabled) noexcept -> void { auto_sleep_enabled_ = enabled; }

  // Pixel operations with bounds checking
  auto set_pixel(std::size_t x, std::size_t y, Color color) -> void;
  [[nodiscard]] auto get_pixel(std::size_t x, std::size_t y) const -> Color;

  // Clear entire screen to specified color
  auto clear(Color color = Color::White) -> void;

  // Clear a rectangular region
  auto clear_region(std::size_t x_start, std::size_t y_start, std::size_t x_end, std::size_t y_end, Color color)
      -> void;

  // Send buffer to display and refresh (automatically sleeps if auto_sleep_enabled)
  auto refresh() -> void;

  // Direct buffer access (const)
  [[nodiscard]] auto buffer() const -> std::span<const std::byte> { return buffer_; }

  // Set display mode (requires re-initialization)
  auto set_mode(DisplayMode mode) -> void;

private:
  // Helper: transform coordinates based on orientation
  [[nodiscard]] auto transform_coordinates(std::size_t x, std::size_t y) const -> std::pair<std::size_t, std::size_t>;

  // Helper: calculate bit position in buffer for B/W mode
  [[nodiscard]] auto calculate_bw_position(std::size_t x, std::size_t y) const -> std::pair<std::size_t, std::uint8_t>;

  // Helper: calculate byte position in buffer for grayscale mode
  [[nodiscard]] auto calculate_gray_position(std::size_t x, std::size_t y) const
      -> std::pair<std::size_t, std::uint8_t>;

  Driver &driver_;
  std::vector<std::byte> buffer_;
  std::size_t width_;
  std::size_t height_;
  DisplayMode mode_;
  Orientation orientation_;
  bool auto_sleep_enabled_;
};

} // namespace epaper
