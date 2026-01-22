#pragma once

/**
 * @file font.hpp
 * @brief Bitmap font rendering for e-paper displays.
 *
 * Provides fixed-width bitmap font support using Waveshare font format.
 * Fonts are stored as packed bitmap arrays with MSB-first encoding.
 *
 * **Font Format Specification:**
 * ```
 * Waveshare Bitmap Font Format
 * ├─ Fixed-width: All characters same dimensions (width × height)
 * ├─ ASCII Range: 0x20 (space) to 0x7E (tilde) = 95 printable characters
 * ├─ Bitmap Layout: Row-major, MSB-first bit packing
 * └─ No Kerning: Characters placed at fixed intervals
 * ```
 *
 * **Bitmap Encoding:**
 * ```
 * Character 'A' (8×12 font):
 * Row 0: [76543210]  ← MSB is leftmost pixel
 * Row 1: [76543210]
 * ...
 * Row 11: [76543210]
 *
 * Total bytes = ceil(width/8) × height
 * Example: 8×12 → 1 byte/row × 12 rows = 12 bytes
 *          12×16 → 2 bytes/row × 16 rows = 32 bytes
 * ```
 *
 * **Memory Layout:**
 * ```
 * Font table array:
 * [Char_0x20][Char_0x21][Char_0x22]...[Char_0x7E]
 *    ↓          ↓          ↓              ↓
 *  Space       '!'        '"'           '~'
 *
 * Each character occupies bytes_per_char() bytes sequentially.
 * ```
 *
 * **Built-in Fonts:**
 * - font8():  8×8 pixels  - Small, compact (terminals, labels)
 * - font12(): 12×12 pixels - Medium (body text)
 * - font16(): 16×16 pixels - Large (headers, important text)
 * - font20(): 20×20 pixels - Extra large (titles)
 * - font24(): 24×24 pixels - Display headers
 *
 * **Usage Patterns:**
 * ```cpp
 * // Using built-in fonts
 * const Font& f = Font::font16();
 * display.draw(display.text("Hello").at({10,10}).font(&f).build());
 *
 * // Custom font from external data
 * extern const uint8_t my_font_data[];
 * Font custom_font{my_font_data, 16, 20};  // 16px wide, 20px tall
 * ```
 *
 * **Text Rendering Process:**
 * 1. TextCommand specifies string and font
 * 2. Graphics::draw_text() iterates characters
 * 3. Font::char_data() retrieves bitmap for each char
 * 4. Bitmap bits decoded MSB-first, drawn as pixels
 *
 * **Limitations:**
 * - No variable-width fonts (monospaced only)
 * - No Unicode support (ASCII 0x20-0x7E only)
 * - No anti-aliasing (1-bit bitmaps)
 * - No font scaling (use different size fonts)
 *
 * @see Graphics::draw_text(), TextCommand, FontMetrics
 */

#include <cstddef>
#include <cstdint>
#include <span>

namespace epaper {

/**
 * @brief Font dimension metrics.
 *
 * Describes the bounding box dimensions for all characters in a font.
 * Since fonts are fixed-width, all characters share these dimensions.
 *
 * **Member Semantics:**
 * - `width`: Character cell width in pixels (horizontal advance)
 * - `height`: Character cell height in pixels (vertical extent)
 *
 * **Usage:**
 * ```cpp
 * const Font& font = Font::font16();
 * auto metrics = font.metrics();
 * std::size_t text_width = metrics.width * text.length();
 * std::size_t text_height = metrics.height;
 * ```
 */
struct FontMetrics {
  std::uint16_t width;
  std::uint16_t height;
};

/**
 * @brief Bitmap font wrapper.
 *
 * Encapsulates raw font data and provides access to character bitmaps.
 * Supports fixed-width bitmap fonts in Waveshare format.
 *
 * **Design:**
 * - Value type: Cheap to copy (just pointer + dimensions)
 * - Non-owning: Font data must outlive Font instance (typically static)
 * - Thread-safe: Immutable after construction (const methods only)
 *
 * **Character Addressing:**
 * - ASCII printable range: 0x20 (space) to 0x7E (~) = 95 characters
 * - Invalid characters (< 0x20 or > 0x7E) return empty span
 * - Offset calculation: (ascii_code - 0x20) × bytes_per_char()
 *
 * **Factory Methods:**
 * - font8(), font12(), font16(), font20(), font24(): Built-in fonts
 * - Return const references to static instances (zero allocation)
 * - Thread-safe initialization (C++11 static local guarantee)
 *
 * @example
 * ```cpp
 * // Using built-in font
 * const Font& font = Font::font16();
 * std::cout << "Font size: " << font.width() << "x" << font.height() << std::endl;
 *
 * // Accessing character bitmap
 * auto bitmap = font.char_data('A');
 * std::cout << "'A' bitmap size: " << bitmap.size() << " bytes" << std::endl;
 *
 * // Custom font from external data
 * extern const std::uint8_t custom_font_table[];
 * Font custom{custom_font_table, 12, 16};
 * ```
 *
 * @see FontMetrics, Graphics::draw_text()
 */
class Font {
public:
  /**
   * @brief Construct a font from raw bitmap data.
   *
   * **Lifetime Requirements:**
   * - `table` pointer must remain valid for Font lifetime
   * - Typically `table` points to static or embedded font data
   * - Font does NOT take ownership (no allocation/deallocation)
   *
   * **Data Format:**
   * - MSB-first bitmap encoding
   * - Row-major layout (rows within character, characters sequential)
   * - Size: 95 characters × bytes_per_char()
   *
   * @param table Pointer to raw font data array (must outlive Font)
   * @param width Character width in pixels (1-255)
   * @param height Character height in pixels (1-255)
   *
   * @warning Undefined behavior if table is null or too small.
   *          No bounds checking performed.
   *
   * @example
   * ```cpp
   * // Static font data (guaranteed lifetime)
   * static const uint8_t my_font[] = { ... };  // bitmap data
   * Font font{my_font, 16, 16};
   *
   * // INCORRECT: temporary data
   * // std::vector<uint8_t> temp_data = load_font();
   * // Font bad_font{temp_data.data(), 16, 16};  // DANGER: temp_data destroyed
   * ```
   */
  constexpr Font(const std::uint8_t *table, std::uint16_t width, std::uint16_t height)
      : table_(table), width_(width), height_(height) {}

