#pragma once

#include "epaper/device.hpp"
#include "epaper/drivers/driver.hpp"
#include "epaper/font.hpp"
#include <cstddef>
#include <cstdint>
#include <expected>
#include <memory>
#include <span>
#include <string_view>
#include <vector>

namespace epaper {

/**
 * @brief Color representation for e-paper displays.
 *
 * Defines the color palette available for e-paper displays, including
 * black/white and grayscale levels.
 */
enum class Color : std::uint8_t {
  White = 0xFF, ///< White (or lightest gray)
  Black = 0x00, ///< Black (or darkest gray)
  Gray1 = 0x80, ///< First gray level (lighter)
  Gray2 = 0x40  ///< Second gray level (darker)
};

/**
 * @brief Display orientation modes.
 *
 * Defines the rotation orientation of the display. The display can be
 * rotated in 90-degree increments.
 */
enum class Orientation : std::uint8_t {
  Portrait0 = 0,   ///< Default portrait (0°)
  Landscape90 = 1, ///< Clockwise 90° (landscape)
  Portrait180 = 2, ///< Upside down (180°)
  Landscape270 = 3 ///< Counter-clockwise 90° (270°)
};

/**
 * @brief Bitmap error types.
 *
 * Error codes for bitmap loading and rendering operations.
 */
enum class BitmapError {
  FileNotFound,     ///< Bitmap file not found
  InvalidFormat,    ///< Invalid or unsupported bitmap format
  LoadFailed,       ///< Failed to load bitmap data
  InvalidDimensions ///< Invalid bitmap dimensions
};

/**
 * @brief Drawing styles for points and lines.
 */
enum class DotPixel : std::uint8_t {
  Pixel1x1 = 1, ///< 1x1 pixel
  Pixel2x2 = 2, ///< 2x2 pixel
  Pixel3x3 = 3, ///< 3x3 pixel
  Pixel4x4 = 4, ///< 4x4 pixel
  Pixel5x5 = 5, ///< 5x5 pixel
  Pixel6x6 = 6, ///< 6x6 pixel
  Pixel7x7 = 7, ///< 7x7 pixel
  Pixel8x8 = 8  ///< 8x8 pixel
};

/**
 * @brief Line drawing styles.
 */
enum class LineStyle {
  Solid, ///< Solid line
  Dotted ///< Dotted line
};

/**
 * @brief Fill modes for shapes.
 */
enum class DrawFill {
  Empty, ///< Outline only
  Full   ///< Filled shape
};

/**
 * @brief Unified display interface for e-paper displays.
 *
 * The Display class provides a single, unified API for e-paper display
 * operations, combining framebuffer management and drawing operations.
 * It owns the underlying driver and manages the display lifecycle.
 *
 * This class follows the GDI/UIKit pattern where a single context object
 * provides all drawing operations. It simplifies the API by eliminating
 * the need for separate Screen and Draw objects.
 *
 * @note The display automatically enters sleep mode after refresh if
 *       auto-sleep is enabled (default).
 *
 * @example
 * @code{.cpp}
 * epaper::Device device;
 * device.init();
 *
 * auto display = epaper::create_display<epaper::EPD27>(
 *     device, epaper::DisplayMode::BlackWhite, epaper::Orientation::Landscape90);
 *
 * if (display) {
 *     display->clear();
 *     display->draw_line(10, 10, 100, 100, epaper::Color::Black);
 *     display->draw_string(10, 120, "Hello World", epaper::Font::font16(),
 *                         epaper::Color::Black, epaper::Color::White);
 *     display->refresh();
 * }
 * @endcode
 */
class Display {
public:
  /**
   * @brief Constructs a Display with an existing driver.
   *
   * @param driver Unique pointer to the initialized driver
   * @param orientation Display orientation
   * @param auto_sleep Enable automatic sleep after refresh
   *
   * @note The driver must already be initialized before passing to Display.
   */
  explicit Display(std::unique_ptr<Driver> driver, Orientation orientation = Orientation::Portrait0,
                   bool auto_sleep = true);

  // Non-copyable, movable
  Display(const Display &) = delete;
  Display &operator=(const Display &) = delete;
  Display(Display &&) noexcept = default;
  Display &operator=(Display &&) noexcept = default;

  ~Display() = default;

  // ========== Display Properties ==========

  /**
   * @brief Get physical display width in pixels.
   *
   * @return Physical width (not affected by rotation)
   */
  [[nodiscard]] auto width() const noexcept -> std::size_t { return width_; }

