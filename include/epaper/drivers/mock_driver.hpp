#pragma once

#include "epaper/core/device.hpp"
#include "epaper/core/errors.hpp"
#include "epaper/drivers/capabilities.hpp"
#include "epaper/drivers/driver.hpp"
#include "epaper/graphics/pixel_codec.hpp"
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <format>
#include <span>
#include <vector>

// Forward declare stb_image_write function
extern "C" auto stbi_write_png(char const *filename, int w, int h, int comp, const void *data, int stride_in_bytes)
    -> int;

namespace epaper {

/**
 * @brief Mock driver for testing without hardware (Linux/Raspberry Pi).
 *
 * Used for CI testing, unit tests, and development without physical displays.
 *
 * Features:
 * - Configurable dimensions
 * - Records method calls for verification
 * - Configurable return values for error testing
 * - Optional PNG image saving
 *
 * @example
 * @code{.cpp}
 * Device device;
 * device.init();
 * MockDriver driver{device, 176, 264};
 * driver.init(DisplayMode::BlackWhite);
 * driver.display(buffer);
 * assert(driver.display_called());
 * @endcode
 */
class MockDriver {
public:
  /**
   * @brief Construct MockDriver using Linux Device with custom dimensions.
   *
   * @param device Initialized Linux device
   * @param width Display width in pixels (default: 600)
   * @param height Display height in pixels (default: 300)
   * @param save_images Whether to automatically save PNG images on refresh (default: true)
   */
  explicit MockDriver(Device &device, std::size_t width = 600, std::size_t height = 300,
                      bool save_images = true) noexcept
      : spi_(device.get_spi()), cs_(device.get_output(Pin{0})), // Dummy CS pin
        dc_(device.get_output(Pin{1})),                         // Dummy DC pin
        rst_(device.get_output(Pin{2})),                        // Dummy RST pin
        busy_(device.get_input(Pin{3})),                        // Dummy BUSY pin
        width_(width), height_(height), mode_(DisplayMode::BlackWhite), save_images_(save_images),
        output_dir_("mock_outputs") {
    if (save_images_) {
      std::filesystem::create_directories(output_dir_);
    }
  }

  ~MockDriver() = default;

  // Lifecycle
  [[nodiscard]] auto init(DisplayMode mode) -> std::expected<void, Error> {
    ++init_count_;
    if (init_should_fail_) {
      return std::unexpected(Error(ErrorCode::DriverInitFailed, "MockDriver: init configured to fail"));
    }
    mode_ = mode;
    initialized_ = true;
    is_asleep_ = false;
    return {};
  }

  [[nodiscard]] auto clear() -> std::expected<void, Error> {
    ++clear_count_;
    if (!initialized_) {
      return std::unexpected(Error(ErrorCode::DriverNotInitialized, "MockDriver: not initialized"));
    }
    return {};
  }

  [[nodiscard]] auto display(std::span<const std::byte> buffer) -> std::expected<void, Error> {
    ++display_count_;
    if (!initialized_) {
      return std::unexpected(Error(ErrorCode::DriverNotInitialized, "MockDriver: not initialized"));
    }
    if (display_should_fail_) {
      return std::unexpected(Error(ErrorCode::RefreshFailed, "MockDriver: display configured to fail"));
    }

    // Auto-wake if asleep
    if (is_asleep_) {
      if (auto result = wake(); !result) {
        return result;
      }
    }

    // Store buffer for verification
    last_buffer_ = std::vector<std::byte>(buffer.begin(), buffer.end());

    // Save time-stamped image if enabled
    if (save_images_) {
      save_buffer_as_png(buffer);
    }

    return {};
  }

  [[nodiscard]] auto display_planes(std::span<const std::span<const std::byte>> planes) -> std::expected<void, Error> {
    if (planes.empty()) {
      return std::unexpected(Error(ErrorCode::InvalidDimensions, "MockDriver: no planes provided"));
    }

    if (planes.size() == 1) {
      return display(planes[0]);
    }

    // Combine planes into a single contiguous buffer for saving
    // This supports BWR/BWY modes where pixel_codec expects [Plane1][Plane2] layout
    std::vector<std::byte> combined_buffer;
    // Pre-calculate size to avoid reallocations
    std::size_t total_size = 0;
    for (const auto &plane : planes) {
      total_size += plane.size();
    }
    combined_buffer.reserve(total_size);

    for (const auto &plane : planes) {
      combined_buffer.insert(combined_buffer.end(), plane.begin(), plane.end());
    }

    return display(combined_buffer);
  }

  // Power management
  [[nodiscard]] auto sleep() -> std::expected<void, Error> {
    ++sleep_count_;
    if (sleep_should_fail_) {
      return std::unexpected(Error(ErrorCode::TransferFailed, "MockDriver: sleep configured to fail"));
    }
    is_asleep_ = true;
    return {};
  }

  [[nodiscard]] auto wake() -> std::expected<void, Error> {
    ++wake_count_;
    if (wake_should_fail_) {
      return std::unexpected(Error(ErrorCode::DriverInitFailed, "MockDriver: wake configured to fail"));
    }
    is_asleep_ = false;
    return {};
  }

  [[nodiscard]] auto power_off() -> std::expected<void, Error> {
    // Mock: no-op, always succeeds
    return {};
  }

  [[nodiscard]] auto power_on() -> std::expected<void, Error> {
    // Mock: no-op, always succeeds
    return {};
  }

  // Capabilities
  [[nodiscard]] auto width() const noexcept -> std::size_t { return width_; }

  [[nodiscard]] auto height() const noexcept -> std::size_t { return height_; }

  [[nodiscard]] auto mode() const noexcept -> DisplayMode { return mode_; }

