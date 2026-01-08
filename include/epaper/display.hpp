#pragma once

#include "epaper/device.hpp"
#include "epaper/drivers/driver.hpp"
#include "epaper/errors.hpp"
#include "epaper/font.hpp"
#include "epaper/geometry.hpp"
#include <cstddef>
#include <cstdint>
#include <expected>
#include <memory>
#include <span>
#include <string_view>
#include <vector>

namespace epaper {

// Forward declarations for types defined later
struct LineCommand;
struct RectangleCommand;
struct CircleCommand;
struct PointCommand;
struct TextCommand;
class LineBuilder;
class RectangleBuilder;
class CircleBuilder;
class PointBuilder;
class TextBuilder;

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
 * @note Exception Safety: All operations provide at least the basic guarantee.
 *       The Display object remains in a valid state after any operation.
 *       Drawing operations never throw and silently clip out-of-bounds coordinates.
 *       Move operations are noexcept.
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
 *     if (auto result = display->refresh(); !result) {
 *         std::cerr << "Refresh failed: " << result.error().what() << "\n";
 *     }
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
   * @note Exception Safety: Strong guarantee - throws on allocation failure.
   */
  explicit Display(std::unique_ptr<Driver> driver, Orientation orientation = Orientation::Portrait0,
                   bool auto_sleep = true);

  // Non-copyable, movable
  Display(const Display &) = delete;
  auto operator=(const Display &) -> Display & = delete;

  /**
   * @brief Move constructor.
   *
   * @param other Display to move from
   * @note Exception Safety: Nothrow guarantee.
   */
  Display(Display &&) noexcept = default;

  /**
   * @brief Move assignment operator.
   *
   * @param other Display to move from
   * @return Reference to this display
   * @note Exception Safety: Nothrow guarantee.
   */
  auto operator=(Display &&) noexcept -> Display & = default;

  /**
   * @brief Destructor.
   *
   * @note Exception Safety: Nothrow guarantee - never throws.
   */
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
   *
   * @return void on success, Error on failure
   * @note Exception Safety: Basic guarantee - display remains in valid state.
   */
  [[nodiscard]] auto refresh() -> std::expected<void, Error>;

  // ========== Power Management ==========

  /**
   * @brief Put display into low-power sleep mode.
   *
   * This method manually puts the display into sleep mode. It's useful when
   * auto-sleep is disabled and you want to control when the display sleeps.
   *
   * @note This is automatically called after refresh() if auto-sleep is enabled.
   * @note Exception Safety: Basic guarantee - display remains in valid state.
   */
  auto sleep() -> void;

  /**
   * @brief Wake display from sleep mode.
   *
   * Wakes the display from sleep mode. For displays that don't support
   * true wake from sleep (like EPD27), this will re-initialize the display.
   *
   * @return void on success, Error on failure
   * @note Exception Safety: Basic guarantee - display remains in valid state.
   */
  [[nodiscard]] auto wake() -> std::expected<void, Error>;

  /**
   * @brief Turn display power completely off.
   *
   * Performs a hardware power down of the display. This is different from
   * sleep() which puts the display in low-power mode but keeps it powered.
   *
   * @return void on success, Error on failure
   * @note Exception Safety: Basic guarantee - display remains in valid state.
   */
  [[nodiscard]] auto power_off() -> std::expected<void, Error>;

  /**
   * @brief Turn display power on.
   *
   * Powers on the display hardware. This should be called after power_off()
   * before attempting to use the display.
   *
   * @return void on success, Error on failure
   * @note Exception Safety: Basic guarantee - display remains in valid state.
   */
  [[nodiscard]] auto power_on() -> std::expected<void, Error>;

  /**
   * @brief Check if wake from sleep is supported.
   *
   * Note: With transparent sleep/wake management, all displays effectively
   * support wake (the library handles re-initialization if needed).
   *
   * @return true (always - transparent wake is handled internally)
   */
  [[nodiscard]] static auto supports_wake() noexcept -> bool { return true; }

  /**
   * @brief Check if power control is supported.
   *
   * @return true if the driver supports power_off() and power_on() methods
   */
  [[nodiscard]] auto supports_power_control() const noexcept -> bool;

