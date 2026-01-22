#pragma once

/**
 * @file gpio.hpp
 * @brief GPIO pin concepts for hardware abstraction.
 *
 * Defines C++20 concepts for digital input/output pins, enabling
 * generic driver code that works with any GPIO implementation.
 *
 * **Design Philosophy:**
 * ```
 * Hardware Abstraction via Concepts
 * ├─ Platform Independence: Drivers work with any GPIO backend
 * ├─ Mockability: Easy to substitute mock pins for testing
 * ├─ Zero Overhead: Concepts compile to direct calls (no vtable)
 * └─ Type Safety: Compile-time validation of pin capabilities
 * ```
 *
 * **Concept Hierarchy:**
 * ```
 * DigitalOutput: write(bool) -> void
 * DigitalInput:  read() -> bool
 * ```
 *
 * **Typical Implementations:**
 * - **Production**: libgpiod v2 wrapper (Linux GPIO character device)
 * - **Testing**: MockPin (in-memory state, no hardware)
 * - **Embedded**: Platform-specific register writes (ARM, RISC-V)
 *
 * **Pin Semantics:**
 * - `write(true)`: Set pin to logic HIGH (typically 3.3V or 5V)
 * - `write(false)`: Set pin to logic LOW (typically 0V/GND)
 * - `read()`: Sample current pin state (true=HIGH, false=LOW)
 *
 * **No Buffering:**
 * - All operations are immediate (no internal buffering)
 * - `write()` may not guarantee signal stability until next call
 * - Debouncing/filtering must be done externally
 *
 * @example
 * ```cpp
 * // Using concept-constrained function
 * template <hal::DigitalOutput Pin>
 * void blink_led(Pin& led) {
 *   led.write(true);   // Turn on
 *   delay_ms(500);
 *   led.write(false);  // Turn off
 * }
 *
 * // Works with any implementation
 * RealGPIOPin gpio_pin{17};
 * MockPin mock_pin{};
 * blink_led(gpio_pin);  // OK - satisfies DigitalOutput
 * blink_led(mock_pin);  // OK - satisfies DigitalOutput
 * ```
 *
 * @see MockPin, Device (uses GPIO concepts)
 */

#include <concepts>

namespace epaper::hal {

/**
 * @brief Concept for a digital output pin.
 *
 * Represents a pin that can be written to (set high/low).
 *
 * **Required Operations:**
 * - `write(bool level)`: Set pin to HIGH (true) or LOW (false)
 *
 * **Method Signature:**
 * ```cpp
 * auto write(bool level) -> void;
 * ```
 *
 * **Semantic Requirements (not enforced by concept):**
 * - `write(true)` sets pin to logic HIGH (voltage >= VIH threshold)
 * - `write(false)` sets pin to logic LOW (voltage <= VIL threshold)
 * - No return value (errors handled by implementation, e.g., exceptions)
 * - Thread safety: implementation-defined (typically not thread-safe)
 *
 * **Typical Use Cases:**
 * - Control signals (CS, DC, RST for SPI displays)
 * - LED control
 * - Relay/transistor switching
 *
 * @tparam T Type to check for DigitalOutput conformance
 *
 * @example
 * ```cpp
 * template <hal::DigitalOutput Pin>
 * void set_chip_select(Pin& cs, bool enable) {
 *   cs.write(!enable);  // Active-low CS (typical for SPI)
 * }
 * ```
 *
 * @see DigitalInput, MockPin
 */
template <typename T>
concept DigitalOutput = requires(T out, bool level) {
  { out.write(level) } -> std::same_as<void>;
};

/**
 * @brief Concept for a digital input pin.
 *
 * Represents a pin that can be read from (sample high/low state).
 *
 * **Required Operations:**
 * - `read()`: Sample current pin logic level
 *
 * **Method Signature:**
 * ```cpp
 * auto read() const -> bool;
 * ```
 *
 * **Semantic Requirements (not enforced by concept):**
 * - Returns `true` if pin voltage >= VIH (logic HIGH)
 * - Returns `false` if pin voltage <= VIL (logic LOW)
 * - Undefined behavior if voltage in dead zone (VIL < V < VIH)
 * - No debouncing (raw pin state, may glitch)
 * - Thread safety: implementation-defined
 *
 * **Typical Use Cases:**
 * - Busy signal detection (e.g., display BUSY pin)
 * - Button/switch input
 * - Interrupt source monitoring
 *
 * @tparam T Type to check for DigitalInput conformance
 *
 * @example
 * ```cpp
 * template <hal::DigitalInput Pin>
 * void wait_for_ready(Pin& busy_pin) {
 *   while (busy_pin.read()) {
 *     // Busy-wait until pin goes LOW
 *   }
 * }
 * ```
 *
 * @see DigitalOutput, MockPin
 */
template <typename T>
concept DigitalInput = requires(T in) {
  { in.read() } -> std::same_as<bool>;
};

} // namespace epaper::hal