  /**
   * @brief Get physical display height in pixels.
   *
   * @return Physical height (not affected by rotation)
   */
  [[nodiscard]] auto height() const noexcept -> std::size_t { return height_; }

  /**
   * @brief Get effective display width accounting for rotation.
   *
   * @return Effective width after applying orientation
   */
  [[nodiscard]] auto effective_width() const noexcept -> std::size_t;

  /**
   * @brief Get effective display height accounting for rotation.
   *
   * @return Effective height after applying orientation
   */
  [[nodiscard]] auto effective_height() const noexcept -> std::size_t;

  /**
   * @brief Get current display mode.
   *
   * @return Current display mode (BlackWhite or Grayscale4)
   */
  [[nodiscard]] auto mode() const noexcept -> DisplayMode { return mode_; }

  /**
   * @brief Get current orientation.
   *
   * @return Current display orientation
   */
  [[nodiscard]] auto orientation() const noexcept -> Orientation { return orientation_; }

  /**
   * @brief Check if auto-sleep is enabled.
   *
   * @return true if auto-sleep is enabled
   */
  [[nodiscard]] auto auto_sleep_enabled() const noexcept -> bool { return auto_sleep_enabled_; }

  /**
   * @brief Enable or disable auto-sleep mode.
   *
   * When enabled, the display automatically enters sleep mode after refresh
   * to prevent screen burn-in (recommended by manufacturer).
   *
   * @param enabled true to enable auto-sleep
   */
  auto set_auto_sleep(bool enabled) noexcept -> void { auto_sleep_enabled_ = enabled; }

  // ========== Framebuffer Operations ==========

  /**
   * @brief Set a single pixel to the specified color.
   *
   * Coordinates are in logical space (accounting for orientation).
   * Out-of-bounds coordinates are silently ignored.
   *
   * @param x X coordinate (0 to effective_width-1)
   * @param y Y coordinate (0 to effective_height-1)
   * @param color Pixel color
   */
  auto set_pixel(std::size_t x, std::size_t y, Color color) -> void;

  /**
   * @brief Get the color of a pixel.
   *
   * Coordinates are in logical space (accounting for orientation).
   * Out-of-bounds coordinates return White.
   *
   * @param x X coordinate (0 to effective_width-1)
   * @param y Y coordinate (0 to effective_height-1)
   * @return Pixel color
   */
  [[nodiscard]] auto get_pixel(std::size_t x, std::size_t y) const -> Color;

  /**
   * @brief Clear entire display to specified color.
   *
   * @param color Fill color (default: White)
   */
  auto clear(Color color = Color::White) -> void;

  /**
   * @brief Clear a rectangular region.
   *
   * @param x_start Starting X coordinate
   * @param y_start Starting Y coordinate
   * @param x_end Ending X coordinate
   * @param y_end Ending Y coordinate
   * @param color Fill color
   */
  auto clear_region(std::size_t x_start, std::size_t y_start, std::size_t x_end, std::size_t y_end, Color color)
      -> void;

  /**
   * @brief Send framebuffer to display and refresh.
   *
   * This operation takes several seconds to complete. If auto-sleep is
   * enabled, the display will automatically enter sleep mode after refresh.
   */
  auto refresh() -> void;

  // ========== Power Management ==========

  /**
   * @brief Put display into low-power sleep mode.
   *
   * This method manually puts the display into sleep mode. It's useful when
   * auto-sleep is disabled and you want to control when the display sleeps.
   *
   * @note This is automatically called after refresh() if auto-sleep is enabled.
   */
  auto sleep() -> void;

  /**
   * @brief Wake display from sleep mode.
   *
   * Wakes the display from sleep mode. For displays that don't support
   * true wake from sleep (like EPD27), this will re-initialize the display.
   *
   * @return void on success, DriverError on failure
   */
  [[nodiscard]] auto wake() -> std::expected<void, DriverError>;

  /**
   * @brief Turn display power completely off.
   *
   * Performs a hardware power down of the display. This is different from
   * sleep() which puts the display in low-power mode but keeps it powered.
   *
   * @return void on success, DriverError on failure
   */
  [[nodiscard]] auto power_off() -> std::expected<void, DriverError>;

  /**
   * @brief Turn display power on.
   *
   * Powers on the display hardware. This should be called after power_off()
   * before attempting to use the display.
   *
   * @return void on success, DriverError on failure
   */
  [[nodiscard]] auto power_on() -> std::expected<void, DriverError>;