  // ========== Drawing Operations (Builder API) ==========
  //
  // Modern fluent builder interface for drawing operations.
  // All drawing operations:
  // - Out-of-bounds coordinates are silently clipped (no errors)
  // - Operations never fail or throw exceptions
  // - Coordinates are in logical space (accounting for display orientation)
  // - Drawing is performed on the framebuffer; call refresh() to update the display
  //
  // Use the factory methods (line(), rectangle(), etc.) to create builders,
  // then call build() to produce a command, and pass it to draw().
  //
  // @example
  // @code{.cpp}
  // display.draw(
  //     display.line()
  //         .from({10, 10})
  //         .to({100, 100})
  //         .color(Color::Black)
  //         .width(DotPixel::Pixel2x2)
  //         .build()
  // );
  // @endcode

  /**
   * @brief Create a line builder for fluent line drawing.
   *
   * @return LineBuilder for constructing a line command
   *
   * @example
   * @code{.cpp}
   * display.draw(
   *     display.line()
   *         .from({10, 10})
   *         .to({100, 100})
   *         .color(Color::Black)
   *         .build()
   * );
   * @endcode
   */
  [[nodiscard]] auto line() -> LineBuilder;

  /**
   * @brief Create a rectangle builder for fluent rectangle drawing.
   *
   * @return RectangleBuilder for constructing a rectangle command
   *
   * @example
   * @code{.cpp}
   * display.draw(
   *     display.rectangle()
   *         .top_left({10, 10})
   *         .bottom_right({100, 50})
   *         .color(Color::Black)
   *         .fill(DrawFill::Empty)
   *         .build()
   * );
   * @endcode
   */
  [[nodiscard]] auto rectangle() -> RectangleBuilder;

  /**
   * @brief Create a circle builder for fluent circle drawing.
   *
   * @return CircleBuilder for constructing a circle command
   *
   * @example
   * @code{.cpp}
   * display.draw(
   *     display.circle()
   *         .center({50, 50})
   *         .radius(25)
   *         .color(Color::Black)
   *         .fill(DrawFill::Full)
   *         .build()
   * );
   * @endcode
   */
  [[nodiscard]] auto circle() -> CircleBuilder;

  /**
   * @brief Create a point builder for fluent point drawing.
   *
   * @return PointBuilder for constructing a point command
   *
   * @example
   * @code{.cpp}
   * display.draw(
   *     display.point()
   *         .at({10, 10})
   *         .color(Color::Black)
   *         .size(DotPixel::Pixel3x3)
   *         .build()
   * );
   * @endcode
   */
  [[nodiscard]] auto point() -> PointBuilder;

  /**
   * @brief Create a text builder for fluent text drawing.
   *
   * @param content Text content (optional, can be set via builder)
   * @return TextBuilder for constructing a text command
   *
   * @example
   * @code{.cpp}
   * display.draw(
   *     display.text("Hello")
   *         .at({10, 20})
   *         .font(&Font::font16())
   *         .foreground(Color::Black)
   *         .background(Color::White)
   *         .build()
   * );
   * @endcode
   */
  [[nodiscard]] auto text(std::string_view content = "") -> TextBuilder;

  /**
   * @brief Draw a line using a command.
   *
   * Uses Bresenham's line algorithm for efficient rendering.
   *
   * @param cmd LineCommand produced by LineBuilder
   * @note Exception Safety: Nothrow guarantee - never throws.
   */
  auto draw(const LineCommand &cmd) -> void;

  /**
   * @brief Draw a rectangle using a command.
   *
   * @param cmd RectangleCommand produced by RectangleBuilder
   * @note Exception Safety: Nothrow guarantee - never throws.
   */
  auto draw(const RectangleCommand &cmd) -> void;

  /**
   * @brief Draw a circle using a command.
   *
   * Uses midpoint circle algorithm for efficient rendering.
   *
   * @param cmd CircleCommand produced by CircleBuilder
   * @note Exception Safety: Nothrow guarantee - never throws.
   */
  auto draw(const CircleCommand &cmd) -> void;

  /**
   * @brief Draw a point using a command.
   *
   * @param cmd PointCommand produced by PointBuilder
   * @note Exception Safety: Nothrow guarantee - never throws.
   */
  auto draw(const PointCommand &cmd) -> void;

