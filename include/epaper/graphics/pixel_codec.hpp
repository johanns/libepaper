#pragma once

/**
 * @file pixel_codec.hpp
 * @brief Centralized pixel encoding/decoding utilities for framebuffers.
 *
 * Provides all the bit manipulation logic for reading and writing pixels
 * in various display modes. This eliminates duplication between Display
 * and MockDriver while documenting the exact framebuffer format for each mode.
 *
 * **Supported Display Modes:**
 * - **BlackWhite**: 1 bit per pixel, MSB-first, 8 pixels/byte
 * - **Grayscale4**: 2 bits per pixel, 4 levels (White/Gray1/Gray2/Black), 4 pixels/byte
 * - **BWR/BWY**: Dual-plane, 1 bit per pixel per plane (BW + Color), active-low color
 * - **Spectra6**: 3 bits per pixel, 6 colors, cross-byte packing
 *
 * **Design Principles:**
 * - Pure functions: No side effects, all constexpr where possible
 * - Type safety: std::span<std::byte> instead of raw pointers
 * - Bounds checking: All functions validate buffer sizes
 * - Integer-only: No floating point in pixel operations
 * - Fallback values: Return safe defaults (White) on error
 *
 * **Bit Layout Examples:**
 * ```
 * Black/White (1bpp MSB-first):
 *   Byte: [p0 p1 p2 p3 p4 p5 p6 p7]
 *   p0 = MSB (bit 7), p7 = LSB (bit 0)
 *   1 = White, 0 = Black
 *
 * Grayscale4 (2bpp):
 *   Byte: [p0_hi p0_lo | p1_hi p1_lo | p2_hi p2_lo | p3_hi p3_lo]
 *   00 = Black, 01 = Gray2, 10 = Gray1, 11 = White
 *
 * BWR/BWY (dual-plane 1bpp):
 *   Plane 0 (BW): [b0 b1 b2 ... ] (1=White, 0=Black)
 *   Plane 1 (Color): [c0 c1 c2 ... ] (0=Color, 1=Transparent) [active-low]
 *   Color pixel = (BW=0, Color=0), White = (BW=1, Color=1)
 *
 * Spectra6 (3bpp cross-byte):
 *   3 pixels = 9 bits = 1 byte + 1 bit:
 *   [p0_b2 p0_b1 p0_b0 | p1_b2 p1_b1 p1_b0 | p2_b2 p2_b1 p2_b0 ...]
 *   Values: 0=Black, 1=White, 2=Red, 3=Yellow, 4=Blue, 5=Green
 * ```
 *
 * @see DisplayMode, Color, MonoFramebuffer, MultiPlaneFramebuffer
 */

#include "epaper/color/color.hpp" // For RGB struct? If RGB is used.
#include "epaper/core/types.hpp"
#include "epaper/drivers/driver.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace epaper {

// ============================================================================
// Constants for bit manipulation
// ============================================================================

namespace pixel_constants {

/// @name Black/White Mode Constants
/// @{
constexpr std::uint8_t BW_PIXELS_PER_BYTE = 8;
constexpr std::uint8_t BW_MSB_MASK = 0x80;
/// @}

/// @name Grayscale4 Mode Constants
/// @{
constexpr std::uint8_t GRAY_PIXELS_PER_BYTE = 4;
constexpr std::uint8_t GRAY_BITS_PER_PIXEL = 2;
constexpr std::uint8_t GRAY_PIXEL_MASK = 0xC0;
/// @}

/// @name Spectra6 Mode Constants
/// @{
constexpr std::uint8_t SPECTRA6_BITS_PER_PIXEL = 3;
constexpr std::uint8_t SPECTRA6_COLOR_MASK = 0x07;
/// @}

/// @name Grayscale Thresholds (for RGB conversion)
/// @{
constexpr std::uint8_t GRAY_THRESHOLD_WHITE = 192;
constexpr std::uint8_t GRAY_THRESHOLD_LIGHT = 128;
constexpr std::uint8_t GRAY_THRESHOLD_DARK = 64;
/// @}

} // namespace pixel_constants

// ============================================================================
// Spectra6 Color Mapping
// ============================================================================

