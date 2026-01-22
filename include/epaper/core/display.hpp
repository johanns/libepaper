#pragma once

#include "epaper/core/device.hpp"
#include "epaper/core/errors.hpp"
#include "epaper/core/framebuffer.hpp"
#include "epaper/core/framebuffer_concepts.hpp"
#include "epaper/core/types.hpp"
#include "epaper/draw/builders.hpp"
#include "epaper/draw/commands.hpp"
#include "epaper/drivers/capabilities.hpp"
#include "epaper/drivers/driver.hpp"
#include "epaper/drivers/driver_concepts.hpp"
#include "epaper/graphics/graphics.hpp"
#include "epaper/io/image_io.hpp"
#include <cstddef>
#include <expected>
#include <span>
#include <string_view>
#include <vector>

namespace epaper {

// Forward declarations used in drawing
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
class Font;

/**
 * @brief Unified display interface for e-paper displays.
 *
 * Coordinates display driver and framebuffer, providing high-level drawing
 * and display management operations. Uses compile-time polymorphism (templates)
 * to avoid virtual function overhead while maintaining type safety.
 *
 * **Architecture:**
 * - `DriverT`: Hardware driver implementing low-level SPI/GPIO communication
 * - `FramebufferT`: Memory buffer managing pixel data and color encoding
 * - Orientation transforms applied automatically by framebuffer
 * - Auto-sleep manages power state across refresh operations
 *
 * **Thread Safety:** Not thread-safe. External synchronization required.
 *
 * @tparam DriverT The concrete driver type (e.g., EPD27, MockDriver)
 * @tparam FramebufferT The concrete framebuffer type (MonoFramebuffer or MultiPlaneFramebuffer<N>)
 *
 * @example
 * ```cpp
 * // Complete display initialization and usage
 * Device device{};
 * if (auto result = device.init(); !result) {
 *   std::cerr << "Device init failed: " << result.error().what() << std::endl;
 *   return EXIT_FAILURE;
 * }
 *
 * // Create display with EPD27 driver
 * auto display_result = create_display<EPD27>(device, DisplayMode::BlackWhite);
 * if (!display_result) {
 *   std::cerr << "Display creation failed: " << display_result.error().what() << std::endl;
 *   return EXIT_FAILURE;
 * }
 * auto display = std::move(*display_result);
 *
 * // Drawing operations
 * display.clear(Color::White);
 * display.line({0, 0}, {100, 100}).color(Color::Black).draw();
 * display.rectangle({10, 10}, {50, 50}).fill(DrawFill::Full).draw();
 * display.text("Hello, e-paper!").at({60, 20}).font(font).draw();
 *
 * // Refresh display (sends framebuffer to hardware)
 * if (auto result = display.refresh(); !result) {
 *   std::cerr << "Refresh failed: " << result.error().what() << std::endl;
 * }
 *
 * // Power management
 * display.sleep();  // Low power mode
 * display.wake();   // Resume from sleep
 * ```
 *
 * @see create_display(), MonoFramebuffer, MultiPlaneFramebuffer, Driver
 */
template <Driver DriverT, FramebufferLike FramebufferT> class Display {
public:
  Display(DriverT driver, DisplayMode mode, Orientation orientation = Orientation::Portrait0, bool auto_sleep = true)
      : driver_(std::move(driver)), framebuffer_(driver_.width(), driver_.height(), mode), orientation_(orientation),
        auto_sleep_enabled_(auto_sleep), physical_width_(driver_.width()), physical_height_(driver_.height()),
        display_mode_(mode) {}

  // Non-copyable, movable
  Display(const Display &) = delete;
  auto operator=(const Display &) -> Display & = delete;
  Display(Display &&) noexcept = default;
  auto operator=(Display &&) noexcept -> Display & = default;
  ~Display() = default;

  // --- Core Functionality (formerly DisplayCore) ---

  [[nodiscard]] auto width() const noexcept -> std::size_t {
    return (orientation_ == Orientation::Portrait0 || orientation_ == Orientation::Portrait180) ? physical_width_
                                                                                                : physical_height_;
  }

  [[nodiscard]] auto height() const noexcept -> std::size_t {
    return (orientation_ == Orientation::Portrait0 || orientation_ == Orientation::Portrait180) ? physical_height_
                                                                                                : physical_width_;
  }

  [[nodiscard]] auto effective_width() const noexcept -> std::size_t { return width(); }
  [[nodiscard]] auto effective_height() const noexcept -> std::size_t { return height(); }
  [[nodiscard]] auto mode() const noexcept -> DisplayMode { return display_mode_; }

  [[nodiscard]] auto orientation() const noexcept -> Orientation { return orientation_; }
  [[nodiscard]] auto auto_sleep_enabled() const noexcept -> bool { return auto_sleep_enabled_; }
  auto set_auto_sleep(bool enabled) noexcept -> void { auto_sleep_enabled_ = enabled; }

  [[nodiscard]] auto is_color() const noexcept -> bool { return epaper::is_color_mode(mode()); }
  [[nodiscard]] auto get_num_planes() const noexcept -> std::size_t { return epaper::num_planes(mode()); }

  [[nodiscard]] auto available_colors() const -> std::vector<Color> {
    const auto m = mode();
    if (!is_color()) {
      return {Color::Black, Color::White};
    }
    if (m == DisplayMode::BWR) {
      return {Color::Black, Color::White, Color::Red};
    }
    if (m == DisplayMode::BWY) {
      return {Color::Black, Color::White, Color::Yellow};
    }
    if (m == DisplayMode::Spectra6) {
      return {Color::Black, Color::White, Color::Red, Color::Green, Color::Blue, Color::Yellow};
    }
    return {Color::Black, Color::White, Color::Red, Color::Yellow};
  }

  auto set_pixel(std::size_t x, std::size_t y, Color color) -> void {
    framebuffer_.set_pixel(x, y, color, orientation_);
  }

  [[nodiscard]] auto get_pixel(std::size_t x, std::size_t y) const -> Color {
    return framebuffer_.get_pixel(x, y, orientation_);
  }

  auto clear(Color color = Color::White) -> void { framebuffer_.clear(color); }

  // --- Builder Factories ---
  [[nodiscard]] auto line() -> LineBuilder { return {}; }
  [[nodiscard]] auto rectangle() -> RectangleBuilder { return {}; }
  [[nodiscard]] auto circle() -> CircleBuilder { return {}; }
  [[nodiscard]] auto point() -> PointBuilder { return {}; }
  [[nodiscard]] auto text(std::string_view content = "") -> TextBuilder { return TextBuilder(std::string(content)); }

  // --- Drawing Commands ---
  auto draw(const LineCommand &cmd) -> void {
    Graphics::draw_line(framebuffer_, cmd.from, cmd.to, cmd.style, cmd.color, orientation_);
  }

  auto draw(const RectangleCommand &cmd) -> void {
    Graphics::draw_rectangle(framebuffer_, cmd.top_left, cmd.bottom_right, LineStyle::Solid, cmd.color, cmd.fill,
                             orientation_);
  }

  auto draw(const CircleCommand &cmd) -> void {
    Graphics::draw_circle(framebuffer_, cmd.center, cmd.radius, LineStyle::Solid, cmd.color, cmd.fill, orientation_);
  }

  auto draw(const PointCommand &cmd) -> void {
    framebuffer_.set_pixel(cmd.position.x, cmd.position.y, cmd.color, orientation_);
  }

  auto draw(const TextCommand &cmd) -> void {
    Graphics::draw_text(framebuffer_, cmd.position, cmd.text, *cmd.font, cmd.foreground, cmd.background, orientation_);
  }

  // --- Bitmap Drawing ---
  auto draw_bitmap(std::size_t x, std::size_t y, std::span<const std::uint8_t> data, std::size_t w, std::size_t h,
                   std::size_t target_w = 0, std::size_t target_h = 0) -> void {
    Graphics::draw_bitmap(framebuffer_, {x, y}, data, w, h, target_w, target_h, orientation_);
  }

  auto draw_bitmap(std::size_t x, std::size_t y, std::span<const Color> data, std::size_t w, std::size_t h,
                   std::size_t target_w = 0, std::size_t target_h = 0) -> void {
    auto bytes = std::span<const std::uint8_t>(reinterpret_cast<const std::uint8_t *>(data.data()), data.size());
    Graphics::draw_bitmap(framebuffer_, {x, y}, bytes, w, h, target_w, target_h, orientation_);
  }

  template <typename T>
  auto draw_bitmap(std::size_t x, std::size_t y, const std::vector<T> &data, std::size_t w, std::size_t h,
                   std::size_t target_w = 0, std::size_t target_h = 0) -> void {
    draw_bitmap(x, y, std::span(data), w, h, target_w, target_h);
  }

  [[nodiscard]] auto draw_bitmap_from_file(std::size_t x, std::size_t y, std::string_view file_path,
                                           std::size_t target_width = 0, std::size_t target_height = 0)
      -> std::expected<void, Error> {
    auto image_res = ImageIO::load_image(file_path, 0);
    if (!image_res) {
      return std::unexpected(image_res.error());
    }

    const auto &img = *image_res;
    std::size_t draw_w = (target_width > 0) ? target_width : img.width;
    std::size_t draw_h = (target_height > 0) ? target_height : img.height;

    for (std::size_t dy = 0; dy < draw_h; ++dy) {
      for (std::size_t dx = 0; dx < draw_w; ++dx) {
        std::size_t sx = dx * img.width / draw_w;
        std::size_t sy = dy * img.height / draw_h;
        std::size_t src_idx = (sy * img.width + sx) * img.channels;

        std::uint8_t r = 0;
        std::uint8_t g = 0;
        std::uint8_t b = 0;
        if (img.channels >= 3) {
          r = img.data[src_idx];
          g = img.data[src_idx + 1];
          b = img.data[src_idx + 2];
        } else if (img.channels == 1) {
          r = g = b = img.data[src_idx];
        }

        // Simple color quantization for generic display
        Color color = Color::White;
        if (r < 128 && g < 128 && b < 128) {
          color = Color::Black;
        } else if (r > 200 && g < 100 && b < 100) {
          color = Color::Red;
        } else if (r > 200 && g > 200 && b < 100) {
          color = Color::Yellow;
        } else if (r < 100 && g < 100 && b > 200) {
          color = Color::Blue;
        } else if (r < 100 && g > 200 && b < 100) {
          color = Color::Green;
        }

        set_pixel(x + dx, y + dy, color);
      }
    }
    return {};
  }

  [[nodiscard]] auto save_framebuffer_to_png(std::string_view filename) const -> std::expected<void, Error> {
    auto rgb_data = ImageIO::framebuffer_to_rgb(framebuffer_);
    return ImageIO::save_png(filename, framebuffer_.width(), framebuffer_.height(), 3, rgb_data);
  }

  /**
   * @brief Refresh the display by transferring framebuffer to hardware.
   *
   * Executes the following sequence:
   * 1. Wake driver from sleep (if auto_sleep_enabled)
   * 2. Transfer framebuffer data via SPI
   * 3. Trigger display update (hardware-specific timing)
   * 4. Return to sleep mode (if auto_sleep_enabled)
   *
   * @return void on success, Error on failure (transfer/timeout/driver errors)
   *
   * @note This operation blocks until the display refresh completes.
   *       Refresh time varies by driver and mode (typically 1-15 seconds).
   *
   * @pre Display must be initialized via create_display()
   * @post Framebuffer contents are visible on physical display
   */
  auto refresh() -> std::expected<void, Error> {
    // Auto-Sleep State Machine:
    // State 1: ASLEEP (low power) → wake() → State 2: AWAKE (active)
    // State 2: AWAKE → display() → AWAKE (still active)
    // State 2: AWAKE → sleep() → State 1: ASLEEP (low power)
    //
    // This implements automatic power management to minimize energy consumption
    // on battery-powered devices. The display wakes only during refresh operations.

    if (auto_sleep_enabled_) {
      // Wake from sleep mode before display transfer
      // Driver may be in deep sleep (VCOM off, high-impedance outputs)
      // wake() re-initializes power rails and prepares for SPI communication
      if (auto res = driver_.wake(); !res && res.error().code != ErrorCode::InvalidMode) {
        // InvalidMode indicates driver doesn't support wake (always active)
        // Other errors (GPIO failures, timeout) are fatal
        return std::unexpected(res.error());
      }
    }

    // Transfer framebuffer to display controller
    // Multi-plane displays (BWR, BWY) require separate plane buffers
    // Single-plane displays use contiguous framebuffer
    if (get_num_planes() > 1) {
      if (auto res = driver_.display_planes(get_planes()); !res) {
        return std::unexpected(res.error());
      }
    } else {
      if (auto res = driver_.display(get_buffer()); !res) {
        return std::unexpected(res.error());
      }
    }

    if (auto_sleep_enabled_) {
      // Return to sleep mode after refresh completes
      // sleep() powers down VCOM, enters deep sleep, reduces current draw
      // Typical power: Active ~20mA, Sleep <1µA
      if (auto res = driver_.sleep(); !res) {
        return std::unexpected(res.error());
      }
    }
    return {};
  }

  auto sleep() -> std::expected<void, Error> { return driver_.sleep(); }
  auto wake() -> std::expected<void, Error> { return driver_.wake(); }
  auto power_off() -> std::expected<void, Error> { return driver_.power_off(); }
  auto power_on() -> std::expected<void, Error> { return driver_.power_on(); }

  [[nodiscard]] auto supports_wake() const noexcept -> bool { return driver_.supports_wake(); }
  [[nodiscard]] auto supports_power_control() const noexcept -> bool { return driver_.supports_power_control(); }

  auto driver() -> DriverT & { return driver_; }
  auto driver() const -> const DriverT & { return driver_; }

  // Accessors mainly for testing or advanced usage
  auto framebuffer() -> FramebufferT & { return framebuffer_; }
  auto framebuffer() const -> const FramebufferT & { return framebuffer_; }

private:
  [[nodiscard]] auto get_planes() const -> std::vector<std::span<const std::byte>> { return framebuffer_.get_planes(); }

  [[nodiscard]] auto get_buffer() const -> std::span<const std::byte> { return framebuffer_.data(); }

  DriverT driver_;
  FramebufferT framebuffer_;
  Orientation orientation_;
  bool auto_sleep_enabled_;
  std::size_t physical_width_;
  std::size_t physical_height_;
  DisplayMode display_mode_;
};

/**
 * @brief Primary factory function for creating displays (runtime mode selection).
 *
 * This is the main entry point that most code should use. It determines the
 * correct framebuffer type at runtime based on the mode parameter and returns
 * a properly typed Display instance. Performs driver initialization and
 * capability validation.
 *
 * **Validation Steps:**
 * - Checks if driver supports requested DisplayMode
 * - Verifies framebuffer compatibility with mode
 * - Initializes driver hardware interface
 * - Configures display orientation and auto-sleep
 *
 * @tparam DriverType The driver type to use (e.g., EPD27, MockDriver)
 * @param device Initialized Device instance (must have init() called successfully)
 * @param mode Display mode (determined at runtime - BlackWhite, Grayscale4, BWR, etc.)
 * @param orientation Display orientation (default: Portrait0)
 * @param auto_sleep Enable auto-sleep after refresh (default: true for power savings)
 * @return Display with correctly typed framebuffer on success, Error on failure
 *
 * @example
 * ```cpp
 * Device device{};
 * device.init();
 *
 * // Simple creation with defaults
 * auto result = create_display<EPD27>(device, DisplayMode::BlackWhite);
 *
 * // Custom orientation and no auto-sleep
 * auto result2 = create_display<EPD27>(device,
 *                                      DisplayMode::BWR,
 *                                      Orientation::Landscape90,
 *                                      false);
 * ```
 *
 * @see Display, Device::init(), DisplayMode
 */
/**
 * @brief Convenience factory for mono-plane displays.
 *
 * Defaults to `MonoFramebuffer` and validates that the requested mode is single-plane.
 *
 * @tparam DriverType The driver type to use
 * @param device Initialized device
 * @param mode Display mode
 * @param orientation Display orientation
 * @param auto_sleep Enable auto-sleep after refresh
 * @return Display with a MonoFramebuffer on success, Error on failure
 */
template <typename DriverType>
  requires Driver<DriverType> && DriverTraits<DriverType>
[[nodiscard]] auto create_display(Device &device, DisplayMode mode, Orientation orientation = Orientation::Portrait0,
                                  bool auto_sleep = true)
    -> std::expected<Display<DriverType, MonoFramebuffer>, Error> {
  return create_display<DriverType, MonoFramebuffer>(device, mode, orientation, auto_sleep);
}

/**
 * @brief Primary factory function for creating displays with explicit framebuffer type.
 *
 * @tparam DriverType The driver type to use
 * @tparam FramebufferT Framebuffer type to use
 * @param device Initialized device
 * @param mode Display mode
 * @param orientation Display orientation
 * @param auto_sleep Enable auto-sleep after refresh
 * @return Display with the requested framebuffer type on success, Error on failure
 */
template <typename DriverType, FramebufferLike FramebufferT>
  requires Driver<DriverType> && DriverTraits<DriverType>
[[nodiscard]] auto create_display(Device &device, DisplayMode mode, Orientation orientation = Orientation::Portrait0,
                                  bool auto_sleep = true) -> std::expected<Display<DriverType, FramebufferT>, Error> {
  // Validate capabilities
  if (mode == DisplayMode::Grayscale4 && !driver_traits<DriverType>::supports_grayscale) {
    return std::unexpected(Error(ErrorCode::InvalidMode, "Grayscale not supported by this driver"));
  }

  if (mode > driver_traits<DriverType>::max_mode) {
    return std::unexpected(Error(ErrorCode::InvalidMode, "Display mode exceeds driver capabilities"));
  }

  if (!FramebufferT::supports_mode(mode)) {
    return std::unexpected(Error(ErrorCode::InvalidMode, "Display mode not supported by framebuffer"));
  }

  auto driver = DriverType(device);
  if (auto result = driver.init(mode); !result) {
    return std::unexpected(result.error());
  }

  return Display<DriverType, FramebufferT>(std::move(driver), mode, orientation, auto_sleep);
}

} // namespace epaper

// Also include command builders and image I/O to maintain API compatibility
