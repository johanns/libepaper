#pragma once

#include <cstddef>
#include <cstdint>

namespace epaper::hal {

/**
 * @brief Mock delay policy for testing.
 *
 * Tracks delay calls without actually blocking execution.
 * Useful for verifying timing behavior in tests without waiting.
 *
 * @example
 * @code{.cpp}
 * MockDelay::reset();
 * MockDelay::delay_ms(100);
 * MockDelay::delay_ms(50);
 * assert(MockDelay::total_delay_ms() == 150);
 * assert(MockDelay::delay_count() == 2);
 * @endcode
 */
class MockDelay {
public:
  /**
   * @brief Mock millisecond delay (records but doesn't block).
   *
   * @param ms Milliseconds to delay
   */
  static auto delay_ms(std::uint32_t ms) -> void {
    total_delay_ms_ += ms;
    delay_count_++;
  }

  /**
   * @brief Get total accumulated delay time.
   * @return Total milliseconds across all delay_ms() calls
   */
  [[nodiscard]] static auto total_delay_ms() -> std::uint32_t { return total_delay_ms_; }

  /**
   * @brief Get total number of delay calls.
   * @return Delay call count
   */
  [[nodiscard]] static auto delay_count() -> std::size_t { return delay_count_; }

  /**
   * @brief Reset delay tracking for new test.
   */
  static auto reset() -> void {
    total_delay_ms_ = 0;
    delay_count_ = 0;
  }

private:
  inline static std::uint32_t total_delay_ms_ = 0;
  inline static std::size_t delay_count_ = 0;
};

} // namespace epaper::hal