/// Mapping from 3-bit Spectra6 value to Color enum
constexpr std::array<Color, 8> SPECTRA6_VALUE_TO_COLOR = {{
    Color::Black,  // 0
    Color::White,  // 1
    Color::Red,    // 2
    Color::Yellow, // 3
    Color::Blue,   // 4
    Color::Green,  // 5
    Color::Black,  // 6 (undefined, fallback)
    Color::Black   // 7 (undefined, fallback)
}};

/**
 * @brief Convert a Color to its 3-bit Spectra6 value.
 *
 * @param color The color to convert
 * @return 3-bit value (0-5) for Spectra6 encoding
 */
[[nodiscard]] constexpr auto spectra6_color_to_value(Color color) noexcept -> std::uint8_t {
  switch (color) {
  case Color::Black:
    return 0;
  case Color::White:
    return 1;
  case Color::Red:
    return 2;
  case Color::Yellow:
    return 3;
  case Color::Blue:
    return 4;
  case Color::Green:
    return 5;
  default:
    return 0; // Fallback to black
  }
}

/**
 * @brief Convert a 3-bit Spectra6 value to Color.
 *
 * @param value 3-bit value (0-7)
 * @return Corresponding Color enum value
 */
[[nodiscard]] constexpr auto spectra6_value_to_color(std::uint8_t value) noexcept -> Color {
  return SPECTRA6_VALUE_TO_COLOR.at(value & pixel_constants::SPECTRA6_COLOR_MASK);
}

// ============================================================================
// Color <-> RGB Conversion
// ============================================================================

/**
 * @brief Convert a Color enum to RGB values.
 *
 * @param color The e-paper color
 * @return RGB struct with red, green, blue components
 */
[[nodiscard]] constexpr auto color_to_rgb(Color color) noexcept -> RGB {
  switch (color) {
  case Color::White:
    return {255, 255, 255};
  case Color::Gray1:
    return {170, 170, 170};
  case Color::Gray2:
    return {85, 85, 85};
  case Color::Black:
    return {0, 0, 0};
  case Color::Red:
    return {255, 0, 0};
  case Color::Yellow:
    return {255, 255, 0};
  case Color::Blue:
    return {0, 0, 255};
  case Color::Green:
    return {0, 255, 0};
  }
  return {0, 0, 0}; // Fallback to black
}

/**
 * @brief Convert RGB values to grayscale using standard luminance formula.
 *
 * Uses the ITU-R BT.601 luma formula: Y = 0.299R + 0.587G + 0.114B
 *
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @return Grayscale value (0-255)
 */
[[nodiscard]] constexpr auto rgb_to_grayscale(std::uint8_t r, std::uint8_t g, std::uint8_t b) noexcept -> std::uint8_t {
  // Use integer arithmetic to avoid floating point
  // Formula: (299*R + 587*G + 114*B) / 1000
  return static_cast<std::uint8_t>((299U * r + 587U * g + 114U * b) / 1000U);
}

/**
 * @brief Convert RGB to Color for black/white mode.
 *
 * Uses threshold of 128 for binarization.
 *
 * @param r Red component
 * @param g Green component
 * @param b Blue component
 * @return Color::White or Color::Black
 */
[[nodiscard]] constexpr auto rgb_to_color_bw(std::uint8_t r, std::uint8_t g, std::uint8_t b) noexcept -> Color {
  const auto gray = rgb_to_grayscale(r, g, b);
  return gray >= pixel_constants::GRAY_THRESHOLD_LIGHT ? Color::White : Color::Black;
}

/**
 * @brief Convert RGB to Color for 4-level grayscale mode.
 *
 * Quantizes to 4 levels: White, Gray1, Gray2, Black.
 *
 * @param r Red component
 * @param g Green component
 * @param b Blue component
 * @return One of the four grayscale Color values
 */
[[nodiscard]] constexpr auto rgb_to_color_grayscale4(std::uint8_t r, std::uint8_t g, std::uint8_t b) noexcept -> Color {
  const auto gray = rgb_to_grayscale(r, g, b);
  if (gray >= pixel_constants::GRAY_THRESHOLD_WHITE) {
    return Color::White;
  }
  if (gray >= pixel_constants::GRAY_THRESHOLD_LIGHT) {
    return Color::Gray1;
  }
  if (gray >= pixel_constants::GRAY_THRESHOLD_DARK) {
    return Color::Gray2;
  }
  return Color::Black;
}