  /**
   * @brief Check if wake from sleep is supported.
   *
   * @return true if the driver supports waking from sleep without re-initialization
   */
  [[nodiscard]] auto supports_wake() const noexcept -> bool;

  /**
   * @brief Check if power control is supported.
   *
   * @return true if the driver supports power_off() and power_on() methods
   */
  [[nodiscard]] auto supports_power_control() const noexcept -> bool;

  // ========== Drawing Operations ==========

  /**
   * @brief Draw a point with specified size.
   *
   * @param x X coordinate
   * @param y Y coordinate
   * @param color Point color
   * @param pixel_size Size of the point (default: 1x1)
   */
  auto draw_point(std::size_t x, std::size_t y, Color color, DotPixel pixel_size = DotPixel::Pixel1x1) -> void;

  /**
   * @brief Draw a line between two points.
   *
   * Uses Bresenham's line algorithm for efficient rendering.
   *
   * @param x_start Starting X coordinate
   * @param y_start Starting Y coordinate
   * @param x_end Ending X coordinate
   * @param y_end Ending Y coordinate
   * @param color Line color
   * @param line_width Line width (default: 1x1)
   * @param style Line style (default: Solid)
   */
  auto draw_line(std::size_t x_start, std::size_t y_start, std::size_t x_end, std::size_t y_end, Color color,
                 DotPixel line_width = DotPixel::Pixel1x1, LineStyle style = LineStyle::Solid) -> void;

  /**
   * @brief Draw a rectangle.
   *
   * @param x_start Top-left X coordinate
   * @param y_start Top-left Y coordinate
   * @param x_end Bottom-right X coordinate
   * @param y_end Bottom-right Y coordinate
   * @param color Rectangle color
   * @param line_width Border width (default: 1x1)
   * @param fill Fill mode (default: Empty)
   */
  auto draw_rectangle(std::size_t x_start, std::size_t y_start, std::size_t x_end, std::size_t y_end, Color color,
                      DotPixel line_width = DotPixel::Pixel1x1, DrawFill fill = DrawFill::Empty) -> void;

  /**
   * @brief Draw a circle.
   *
   * Uses midpoint circle algorithm for efficient rendering.
   *
   * @param x_center Center X coordinate
   * @param y_center Center Y coordinate
   * @param radius Circle radius in pixels
   * @param color Circle color
   * @param line_width Border width (default: 1x1)
   * @param fill Fill mode (default: Empty)
   */
  auto draw_circle(std::size_t x_center, std::size_t y_center, std::size_t radius, Color color,
                   DotPixel line_width = DotPixel::Pixel1x1, DrawFill fill = DrawFill::Empty) -> void;

  // ========== Text Operations ==========

  /**
   * @brief Draw a single character.
   *
   * @param x X coordinate
   * @param y Y coordinate
   * @param character Character to draw (printable ASCII only)
   * @param font Font to use
   * @param foreground Foreground color
   * @param background Background color
   */
  auto draw_char(std::size_t x, std::size_t y, char character, const Font &font, Color foreground, Color background)
      -> void;

  /**
   * @brief Draw a text string.
   *
   * Supports newline (\n) and carriage return (\r) characters.
   *
   * @param x Starting X coordinate
   * @param y Starting Y coordinate
   * @param text Text to draw
   * @param font Font to use
   * @param foreground Foreground color
   * @param background Background color
   */
  auto draw_string(std::size_t x, std::size_t y, std::string_view text, const Font &font, Color foreground,
                   Color background) -> void;

  /**
   * @brief Draw an integer number.
   *
   * @param x X coordinate
   * @param y Y coordinate
   * @param number Number to draw
   * @param font Font to use
   * @param foreground Foreground color
   * @param background Background color
   */
  auto draw_number(std::size_t x, std::size_t y, std::int32_t number, const Font &font, Color foreground,
                   Color background) -> void;

  /**
   * @brief Draw a decimal number with specified precision.
   *
   * @param x X coordinate
   * @param y Y coordinate
   * @param number Number to draw
   * @param decimal_places Number of decimal places
   * @param font Font to use
   * @param foreground Foreground color
   * @param background Background color
   */
  auto draw_decimal(std::size_t x, std::size_t y, double number, std::uint8_t decimal_places, const Font &font,
                    Color foreground, Color background) -> void;

  // ========== Bitmap Operations ==========

