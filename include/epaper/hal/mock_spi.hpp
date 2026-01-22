#pragma once

#include "epaper/hal/spi.hpp"
#include <cstddef>
#include <cstdint>
#include <deque>
#include <span>
#include <vector>

namespace epaper::hal {

/**
 * @brief Mock SPI bus for testing.
 *
 * Records all transfers and allows configuring responses.
 * Satisfies hal::SpiBus concept without requiring hardware.
 *
 * @example
 * @code{.cpp}
 * MockSpiBus spi;
 * spi.queue_response(0xAA);
 * auto result = spi.transfer(0x55);
 * assert(result == 0xAA);
 * assert(spi.sent_bytes()[0] == 0x55);
 * @endcode
 */
class MockSpiBus {
public:
  /**
   * @brief Full-duplex byte transfer.
   *
   * Records sent byte and returns queued response (or 0x00 if queue empty).
   *
   * @param byte Byte to send
   * @return Response byte from queue, or 0x00
   */
  auto transfer(std::uint8_t byte) -> std::uint8_t {
    sent_bytes_.push_back(byte);
    if (!response_queue_.empty()) {
      auto response = response_queue_.front();
      response_queue_.pop_front();
      return response;
    }
    return 0x00; // Default response
  }

  /**
   * @brief Bulk write operation.
   *
   * Records all sent bytes.
   *
   * @param data Span of bytes to write
   */
  auto write(std::span<const std::byte> data) -> void {
    for (auto byte : data) {
      sent_bytes_.push_back(static_cast<std::uint8_t>(byte));
    }
  }

  /**
   * @brief Get vector of all bytes sent via transfer() and write().
   * @return Reference to sent bytes vector
   */
  [[nodiscard]] auto sent_bytes() const -> const std::vector<std::uint8_t> & { return sent_bytes_; }

  /**
   * @brief Get total number of bytes transferred.
   * @return Byte count
   */
  [[nodiscard]] auto transfer_count() const -> std::size_t { return sent_bytes_.size(); }

  /**
   * @brief Queue a response byte for next transfer() call.
   * @param byte Byte to return on next transfer()
   */
  auto queue_response(std::uint8_t byte) -> void { response_queue_.push_back(byte); }

  /**
   * @brief Reset bus state for new test.
   */
  auto reset() -> void {
    sent_bytes_.clear();
    response_queue_.clear();
  }

private:
  std::vector<std::uint8_t> sent_bytes_;
  std::deque<std::uint8_t> response_queue_;
};

static_assert(SpiBus<MockSpiBus>, "MockSpiBus must satisfy SpiBus concept");

} // namespace epaper::hal