/**
 * @brief Convert RGB to Color based on display mode.
 *
 * @param mode Current display mode
 * @param r Red component
 * @param g Green component
 * @param b Blue component
 * @return Appropriate Color for the display mode
 */
[[nodiscard]] constexpr auto rgb_to_color(DisplayMode mode, std::uint8_t r, std::uint8_t g, std::uint8_t b) noexcept
    -> Color {
  switch (mode) {
  case DisplayMode::BlackWhite:
    return rgb_to_color_bw(r, g, b);
  case DisplayMode::Grayscale4:
    return rgb_to_color_grayscale4(r, g, b);
  default:
    // For color modes, use grayscale conversion as fallback
    return rgb_to_color_grayscale4(r, g, b);
  }
}

// ============================================================================
// Framebuffer Position Calculations
// ============================================================================

/**
 * @brief Calculate byte index and bit mask for B/W mode pixel.
 *
 * **Encoding:**
 * - 8 pixels per byte, MSB-first (leftmost pixel = bit 7)
 * - Row-major layout: pixels packed left-to-right, top-to-bottom
 * - Partial bytes: right-padded with zeros if width not multiple of 8
 *
 * **Example (width=10):**
 * ```
 * Pixels:  [0][1][2][3][4][5][6][7] | [8][9][pad][pad]...
 * Byte 0:   b7 b6 b5 b4 b3 b2 b1 b0  | Byte 1: b7 b6 ...
 * ```
 *
 * @param width Display width in pixels
 * @param x X coordinate (0-based, left edge = 0)
 * @param y Y coordinate (0-based, top edge = 0)
 * @return Pair of (byte_index, bit_mask) where bit_mask has single bit set
 */
[[nodiscard]] constexpr auto calculate_bw_position(std::size_t width, std::size_t x, std::size_t y) noexcept
    -> std::pair<std::size_t, std::uint8_t> {
  const auto width_bytes = (width + 7) / 8;
  const auto byte_index = (x / 8) + (y * width_bytes);
  const auto bit_offset = static_cast<std::uint8_t>(x % 8);
  const auto bit_mask = static_cast<std::uint8_t>(pixel_constants::BW_MSB_MASK >> bit_offset);
  return {byte_index, bit_mask};
}

/**
 * @brief Calculate byte index and pixel shift for Grayscale4 mode pixel.
 *
 * **Encoding:**
 * - 2 bits per pixel, 4 pixels per byte
 * - Packed MSB-first: pixel 0 in bits [7:6], pixel 1 in bits [5:4], etc.
 * - 4 gray levels: 00=Black, 01=Gray2 (dark), 10=Gray1 (light), 11=White
 *
 * **Example (width=6):**
 * ```
 * Pixels:  [0][1][2][3] | [4][5][pad][pad]
 * Byte 0:  [76][54][32][10] | Byte 1: [76][54][--][--]
 * ```
 *
 * **Pixel shift values:**
 * - x % 4 == 0: shift = 0 (bits [7:6])
 * - x % 4 == 1: shift = 2 (bits [5:4])
 * - x % 4 == 2: shift = 4 (bits [3:2])
 * - x % 4 == 3: shift = 6 (bits [1:0])
 *
 * @param width Display width in pixels
 * @param x X coordinate
 * @param y Y coordinate
 * @return Pair of (byte_index, pixel_shift)
 */
[[nodiscard]] constexpr auto calculate_gray_position(std::size_t width, std::size_t x, std::size_t y) noexcept
    -> std::pair<std::size_t, std::uint8_t> {
  const auto width_bytes = (width + 3) / 4;
  const auto byte_index = (x / 4) + (y * width_bytes);
  const auto pixel_offset = static_cast<std::uint8_t>((x % 4) * 2);
  return {byte_index, pixel_offset};
}