  /**
   * @brief Get font dimensions.
   * @return FontMetrics structure
   */
  [[nodiscard]] constexpr auto metrics() const noexcept -> FontMetrics { return {.width = width_, .height = height_}; }

  /**
   * @brief Get character width in pixels.
   * @return Width
   */
  [[nodiscard]] constexpr auto width() const noexcept -> std::uint16_t { return width_; }

  /**
   * @brief Get character height in pixels.
   * @return Height
   */
  [[nodiscard]] constexpr auto height() const noexcept -> std::uint16_t { return height_; }

  /**
   * @brief Get bitmap data for a specific character.
   *
   * Returns a span containing the packed bitmap bytes for the requested
   * character. Bitmap is MSB-first, row-major encoded.
   *
   * **Supported Range:**
   * - Printable ASCII: 0x20 (space) to 0x7E (tilde)
   * - Control characters (< 0x20) return empty span
   * - Extended ASCII (> 0x7E) return empty span
   *
   * **Bitmap Layout:**
   * ```
   * For 12px wide font:
   * [Row 0 byte 0][Row 0 byte 1]  ← First row (MSB-first)
   * [Row 1 byte 0][Row 1 byte 1]  ← Second row
   * ...
   * [Row N byte 0][Row N byte 1]  ← Last row
   * ```
   *
   * **Decoding:**
   * ```cpp
   * auto bitmap = font.char_data('A');
   * for (size_t row = 0; row < font.height(); ++row) {
   *   for (size_t col = 0; col < font.width(); ++col) {
   *     size_t byte_idx = row * width_bytes + col / 8;
   *     bool pixel = (bitmap[byte_idx] & (0x80 >> (col % 8))) != 0;
   *     // pixel: true = foreground, false = background
   *   }
   * }
   * ```
   *
   * @param c Character to retrieve (ASCII 0x20-0x7E)
   * @return Span containing bitmap bytes (size = bytes_per_char())
   *         Returns empty span for unsupported characters
   *
   * @see bytes_per_char(), Graphics::draw_text()
   */
  [[nodiscard]] auto char_data(char c) const -> std::span<const std::uint8_t>;

  /**
   * @brief Calculate storage size for a single character.
   *
   * Computes the number of bytes needed to store one character's bitmap
   * based on font dimensions.
   *
   * **Formula:**
   * ```
   * width_bytes = ceil(width / 8)
   * bytes_per_char = width_bytes × height
   * ```
   *
   * **Examples:**
   * - 8×8 font: 1 × 8 = 8 bytes
   * - 12×12 font: 2 × 12 = 24 bytes
   * - 16×16 font: 2 × 16 = 32 bytes
   * - 24×24 font: 3 × 24 = 72 bytes
   *
   * **Total Font Size:**
   * ```
   * total_bytes = 95 × bytes_per_char()  // 95 ASCII printable chars
   * ```
   *
   * @return Number of bytes per character
   */
  [[nodiscard]] constexpr auto bytes_per_char() const noexcept -> std::size_t {
    const auto width_bytes = static_cast<std::size_t>((width_ % 8 == 0) ? (width_ / 8) : ((width_ / 8) + 1));
    return width_bytes * static_cast<std::size_t>(height_);
  }

  // Factory methods for built-in fonts
  /**
   * @brief Get 8-pixel font.
   * @return Font reference
   */
  static auto font8() -> const Font &;

  /**
   * @brief Get 12-pixel font.
   * @return Font reference
   */
  static auto font12() -> const Font &;

  /**
   * @brief Get 16-pixel font.
   * @return Font reference
   */
  static auto font16() -> const Font &;

  /**
   * @brief Get 20-pixel font.
   * @return Font reference
   */
  static auto font20() -> const Font &;

  /**
   * @brief Get 24-pixel font.
   * @return Font reference
   */
  static auto font24() -> const Font &;

private:
  const std::uint8_t *table_;
  std::uint16_t width_;
  std::uint16_t height_;
};

} // namespace epaper
