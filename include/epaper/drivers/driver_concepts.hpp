#pragma once

/**
 * @file driver_concepts.hpp
 * @brief C++20 concept defining the Driver interface contract.
 *
 * Uses modern C++ concepts to specify the required interface for display drivers.
 * Provides compile-time validation that driver implementations satisfy the contract.
 *
 * **Design Philosophy:**
 * ```
 * Concept-Based Polymorphism (C++20)
 * ├─ Compile-Time Validation: Errors caught at instantiation, not runtime
 * ├─ Zero Overhead: No vtables, no dynamic dispatch
 * ├─ Duck Typing: If it walks like a driver and quacks like a driver...
 * └─ Template Constraints: Display<Driver> only compiles for valid drivers
 * ```
 *
 * **Benefits Over Virtual Interfaces:**
 * - No runtime cost (no vtable pointer, no virtual dispatch)
 * - Better inlining opportunities (all calls known at compile time)
 * - Stronger type safety (type mismatches caught at compile time)
 * - Clearer error messages (constraint violations are explicit)
 *
 * **Required Driver Interface:**
 * ```cpp
 * class MyDriver {
 * public:
 *   // Lifecycle (returns std::expected<void, Error>)
 *   auto init(DisplayMode mode) -> std::expected<void, Error>;
 *   auto sleep() -> std::expected<void, Error>;
 *   auto wake() -> std::expected<void, Error>;
 *
 *   // Drawing (returns std::expected<void, Error>)
 *   auto clear() -> std::expected<void, Error>;
 *   auto display(std::span<const std::byte> buffer) -> std::expected<void, Error>;
 *
 *   // Dimensions (const or static, returns size_t or convertible)
 *   auto width() const -> std::size_t;
 *   auto height() const -> std::size_t;
 * };
 * ```
 *
 * **Movability Requirement:**
 * - Driver must be `std::movable` (no manual resource management for move)
 * - Enables returning drivers from factory functions
 * - Move semantics used by Display to store driver instance
 *
 * **Validation Example:**
 * ```cpp
 * // Compile-time check
 * static_assert(Driver<EPD27>);         // OK - EPD27 satisfies concept
 * static_assert(Driver<MockDriver>);    // OK - MockDriver satisfies concept
 * // static_assert(Driver<int>);        // ERROR - int doesn't satisfy Driver
 *
 * // Use in template constraints
 * template <Driver D>
 * class Display { ... };  // Only accepts types satisfying Driver concept
 * ```
 *
 * @see Display, driver_traits, EPD27, MockDriver
 */

#include "epaper/core/errors.hpp"
#include "epaper/drivers/driver.hpp"
#include <concepts>
#include <cstddef>
#include <expected>
#include <span>

namespace epaper {

/**
 * @brief Concept defining the required interface for a Display Driver.
 *
 * A valid driver must provide lifecycle management (init, sleep, wake),
 * drawing operations (clear, display), and dimension queries (width, height).
 *
 * **Semantic Requirements (not enforced by concept):**
 * - `init(mode)`: Initialize hardware in specified display mode, reset state
 * - `sleep()`: Enter low-power sleep mode, preserve state if possible
 * - `wake()`: Wake from sleep, restore previous state or require re-init
 * - `clear()`: Clear display to default color (typically white/blank)
 * - `display(buffer)`: Transfer buffer to display hardware and refresh
 * - `width()`, `height()`: Return physical display dimensions in pixels
 *
 * **Error Handling:**
 * - All operations return `std::expected<void, Error>` for fallible operations
 * - Success: return `{}` (std::expected default-constructed value)
 * - Failure: return `std::unexpected{Error{...}}` with error details
 *
 * **Buffer Format:**
 * - `display(buffer)` expects buffer formatted for current mode:
 *   - BlackWhite: 1bpp MSB-first, (width*height+7)/8 bytes
 *   - Grayscale4: 2bpp, (width*height+3)/4 bytes
 *   - BWR/BWY: 2 planes, 2*(width*height+7)/8 bytes
 *   - Spectra6: 3bpp, (width*height*3+7)/8 bytes
 * - Buffer ownership: driver does not take ownership (span is borrowed)
 *
 * **Move Semantics:**
 * - Driver must be movable (std::movable) for factory functions
 * - Move leaves source in valid-but-unspecified state (can be destroyed)
 * - Typical move: transfer HAL resources (GPIO/SPI) to destination
 *
 * @tparam T The driver type to check
 *
 * @example
 * ```cpp
 * // Custom driver implementation
 * class MyCustomDriver {
 * public:
 *   auto init(DisplayMode mode) -> std::expected<void, Error> {
 *     // Initialize hardware
 *     return {};  // Success
 *   }
 *
 *   auto sleep() -> std::expected<void, Error> { return {}; }
 *   auto wake() -> std::expected<void, Error> { return {}; }
 *   auto clear() -> std::expected<void, Error> { return {}; }
 *
 *   auto display(std::span<const std::byte> buffer) -> std::expected<void, Error> {
 *     // Transfer buffer to hardware
 *     return {};
 *   }
 *
 *   auto width() const -> std::size_t { return 264; }
 *   auto height() const -> std::size_t { return 176; }
 * };
 *
 * static_assert(Driver<MyCustomDriver>);  // OK - satisfies concept
 *
 * // Use with Display
 * auto display = create_display<MyCustomDriver>(device, DisplayMode::BlackWhite);
 * ```
 *
 * @see Display, create_display(), driver_traits
 */
template <typename T>
concept Driver = std::movable<T> && requires(T &d, DisplayMode mode, std::span<const std::byte> buffer) {
  // Lifecycle
  { d.init(mode) } -> std::same_as<std::expected<void, Error>>;
  { d.sleep() } -> std::same_as<std::expected<void, Error>>;
  { d.wake() } -> std::same_as<std::expected<void, Error>>;

  // Drawing
  { d.clear() } -> std::same_as<std::expected<void, Error>>;
  { d.display(buffer) } -> std::same_as<std::expected<void, Error>>;

  // Dimensions (can be static or instance methods, but callable on instance)
  { d.width() } -> std::convertible_to<std::size_t>;
  { d.height() } -> std::convertible_to<std::size_t>;
};

} // namespace epaper