/**
 * @brief Calculate byte index and bit offset for Spectra6 (3-bit) mode pixel.
 *
 * **Encoding:**
 * - 3 bits per pixel (values 0-5 for 6 colors, 6-7 unused)
 * - Pixels packed sequentially across byte boundaries
 * - No alignment - pixel N starts at bit (N * 3) from buffer start
 *
 * **Example (first 3 pixels = 9 bits):**
 * ```
 * Pixel:   [  0  ][  1  ][  2  ]
 * Bits:    210 210 210
 * Byte 0:  [76543210] (contains p0 + p1_b2)
 * Byte 1:  [76543210] (contains p1_b1b0 + p2)
 * ```
 *
 * **Cross-byte handling:**
 * - bit_offset ≤ 5: pixel fits in single byte
 * - bit_offset > 5: pixel spans two bytes (need to merge bits)
 *
 * @param width Display width in pixels
 * @param x X coordinate
 * @param y Y coordinate
 * @return Pair of (byte_index, bit_offset within byte)
 */
[[nodiscard]] constexpr auto calculate_spectra6_position(std::size_t width, std::size_t x, std::size_t y) noexcept
    -> std::pair<std::size_t, std::size_t> {
  const std::size_t pixel_index = (y * width) + x;
  const std::size_t byte_index = (pixel_index * 3) / 8;
  const std::size_t bit_offset = (pixel_index * 3) % 8;
  return {byte_index, bit_offset};
}

// ============================================================================
// Pixel Getters (Read from buffer)
// ============================================================================

/**
 * @brief Read a pixel from a B/W mode buffer.
 *
 * **Bit Transformation:**
 * 1. Calculate byte index and bit mask (MSB-first)
 * 2. Read byte from buffer
 * 3. AND byte with bit_mask
 * 4. Non-zero result = White (1), zero result = Black (0)
 *
 * **Safety:**
 * - Returns Color::White if byte_index out of bounds
 * - No undefined behavior on invalid coordinates
 *
 * @param buffer Framebuffer data (read-only)
 * @param width Display width
 * @param x X coordinate
 * @param y Y coordinate
 * @return Color::White or Color::Black
 */
[[nodiscard]] inline auto get_pixel_bw(std::span<const std::byte> buffer, std::size_t width, std::size_t x,
                                       std::size_t y) noexcept -> Color {
  const auto [byte_index, bit_mask] = calculate_bw_position(width, x, y);
  if (byte_index >= buffer.size()) {
    return Color::White;
  }
  const auto byte_val = static_cast<std::uint8_t>(buffer[byte_index]);
  return ((byte_val & bit_mask) != 0) ? Color::White : Color::Black;
}

/**
 * @brief Read a pixel from a Grayscale4 mode buffer.
 *
 * **Bit Transformation:**
 * 1. Calculate byte index and pixel shift (2 bits per pixel)
 * 2. Read byte from buffer
 * 3. Right-shift by (6 - pixel_shift) to align 2-bit field to LSBs
 * 4. Mask with 0x03 to extract 2-bit value
 * 5. Map 2-bit value to Color:
 *    - 0b00 (0) → Black
 *    - 0b01 (1) → Gray2 (dark gray)
 *    - 0b10 (2) → Gray1 (light gray)
 *    - 0b11 (3) → White
 *
 * **Safety:**
 * - Returns Color::White if byte_index out of bounds
 *
 * @param buffer Framebuffer data (read-only)
 * @param width Display width
 * @param x X coordinate
 * @param y Y coordinate
 * @return One of the four grayscale Color values
 */
[[nodiscard]] inline auto get_pixel_grayscale4(std::span<const std::byte> buffer, std::size_t width, std::size_t x,
                                               std::size_t y) noexcept -> Color {
  const auto [byte_index, pixel_shift] = calculate_gray_position(width, x, y);
  if (byte_index >= buffer.size()) {
    return Color::White;
  }
  const auto byte_val = static_cast<std::uint8_t>(buffer[byte_index]);
  const auto gray_level = (byte_val >> (6 - pixel_shift)) & 0x03;

  switch (gray_level) {
  case 0:
    return Color::Black;
  case 1:
    return Color::Gray2;
  case 2:
    return Color::Gray1;
  case 3:
    return Color::White;
  default:
    return Color::White;
  }
}