  /**
   * @brief Draw text using a command.
   *
   * Supports strings, numbers, and decimal numbers.
   * Text supports newline (\n) and carriage return (\r) characters.
   *
   * @param cmd TextCommand produced by TextBuilder
   * @note Exception Safety: Nothrow guarantee - never throws.
   */
  auto draw(const TextCommand &cmd) -> void;

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
   * @return void on success, Error on failure
   * @note Exception Safety: Strong guarantee - framebuffer unchanged on failure.
   */
  [[nodiscard]] auto draw_bitmap_from_file(std::size_t x, std::size_t y, std::string_view file_path,
                                           std::size_t target_width = 0, std::size_t target_height = 0)
      -> std::expected<void, Error>;

  /**
   * @brief Get direct read-only access to the framebuffer.
   *
   * @return Constant span over the framebuffer data
   */
  [[nodiscard]] auto buffer() const -> std::span<const std::byte> { return buffer_; }

  // ========== Debugging Operations ==========

  /**
   * @brief Save framebuffer contents to BMP file for debugging.
   *
   * Exports the current framebuffer to a 24-bit RGB BMP file, applying
   * orientation transforms so the image matches what will appear on the
   * display after refresh(). Useful for debugging layouts without waiting
   * for slow display refresh cycles.
   *
   * Color mapping:
   * - BlackWhite mode: Black→(0,0,0), White→(255,255,255)
   * - Grayscale4 mode: Black→(0,0,0), Gray2→(85,85,85), Gray1→(170,170,170), White→(255,255,255)
   *
   * @param filename Path to output BMP file (will be overwritten if exists)
   * @return void on success, Error on failure
   * @note Exception Safety: Strong guarantee - file unchanged on failure.
   *
   * @example
   * @code{.cpp}
   * display->clear(Color::White);
   * display->draw_string(10, 10, "Debug", Font::font16(), Color::Black, Color::White);
   * display->save_framebuffer_to_bmp("debug_frame.bmp");  // Save before refresh
   * display->refresh();  // Now refresh to display
   * @endcode
   */
  [[nodiscard]] auto save_framebuffer_to_bmp(std::string_view filename) const -> std::expected<void, Error>;

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

  // ========== Internal Drawing Implementations ==========

  // Internal implementations used by draw() overloads
  auto draw_point_impl(std::size_t x, std::size_t y, Color color, DotPixel pixel_size) -> void;
  auto draw_line_impl(std::size_t x_start, std::size_t y_start, std::size_t x_end, std::size_t y_end, Color color,
                      DotPixel line_width, LineStyle style) -> void;
  auto draw_rectangle_impl(std::size_t x_start, std::size_t y_start, std::size_t x_end, std::size_t y_end, Color color,
                           DotPixel line_width, DrawFill fill) -> void;
  auto draw_circle_impl(std::size_t x_center, std::size_t y_center, std::size_t radius, Color color,
                        DotPixel line_width, DrawFill fill) -> void;
  auto draw_char_impl(std::size_t x, std::size_t y, char character, const Font &font, Color foreground,
                      Color background) -> void;
  auto draw_string_impl(std::size_t x, std::size_t y, std::string_view text, const Font &font, Color foreground,
                        Color background) -> void;
  auto draw_number_impl(std::size_t x, std::size_t y, std::int32_t number, const Font &font, Color foreground,
                        Color background) -> void;
  auto draw_decimal_impl(std::size_t x, std::size_t y, double number, std::uint8_t decimal_places, const Font &font,
                         Color foreground, Color background) -> void;

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
 * @return Display instance on success, Error on failure
 *
 * @note Exception Safety: Strong guarantee - no resources allocated on failure.
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
 *     if (auto result = display->refresh(); !result) {
 *         std::cerr << "Refresh failed: " << result.error().what() << "\n";
 *     }
 * } else {
 *     std::cerr << "Failed to create display: " << display.error().what() << "\n";
 * }
 * @endcode
 */
template <typename DriverType>
[[nodiscard]] auto create_display(Device &device, DisplayMode mode, Orientation orientation = Orientation::Portrait0,
                                  bool auto_sleep = true) -> std::expected<Display, Error> {
  auto driver = std::make_unique<DriverType>(device);
  if (auto result = driver->init(mode); !result) {
    return std::unexpected(result.error());
  }
  return Display(std::move(driver), orientation, auto_sleep);
}

} // namespace epaper

// Include builder definitions after Display class is complete
#include "epaper/draw/builders.hpp"
#include "epaper/draw/commands.hpp"