  /**
   * @brief Draw a bitmap from memory.
   *
   * Supports optional scaling using nearest-neighbor interpolation.
   *
   * @param x X coordinate for top-left corner
   * @param y Y coordinate for top-left corner
   * @param pixels Bitmap pixel data (row-major order)
   * @param bitmap_width Bitmap width in pixels
   * @param bitmap_height Bitmap height in pixels
   * @param target_width Target width (0 = no scaling)
   * @param target_height Target height (0 = no scaling)
   */
  auto draw_bitmap(std::size_t x, std::size_t y, std::span<const Color> pixels, std::size_t bitmap_width,
                   std::size_t bitmap_height, std::size_t target_width = 0, std::size_t target_height = 0) -> void;

  /**
   * @brief Load and draw a bitmap from file.
   *
   * Supports common image formats (PNG, JPEG, BMP, etc.) using stb_image.
   * Images are automatically converted to grayscale and dithered to the
   * current display mode.
   *
   * @param x X coordinate for top-left corner
   * @param y Y coordinate for top-left corner
   * @param file_path Path to image file
   * @param target_width Target width (0 = original size)
   * @param target_height Target height (0 = original size)
   * @return void on success, BitmapError on failure
   */
  auto draw_bitmap_from_file(std::size_t x, std::size_t y, std::string_view file_path, std::size_t target_width = 0,
                             std::size_t target_height = 0) -> std::expected<void, BitmapError>;

  /**
   * @brief Get direct read-only access to the framebuffer.
   *
   * @return Constant span over the framebuffer data
   */
  [[nodiscard]] auto buffer() const -> std::span<const std::byte> { return buffer_; }

private:
  // ========== Helper Methods ==========

  // Transform logical coordinates to physical coordinates based on orientation
  [[nodiscard]] auto transform_coordinates(std::size_t x, std::size_t y) const -> std::pair<std::size_t, std::size_t>;

  // Calculate bit position in buffer for B/W mode
  [[nodiscard]] auto calculate_bw_position(std::size_t x, std::size_t y) const -> std::pair<std::size_t, std::uint8_t>;

  // Calculate byte position in buffer for grayscale mode
  [[nodiscard]] auto calculate_gray_position(std::size_t x, std::size_t y) const
      -> std::pair<std::size_t, std::uint8_t>;

  // Helper: draw horizontal line (optimized)
  auto draw_horizontal_line(std::size_t x_start, std::size_t x_end, std::size_t y, Color color, DotPixel width) -> void;

  // Helper: draw vertical line (optimized)
  auto draw_vertical_line(std::size_t x, std::size_t y_start, std::size_t y_end, Color color, DotPixel width) -> void;

  // Helper: convert RGB to Color enum
  auto rgb_to_color(std::uint8_t r, std::uint8_t g, std::uint8_t b) -> Color;

  // ========== Member Variables ==========

  std::unique_ptr<Driver> driver_;
  std::vector<std::byte> buffer_;
  std::size_t width_;
  std::size_t height_;
  DisplayMode mode_;
  Orientation orientation_;
  bool auto_sleep_enabled_;
};

/**
 * @brief Factory function to create a Display with specified driver type.
 *
 * This function handles driver creation, initialization, and Display
 * construction in a single step with proper error handling.
 *
 * @tparam DriverType The driver type (e.g., EPD27)
 * @param device Reference to initialized Device
 * @param mode Display mode (BlackWhite or Grayscale4)
 * @param orientation Display orientation (default: Portrait0)
 * @param auto_sleep Enable auto-sleep after refresh (default: true)
 * @return Display instance on success, DriverError on failure
 *
 * @example
 * @code{.cpp}
 * epaper::Device device;
 * device.init();
 *
 * auto display = epaper::create_display<epaper::EPD27>(
 *     device, epaper::DisplayMode::BlackWhite);
 *
 * if (display) {
 *     display->draw_string(0, 0, "Hello!", epaper::Font::font16(),
 *                         epaper::Color::Black, epaper::Color::White);
 *     display->refresh();
 * } else {
 *     std::cerr << "Failed to create display\n";
 * }
 * @endcode
 */
template <typename DriverType>
auto create_display(Device &device, DisplayMode mode, Orientation orientation = Orientation::Portrait0,
                    bool auto_sleep = true) -> std::expected<Display, DriverError> {
  auto driver = std::make_unique<DriverType>(device);
  if (auto result = driver->init(mode); !result) {
    return std::unexpected(result.error());
  }
  return Display(std::move(driver), orientation, auto_sleep);
}

} // namespace epaper