/**
 * @brief Read a pixel from a BWR or BWY mode buffer.
 *
 * **Dual-Plane Encoding:**
 * - Buffer layout: [BW plane (plane_size bytes)][Color plane (plane_size bytes)]
 * - Each plane: 1bpp, same layout as BlackWhite mode
 *
 * **Bit Transformation:**
 * 1. Read BW plane bit: 1 = candidate for white/color, 0 = candidate for black/color
 * 2. Read Color plane bit: 0 = use color ink (active-low), 1 = no color ink
 * 3. Decode combination:
 *    - (BW=1, Color=1) → White (no ink on either plane)
 *    - (BW=0, Color=1) → Black (BW ink only)
 *    - (BW=x, Color=0) → Red/Yellow (color ink overwrites BW)
 *
 * **Active-Low Color Plane:**
 * E-paper color planes typically use inverted logic where 0 = "apply color ink".
 * This allows white to be the default (all 1s).
 *
 * **Safety:**
 * - Returns Color::White if out of bounds
 * - Validates both plane accesses
 *
 * @param buffer Framebuffer data (two planes: B/W + color)
 * @param width Display width
 * @param height Display height (needed to calculate plane_size)
 * @param x X coordinate
 * @param y Y coordinate
 * @param is_bwr True for BWR mode (Red), false for BWY mode (Yellow)
 * @return Color::White, Color::Black, Color::Red, or Color::Yellow
 */
[[nodiscard]] inline auto get_pixel_bwr_bwy(std::span<const std::byte> buffer, std::size_t width, std::size_t height,
                                            std::size_t x, std::size_t y, bool is_bwr) noexcept -> Color {
  const std::size_t plane_size = (width * height + 7) / 8;
  const auto [byte_index, bit_mask] = calculate_bw_position(width, x, y);

  if (byte_index >= buffer.size()) {
    return Color::White;
  }

  // Check B/W plane (first plane)
  const auto bw_byte = static_cast<std::uint8_t>(buffer[byte_index]);
  const bool is_white = (bw_byte & bit_mask) != 0;

  // Check color plane (second plane) if available
  if (byte_index + plane_size < buffer.size()) {
    const auto color_byte = static_cast<std::uint8_t>(buffer[byte_index + plane_size]);

    // In e-paper color planes, 0 usually means "Color" (Ink) and 1 means "No Color" (Transparent/White)
    // This is active-low logic.
    const bool is_color = (color_byte & bit_mask) == 0;

    if (is_color) {
      return is_bwr ? Color::Red : Color::Yellow;
    }
  }

  return is_white ? Color::White : Color::Black;
}

/**
 * @brief Read a pixel from a Spectra6 mode buffer.
 *
 * **3-Bit Cross-Byte Decoding:**
 * - Pixel N spans bits [(N*3) : (N*3 + 2)] in buffer
 * - May require reading 1 or 2 bytes depending on alignment
 *
 * **Case 1: Single-byte (bit_offset ≤ 5):**
 * ```
 * Byte:  [76543210]
 *         ^^^       3-bit pixel fits in one byte
 * Shift right by (5 - bit_offset), mask with 0x07
 * ```
 *
 * **Case 2: Cross-byte (bit_offset = 6 or 7):**
 * ```
 * Byte N:   [......10]  (high_bits = 2 or 1)
 * Byte N+1: [1.......]  (low_bits = 1 or 2)
 * Merge: (high_byte & mask) << low_bits | (low_byte >> shift) & mask
 * ```
 *
 * **Color Mapping:**
 * 0=Black, 1=White, 2=Red, 3=Yellow, 4=Blue, 5=Green, 6-7=Fallback to Black
 *
 * **Safety:**
 * - Returns Color::White if byte_index out of bounds
 * - Handles partial second byte gracefully
 *
 * @param buffer Framebuffer data (read-only)
 * @param width Display width
 * @param x X coordinate
 * @param y Y coordinate
 * @return One of the six Spectra6 colors
 */
