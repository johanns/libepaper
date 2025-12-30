#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace epaper {

// Font metrics
struct FontMetrics {
  std::uint16_t width;
  std::uint16_t height;
};

// Font class - wraps Waveshare font data
class Font {
public:
  // Create font from raw data (table pointer, width, height)
  constexpr Font(const std::uint8_t *table, std::uint16_t width, std::uint16_t height)
      : table_(table), width_(width), height_(height) {}

  // Get font metrics
  [[nodiscard]] constexpr auto metrics() const noexcept -> FontMetrics { return {width_, height_}; }

  [[nodiscard]] constexpr auto width() const noexcept -> std::uint16_t { return width_; }
  [[nodiscard]] constexpr auto height() const noexcept -> std::uint16_t { return height_; }

  // Get character bitmap data
  [[nodiscard]] auto char_data(char c) const -> std::span<const std::uint8_t>;

  // Calculate bytes per character
  [[nodiscard]] constexpr auto bytes_per_char() const noexcept -> std::size_t {
    const auto width_bytes = static_cast<std::size_t>((width_ % 8 == 0) ? (width_ / 8) : (width_ / 8 + 1));
    return width_bytes * static_cast<std::size_t>(height_);
  }

  // Factory methods for built-in fonts
  static auto font8() -> const Font &;
  static auto font12() -> const Font &;
  static auto font16() -> const Font &;
  static auto font20() -> const Font &;
  static auto font24() -> const Font &;

private:
  const std::uint8_t *table_;
  std::uint16_t width_;
  std::uint16_t height_;
};

} // namespace epaper
