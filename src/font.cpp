#include "epaper/font.hpp"

// Include the original font data files
extern "C" {
#include "../fonts/fonts.h"
}

namespace epaper {

auto Font::char_data(char c) const -> std::span<const std::uint8_t> {
  // ASCII characters start at 0x20 (space)
  const auto char_offset = static_cast<std::size_t>(static_cast<std::uint8_t>(c) - 0x20);
  const auto bytes = bytes_per_char();
  const auto offset = char_offset * bytes;
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic) - Interfacing with C font data
  const auto *data_ptr = &table_[offset];

  return {data_ptr, bytes};
}

auto Font::font8() -> const Font & {
  static const Font font(Font8.table, Font8.Width, Font8.Height);
  return font;
}

auto Font::font12() -> const Font & {
  static const Font font(Font12.table, Font12.Width, Font12.Height);
  return font;
}

auto Font::font16() -> const Font & {
  static const Font font(Font16.table, Font16.Width, Font16.Height);
  return font;
}

auto Font::font20() -> const Font & {
  static const Font font(Font20.table, Font20.Width, Font20.Height);
  return font;
}

auto Font::font24() -> const Font & {
  static const Font font(Font24.table, Font24.Width, Font24.Height);
  return font;
}

} // namespace epaper