[[nodiscard]] inline auto get_pixel_spectra6(std::span<const std::byte> buffer, std::size_t width, std::size_t x,
                                             std::size_t y) noexcept -> Color {
  const auto [byte_index, bit_offset] = calculate_spectra6_position(width, x, y);

  if (byte_index >= buffer.size()) {
    return Color::White;
  }

  std::uint8_t color_value = 0;

  if (bit_offset <= 5) {
    // 3-bit value fits within single byte
    const auto shift = static_cast<std::uint8_t>(5 - bit_offset);
    const auto byte_val = static_cast<std::uint8_t>(buffer[byte_index]);
    color_value = (byte_val >> shift) & pixel_constants::SPECTRA6_COLOR_MASK;
  } else {
    // 3-bit value spans two bytes
    const auto high_bits = static_cast<std::uint8_t>(8 - bit_offset);
    const auto low_bits = static_cast<std::uint8_t>(3 - high_bits);

    const auto high_byte = static_cast<std::uint8_t>(buffer[byte_index]);
    color_value = static_cast<std::uint8_t>((high_byte & ((1U << high_bits) - 1)) << low_bits);

    if (byte_index + 1 < buffer.size()) {
      const auto low_byte = static_cast<std::uint8_t>(buffer[byte_index + 1]);
      color_value = static_cast<std::uint8_t>(color_value | ((low_byte >> (8 - low_bits)) & ((1U << low_bits) - 1)));
    }
  }

  return spectra6_value_to_color(color_value);
}

/**
 * @brief Read a pixel from a buffer based on display mode.
 *
 * Dispatches to the appropriate mode-specific function.
 *
 * @param mode Display mode
 * @param buffer Framebuffer data
 * @param width Display width
 * @param height Display height (needed for multi-plane modes)
 * @param x X coordinate
 * @param y Y coordinate
 * @return Pixel color
 */
[[nodiscard]] inline auto get_pixel_from_buffer(DisplayMode mode, std::span<const std::byte> buffer, std::size_t width,
                                                std::size_t height, std::size_t x, std::size_t y) noexcept -> Color {
  switch (mode) {
  case DisplayMode::BlackWhite:
    return get_pixel_bw(buffer, width, x, y);
  case DisplayMode::Grayscale4:
    return get_pixel_grayscale4(buffer, width, x, y);
  case DisplayMode::BWR:
    return get_pixel_bwr_bwy(buffer, width, height, x, y, true);
  case DisplayMode::BWY:
    return get_pixel_bwr_bwy(buffer, width, height, x, y, false);
  case DisplayMode::Spectra6:
    return get_pixel_spectra6(buffer, width, x, y);
  }
  return Color::White;
}

// ============================================================================
// Pixel Setters (Write to buffer)
// ============================================================================

/**
 * @brief Write a pixel to a B/W mode buffer.
 *
 * **Bit Transformation:**
 * 1. Calculate byte index and bit mask (single bit set)
 * 2. Read current byte value
 * 3. For White: OR byte with bit_mask (set bit to 1)
 * 4. For Black: AND byte with ~bit_mask (clear bit to 0)
 * 5. Write modified byte back
 *
 * **Read-Modify-Write:**
 * Must preserve other 7 bits in the byte. Using bitwise operations
 * ensures atomic semantics on single-threaded systems.
 *
 * **Safety:**
 * - No-op if byte_index out of bounds (silent fail)
 * - Treats all non-White colors as Black
 *
 * @param buffer Framebuffer data (mutable)
 * @param width Display width
 * @param x X coordinate
 * @param y Y coordinate
 * @param color Pixel color (White or Black)
 */
inline auto set_pixel_bw(std::span<std::byte> buffer, std::size_t width, std::size_t x, std::size_t y,
                         Color color) noexcept -> void {
  const auto [byte_index, bit_mask] = calculate_bw_position(width, x, y);
  if (byte_index >= buffer.size()) {
    return;
  }

  auto &byte_val = buffer[byte_index];
  if (color == Color::White) {
    byte_val = static_cast<std::byte>(static_cast<std::uint8_t>(byte_val) | bit_mask);
  } else {
    byte_val = static_cast<std::byte>(static_cast<std::uint8_t>(byte_val) & ~bit_mask);
  }
}