  [[nodiscard]] auto buffer_size() const noexcept -> std::size_t {
    const auto bpp = bits_per_pixel(mode_);
    return (width_ * height_ * bpp + 7) / 8; // Round up to nearest byte
  }

  [[nodiscard]] auto supports_partial_refresh() const noexcept -> bool { return false; }

  [[nodiscard]] auto supports_power_control() const noexcept -> bool { return false; }

  [[nodiscard]] auto supports_wake() const noexcept -> bool { return true; }

  // Test configuration methods
  auto configure_init_failure(bool should_fail) noexcept -> void { init_should_fail_ = should_fail; }

  auto configure_display_failure(bool should_fail) noexcept -> void { display_should_fail_ = should_fail; }

  auto configure_sleep_failure(bool should_fail) noexcept -> void { sleep_should_fail_ = should_fail; }

  auto configure_wake_failure(bool should_fail) noexcept -> void { wake_should_fail_ = should_fail; }

  // Test verification methods
  [[nodiscard]] auto init_called() const noexcept -> bool { return init_count_ > 0; }

  [[nodiscard]] auto display_called() const noexcept -> bool { return display_count_ > 0; }

  [[nodiscard]] auto sleep_called() const noexcept -> bool { return sleep_count_ > 0; }

  [[nodiscard]] auto wake_called() const noexcept -> bool { return wake_count_ > 0; }

  [[nodiscard]] auto clear_called() const noexcept -> bool { return clear_count_ > 0; }

  [[nodiscard]] auto init_count() const noexcept -> std::size_t { return init_count_; }

  [[nodiscard]] auto display_count() const noexcept -> std::size_t { return display_count_; }

  [[nodiscard]] auto sleep_count() const noexcept -> std::size_t { return sleep_count_; }

  [[nodiscard]] auto wake_count() const noexcept -> std::size_t { return wake_count_; }

  [[nodiscard]] auto clear_count() const noexcept -> std::size_t { return clear_count_; }

  [[nodiscard]] auto is_asleep() const noexcept -> bool { return is_asleep_; }

  [[nodiscard]] auto is_initialized() const noexcept -> bool { return initialized_; }

  [[nodiscard]] auto last_buffer() const noexcept -> const std::vector<std::byte> & { return last_buffer_; }

  auto reset_counts() noexcept -> void {
    init_count_ = 0;
    display_count_ = 0;
    sleep_count_ = 0;
    wake_count_ = 0;
    clear_count_ = 0;
  }

  auto set_output_directory(std::string_view dir) -> void {
    output_dir_ = dir;
    if (save_images_) {
      std::filesystem::create_directories(output_dir_);
    }
  }

  auto enable_image_saving(bool enable) -> void { save_images_ = enable; }

private:
  auto save_buffer_as_png(std::span<const std::byte> buffer) const -> void {
    // Ensure output directory exists
    std::filesystem::create_directories(output_dir_);

    // Generate time-stamped filename
    const auto now = std::chrono::system_clock::now();
    const auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    const auto filename = std::format("{}/frame_{:013d}.png", output_dir_, timestamp);

    // Convert framebuffer to RGB24 format for PNG
    auto render_width = width_;
    auto render_height = height_;

    if (!buffer.empty() && height_ > 0) {
      const auto bytes_per_row = buffer.size() / height_;
      const auto mode_bpp = bits_per_pixel(mode_);
      if (mode_bpp == 1 && bytes_per_row * height_ == buffer.size()) {
        render_width = bytes_per_row * 8;
      }
      if (mode_bpp == 2 && bytes_per_row * height_ == buffer.size()) {
        render_width = bytes_per_row * 4;
      }
    }
    std::vector<std::uint8_t> rgb_data;
    rgb_data.reserve(render_width * render_height * 3);

    for (std::size_t y = 0; y < render_height; ++y) {
      for (std::size_t x = 0; x < render_width; ++x) {
        const auto color = get_pixel_from_buffer(mode_, buffer, render_width, render_height, x, y);
        const auto rgb = color_to_rgb(color);
        rgb_data.push_back(rgb.r);
        rgb_data.push_back(rgb.g);
        rgb_data.push_back(rgb.b);
      }
    }

    // Write PNG file
    stbi_write_png(filename.c_str(), static_cast<int>(render_width), static_cast<int>(render_height), 3,
                   rgb_data.data(), static_cast<int>(render_width * 3));
  }

  // HAL hardware components
  Device::HalSpi spi_;
  Device::HalOutput cs_;
  Device::HalOutput dc_;
  Device::HalOutput rst_;
  Device::HalInput busy_;

  // Display configuration
  std::size_t width_;
  std::size_t height_;
  DisplayMode mode_{};
  bool initialized_{};
  bool is_asleep_{};

  // Failure configuration
  bool init_should_fail_{};
  bool display_should_fail_{};
  bool sleep_should_fail_{};
  bool wake_should_fail_{};

  // Call tracking
  std::size_t init_count_{};
  std::size_t display_count_{};
  std::size_t sleep_count_{};
  std::size_t wake_count_{};
  std::size_t clear_count_{};

  // Buffer storage for verification
  std::vector<std::byte> last_buffer_;

  // Image saving
  bool save_images_;
  std::string output_dir_;
};

template <> struct driver_traits<MockDriver> {
  static constexpr DisplayMode max_mode = DisplayMode::Spectra6;
  static constexpr bool supports_grayscale = true;
  static constexpr bool supports_partial_refresh = true;
  static constexpr bool supports_power_control = true;
  static constexpr bool supports_wake_from_sleep = true;
  static constexpr std::size_t max_width = 800;
  static constexpr std::size_t max_height = 600;
};

} // namespace epaper
