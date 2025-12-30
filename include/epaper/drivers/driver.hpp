#pragma once

#include "epaper/errors.hpp"
#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <string_view>

namespace epaper {

// Display modes
enum class DisplayMode {
  BlackWhite, // 1-bit black and white
  Grayscale4  // 2-bit 4-level grayscale
};

/**
 * @brief Abstract driver interface for e-paper displays.
 *
 * Defines the interface that all e-paper display drivers must implement.
 * Provides hardware abstraction for different display models.
 *
 * @note Exception Safety: All virtual methods provide basic exception safety.
 *       Derived classes must maintain object validity even if operations fail.
 */
class Driver {
public:
  virtual ~Driver() = default;

  /**
   * @brief Initialize the display with specified mode.
   *
   * @param mode Display mode (BlackWhite or Grayscale4)
   * @return void on success, Error on failure
   * @note Exception Safety: Strong guarantee - display remains uninitialized on failure.
   */
  [[nodiscard]] virtual auto init(DisplayMode mode) -> std::expected<void, Error> = 0;

  /**
   * @brief Clear the display (typically to white).
   *
   * @note Exception Safety: Basic guarantee - display remains in valid state.
   *       This operation is benign and does not fail.
   */
  virtual auto clear() -> void = 0;

  /**
   * @brief Send buffer data to display and refresh.
   *
   * @param buffer Framebuffer data to display
   * @return void on success, Error on failure
   * @note Exception Safety: Basic guarantee - display remains in valid state.
   */
  [[nodiscard]] virtual auto display(std::span<const std::byte> buffer) -> std::expected<void, Error> = 0;

  /**
   * @brief Put display into low-power sleep mode.
   *
   * @note Exception Safety: Basic guarantee - display remains in valid state.
   *       This operation is benign and does not fail.
   */
  virtual auto sleep() -> void = 0;

  /**
   * @brief Wake display from sleep mode.
   *
   * @return void on success, Error if wake is not supported or fails
   * @note Exception Safety: Basic guarantee - display remains in valid state.
   */
  [[nodiscard]] virtual auto wake() -> std::expected<void, Error> = 0;

  /**
   * @brief Turn display power completely off (hardware power down).
   *
   * @return void on success, Error if power off is not supported or fails
   * @note Exception Safety: Basic guarantee - display remains in valid state.
   */
  [[nodiscard]] virtual auto power_off() -> std::expected<void, Error> = 0;

  /**
   * @brief Turn display power on (hardware power up).
   *
   * @return void on success, Error if power on is not supported or fails
   * @note Exception Safety: Basic guarantee - display remains in valid state.
   */
  [[nodiscard]] virtual auto power_on() -> std::expected<void, Error> = 0;

  // Get display dimensions
  [[nodiscard]] virtual auto width() const noexcept -> std::size_t = 0;
  [[nodiscard]] virtual auto height() const noexcept -> std::size_t = 0;

  // Get current display mode
  [[nodiscard]] virtual auto mode() const noexcept -> DisplayMode = 0;

  // Calculate required buffer size for current mode
  [[nodiscard]] virtual auto buffer_size() const noexcept -> std::size_t = 0;

  // Driver capabilities (query at runtime)
  [[nodiscard]] virtual auto supports_partial_refresh() const noexcept -> bool = 0;
  [[nodiscard]] virtual auto supports_wake() const noexcept -> bool = 0;
  [[nodiscard]] virtual auto supports_power_control() const noexcept -> bool = 0;

protected:
  Driver() = default;

  // Non-copyable, movable
  Driver(const Driver &) = delete;
  Driver &operator=(const Driver &) = delete;
  Driver(Driver &&) noexcept = default;
  Driver &operator=(Driver &&) noexcept = default;
};

} // namespace epaper