/**
 * @brief Write a pixel to a Grayscale4 mode buffer.
 *
 * **Bit Transformation:**
 * 1. Calculate byte index and pixel shift (2 bits per pixel)
 * 2. Extract 2-bit color value from Color enum (bits [7:6])
 * 3. Shift color value right by pixel_shift to align with target position
 * 4. Create mask for 2-bit field: 0xC0 >> pixel_shift
 * 5. Clear target field: byte & ~mask
 * 6. Set new value: (byte & ~mask) | (color_bits & mask)
 *
 * **Color Encoding:**
 * - Color enum already has correct bit patterns in [7:6]:
 *   - Black = 0b00xx_xxxx → 0b00
 *   - Gray2 = 0b01xx_xxxx → 0b01
 *   - Gray1 = 0b10xx_xxxx → 0b10
 *   - White = 0b11xx_xxxx → 0b11
 *
 * **Safety:**
 * - No-op if byte_index out of bounds
 * - Preserves other 3 pixels in the byte
 *
 * @param buffer Framebuffer data (mutable)
 * @param width Display width
 * @param x X coordinate
 * @param y Y coordinate
 * @param color Pixel color (one of 4 grayscale levels)
 */
inline auto set_pixel_grayscale4(std::span<std::byte> buffer, std::size_t width, std::size_t x, std::size_t y,
                                 Color color) noexcept -> void {
  const auto [byte_index, pixel_shift] = calculate_gray_position(width, x, y);
  if (byte_index >= buffer.size()) {
    return;
  }

  auto &byte_val = buffer[byte_index];
  const auto mask = static_cast<std::uint8_t>(pixel_constants::GRAY_PIXEL_MASK >> pixel_shift);
  const std::uint8_t color_bits = (static_cast<std::uint8_t>(color) & pixel_constants::GRAY_PIXEL_MASK) >> pixel_shift;

  // Clear the 2 bits and set new color
  byte_val = static_cast<std::byte>((static_cast<std::uint8_t>(byte_val) & ~mask) | (color_bits & mask));
}

/**
 * @brief Write a pixel to a BWR or BWY mode buffer.
 *
 * **Dual-Plane Encoding Logic:**
 * ```
 * Input Color → (BW plane bit, Color plane bit)
 * White       → (1, 1)  # No ink on either plane
 * Black       → (0, 1)  # BW ink only
 * Red/Yellow  → (0, 0)  # Color ink (active-low)
 * ```
 *
 * **Bit Transformation:**
 * 1. Calculate byte index and bit mask (same for both planes)
 * 2. Decode input color to (is_bw_white, is_color) flags
 * 3. Write BW plane: set bit if is_bw_white, clear otherwise
 * 4. Write Color plane: clear bit if is_color (active-low!), set otherwise
 *
 * **Active-Low Color Logic:**
 * The Color plane bit is INVERTED:
 * - 0 = "apply color ink" (Red/Yellow pixel)
 * - 1 = "no color ink" (allows BW plane to show through)
 *
 * **Safety:**
 * - No-op if either plane access out of bounds
 * - Validates buffer size includes both planes
 *
 * @param buffer Framebuffer data (mutable, two planes)
 * @param width Display width
 * @param height Display height (for plane_size calculation)
 * @param x X coordinate
 * @param y Y coordinate
 * @param color Pixel color
 * @param is_bwr True for BWR mode (Red), false for BWY mode (Yellow)
 */
inline auto set_pixel_bwr_bwy(std::span<std::byte> buffer, std::size_t width, std::size_t height, std::size_t x,
                              std::size_t y, Color color, bool is_bwr) noexcept -> void {
  const std::size_t plane_size = (width * height + 7) / 8;
  const auto [byte_index, bit_mask] = calculate_bw_position(width, x, y);

  if (byte_index >= buffer.size() || byte_index + plane_size >= buffer.size()) {
    return;
  }

  // Determine plane values based on color
  bool is_bw_white = false;
  bool is_color = false;

  const auto target_color = is_bwr ? Color::Red : Color::Yellow;
  if (color == Color::White) {
    is_bw_white = true;
    is_color = false;
  } else if (color == target_color) {
    is_bw_white = false;
    is_color = true;
  } else {
    // Black or other colors -> black
    is_bw_white = false;
    is_color = false;
  }

  // Set B/W plane (first plane)
  auto &bw_byte = buffer[byte_index];
  if (is_bw_white) {
    bw_byte = static_cast<std::byte>(static_cast<std::uint8_t>(bw_byte) | bit_mask);
  } else {
    bw_byte = static_cast<std::byte>(static_cast<std::uint8_t>(bw_byte) & ~bit_mask);
  }

  // Set color plane (second plane)
  auto &color_byte = buffer[byte_index + plane_size];
  if (is_color) {
    color_byte = static_cast<std::byte>(static_cast<std::uint8_t>(color_byte) | bit_mask);
  } else {
    color_byte = static_cast<std::byte>(static_cast<std::uint8_t>(color_byte) & ~bit_mask);
  }
}

