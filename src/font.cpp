#include "epaper/graphics/font.hpp"

// Include the original font data files from third_party
// These are C-style font tables in Waveshare format
extern "C" {
#include "third_party/fonts/fonts.h"
}

namespace epaper {

auto Font::char_data(char c) const -> std::span<const std::uint8_t> {
  // ASCII printable characters start at 0x20 (space character)
  // Font table stores only printable chars (0x20 to 0x7E = 95 characters)
  // Calculate offset from start of printable range
  const auto char_offset = static_cast<std::size_t>(static_cast<std::uint8_t>(c) - 0x20);

  // Each character occupies bytes_per_char() bytes in the font table
  // Formula: ceil(width/8) * height bytes per character
  const auto bytes = bytes_per_char();

  // Calculate absolute byte offset in font table
  // Table layout: [Char_0x20][Char_0x21]...[Char_0x7E] (sequential)
  const auto offset = char_offset * bytes;

  // Get pointer to start of this character's bitmap data
  // NOLINTNEXTLINE: Interfacing with C font data requires pointer arithmetic
  const auto *data_ptr = &table_[offset];

  // Return span covering this character's bitmap
  // Size = bytes_per_char() for valid chars
  // For out-of-range chars (offset >= table size), span may be invalid
  // but Graphics::draw_text() checks empty() before accessing
  return {data_ptr, bytes};
}

// Factory methods return references to static Font instances
// Static local variables are thread-safe in C++11+ (initialized once)
// Font data (table, width, height) comes from third_party/fonts/fonts.h

auto Font::font8() -> const Font & {
  // 8x8 pixel font - compact, good for dense text or small labels
  static const Font font(Font8.table, Font8.Width, Font8.Height);
  return font;
}

auto Font::font12() -> const Font & {
  // 12x12 pixel font - medium size, balanced readability
  static const Font font(Font12.table, Font12.Width, Font12.Height);
  return font;
}

auto Font::font16() -> const Font & {
  // 16x16 pixel font - large, good for headers or important text
  static const Font font(Font16.table, Font16.Width, Font16.Height);
  return font;
}

auto Font::font20() -> const Font & {
  // 20x20 pixel font - extra large, suitable for titles
  static const Font font(Font20.table, Font20.Width, Font20.Height);
  return font;
}

auto Font::font24() -> const Font & {
  // 24x24 pixel font - very large, display headers or emphasis
  static const Font font(Font24.table, Font24.Width, Font24.Height);
  return font;
}

} // namespace epaper
