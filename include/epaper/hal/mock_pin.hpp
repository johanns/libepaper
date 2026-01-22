#pragma once

#include "epaper/hal/gpio.hpp"
#include <cstddef>

namespace epaper::hal {

/**
 * @brief Mock digital output pin for testing.
 *
 * Records all write operations for verification in tests.
 * Satisfies hal::DigitalOutput concept without requiring hardware.
 *
 * @example
 * @code{.cpp}
 * MockOutputPin pin;
 * pin.write(true);
 * assert(pin.last_level() == true);
 * assert(pin.write_count() == 1);
 * @endcode
 */
class MockOutputPin {
public:
  /**
   * @brief Write logic level to pin (records operation).
   *
   * @param level Logic level (true=HIGH, false=LOW)
   */
  auto write(bool level) -> void {
    last_level_ = level;
    write_count_++;
  }

  /**
   * @brief Get the last written logic level.
   * @return Last level written
   */
  [[nodiscard]] auto last_level() const -> bool { return last_level_; }

  /**
   * @brief Get total number of write operations.
   * @return Write count
   */
  [[nodiscard]] auto write_count() const -> std::size_t { return write_count_; }

  /**
   * @brief Reset pin state for new test.
   */
  auto reset() -> void {
    write_count_ = 0;
    last_level_ = false;
  }

private:
  bool last_level_ = false;
  std::size_t write_count_ = 0;
};

static_assert(DigitalOutput<MockOutputPin>, "MockOutputPin must satisfy DigitalOutput concept");

/**
 * @brief Mock digital input pin for testing.
 *
 * Allows setting return value for read() operations.
 * Satisfies hal::DigitalInput concept without requiring hardware.
 *
 * @example
 * @code{.cpp}
 * MockInputPin pin;
 * pin.set_level(true);
 * assert(pin.read() == true);
 * assert(pin.read_count() == 1);
 * @endcode
 */
class MockInputPin {
public:
  /**
   * @brief Read logic level from pin (returns configured level).
   * @return Configured logic level
   */
  auto read() -> bool {
    read_count_++;
    return level_;
  }

  /**
   * @brief Set the level that read() will return.
   * @param level Logic level to return on read()
   */
  auto set_level(bool level) -> void { level_ = level; }

  /**
   * @brief Get total number of read operations.
   * @return Read count
   */
  [[nodiscard]] auto read_count() const -> std::size_t { return read_count_; }

  /**
   * @brief Reset pin state for new test.
   */
  auto reset() -> void {
    read_count_ = 0;
    level_ = false;
  }

private:
  bool level_ = false;
  std::size_t read_count_ = 0;
};

static_assert(DigitalInput<MockInputPin>, "MockInputPin must satisfy DigitalInput concept");

} // namespace epaper::hal