/**
 * @brief Write a pixel to a Spectra6 mode buffer.
 *
 * @param buffer Framebuffer data (mutable)
 * @param width Display width
 * @param x X coordinate
 * @param y Y coordinate
 * @param color Pixel color
 */
inline auto set_pixel_spectra6(std::span<std::byte> buffer, std::size_t width, std::size_t x, std::size_t y,
                               Color color) noexcept -> void {
  const auto [byte_index, bit_offset] = calculate_spectra6_position(width, x, y);

  if (byte_index >= buffer.size()) {
    return;
  }

  const std::uint8_t color_value = spectra6_color_to_value(color);

  if (bit_offset <= 5) {
    // 3-bit value fits within single byte
    const auto shift = static_cast<std::uint8_t>(5 - bit_offset);
    const auto mask = static_cast<std::uint8_t>(pixel_constants::SPECTRA6_COLOR_MASK << shift);
    auto &byte_val = buffer[byte_index];
    byte_val = static_cast<std::byte>((static_cast<std::uint8_t>(byte_val) & ~mask) | ((color_value & 0x07) << shift));
  } else {
    // 3-bit value spans two bytes
    const auto high_bits = static_cast<std::uint8_t>(8 - bit_offset);
    const auto low_bits = static_cast<std::uint8_t>(3 - high_bits);

    // First byte: set the high bits
    const auto high_mask = static_cast<std::uint8_t>((1U << high_bits) - 1);
    auto &high_byte = buffer[byte_index];
    high_byte = static_cast<std::byte>((static_cast<std::uint8_t>(high_byte) & ~high_mask) |
                                       ((color_value >> low_bits) & high_mask));

    // Second byte: set the low bits
    if (byte_index + 1 < buffer.size()) {
      const auto low_mask = static_cast<std::uint8_t>(((1U << low_bits) - 1) << (8 - low_bits));
      auto &low_byte = buffer[byte_index + 1];
      low_byte = static_cast<std::byte>((static_cast<std::uint8_t>(low_byte) & ~low_mask) |
                                        ((color_value & ((1U << low_bits) - 1)) << (8 - low_bits)));
    }
  }
}

/**
 * @brief Write a pixel to a buffer based on display mode.
 *
 * Dispatches to the appropriate mode-specific function.
 *
 * @param mode Display mode
 * @param buffer Framebuffer data (mutable)
 * @param width Display width
 * @param height Display height (needed for multi-plane modes)
 * @param x X coordinate
 * @param y Y coordinate
 * @param color Pixel color
 */
inline auto set_pixel_in_buffer(DisplayMode mode, std::span<std::byte> buffer, std::size_t width, std::size_t height,
                                std::size_t x, std::size_t y, Color color) noexcept -> void {
  switch (mode) {
  case DisplayMode::BlackWhite:
    set_pixel_bw(buffer, width, x, y, color);
    break;
  case DisplayMode::Grayscale4:
    set_pixel_grayscale4(buffer, width, x, y, color);
    break;
  case DisplayMode::BWR:
    set_pixel_bwr_bwy(buffer, width, height, x, y, color, true);
    break;
  case DisplayMode::BWY:
    set_pixel_bwr_bwy(buffer, width, height, x, y, color, false);
    break;
  case DisplayMode::Spectra6:
    set_pixel_spectra6(buffer, width, x, y, color);
    break;
  }
}

} // namespace epaper
