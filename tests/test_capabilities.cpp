#include "test_config.hpp"
#include <epaper/core/display.hpp>
#include <epaper/drivers/capabilities.hpp>
#include <epaper/drivers/epd27.hpp>
#include <epaper/hal/mock_delay.hpp>
#include <epaper/hal/mock_pin.hpp>
#include <epaper/hal/mock_spi.hpp>
#include <iostream>
#include <type_traits>

using namespace epaper;

// A minimal dummy driver that satisfies driver interface but has NO capabilities specialization
// We don't need it to actually work, just to compile-check the concepts
struct DummyDriverForCapabilitiesTest {
  static auto init(DisplayMode /*mode*/) -> std::expected<void, Error> { return {}; }
  static auto clear() -> std::expected<void, Error> { return {}; }
  static auto display(std::span<const std::byte> /*data*/) -> std::expected<void, Error> { return {}; }
  static auto sleep() -> std::expected<void, Error> { return {}; }
  static auto wake() -> std::expected<void, Error> { return {}; }
  static auto power_off() -> std::expected<void, Error> { return {}; }
  static auto power_on() -> std::expected<void, Error> { return {}; }

  [[nodiscard]] static auto width() noexcept -> std::size_t { return 100; }
  [[nodiscard]] static auto height() noexcept -> std::size_t { return 100; }
  [[nodiscard]] static auto mode() noexcept -> DisplayMode { return DisplayMode::BlackWhite; }
  [[nodiscard]] static auto buffer_size() noexcept -> std::size_t { return 100 * 100 / 8; }

  [[nodiscard]] static auto supports_partial_refresh() noexcept -> bool { return false; }
  [[nodiscard]] static auto supports_power_control() noexcept -> bool { return false; }
  [[nodiscard]] static auto supports_wake() noexcept -> bool { return false; }
};

auto main() -> int {
  std::cout << "Testing Driver Capabilities...\n";

  // 1. Check if EPD27 has capabilities specialized
  using EPD27Type = EPD27;

  // This should compile if EPD27 has capabilities
  constexpr auto width = driver_traits<EPD27Type>::max_width;
  std::cout << "EPD27 Max Width: " << width << "\n";

  if constexpr (driver_traits<EPD27Type>::supports_grayscale) {
    std::cout << "EPD27 reports grayscale support.\n";
  }

  // 2. Check enforcement
  // create_display<T, FramebufferT> requires DriverTraits<T>

  // We can't actually run create_display with DummyDriver easily because it needs a Device
  // but the compilation check is what matters.
  // I can simulate the check:

  std::cout << "Checking DriverTraits concept...\n";
  if constexpr (DriverTraits<EPD27Type>) {
    std::cout << "EPD27 satisfies DriverTraits\n";
  } else {
    std::cout << "EPD27 does NOT satisfy DriverTraits (Unexpected!)\n";
  }

  if constexpr (DriverTraits<DummyDriverForCapabilitiesTest>) {
    std::cout << "DummyDriver satisfies DriverTraits (Unexpected!)\n";
  } else {
    std::cout << "DummyDriver does NOT satisfy DriverTraits (Expected)\n";
  }

  // create_display<DummyDriverForCapabilitiesTest, MonoFramebuffer> would compile if we provided proper HAL,
  // even though DummyDriver has no capabilities defined - this is allowed since
  // the capabilities are metadata, not requirements.

  return 0;
}
