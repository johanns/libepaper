#include "epaper/core/framebuffer.hpp"
#include "epaper/internal/internal.hpp"
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>

namespace epaper {
// Pixel packing constants - used across all encoding modes
namespace {
constexpr std::size_t BYTE_BITS = 8;                // Standard byte size
constexpr std::size_t GRAYSCALE_BITS_PER_PIXEL = 2; // 2 bits per pixel for 4-level grayscale
constexpr std::size_t GRAYSCALE_PIXELS_PER_BYTE = BYTE_BITS / GRAYSCALE_BITS_PER_PIXEL; // 4 pixels per byte
constexpr std::size_t SPECTRA_BITS_PER_PIXEL = 3; // 3 bits per pixel for 6 colors (2^3 = 8 values, using 6)
} // namespace

MonoFramebuffer::MonoFramebuffer(std::size_t width, std::size_t height, DisplayMode mode)
    : width_(width), height_(height), mode_(mode) {
  switch (mode_) {
  case DisplayMode::BlackWhite: {
    // 1 bit per pixel - pack 8 pixels per byte
    // Stride is width rounded up to next byte boundary
    stride_ = (width_ + BYTE_BITS - 1) / BYTE_BITS;
    const std::size_t size = stride_ * height_;
    // Initialize to 0xFF (all white pixels)
    buffer_.assign(size, std::byte{0xFF});

    break;
  }
  case DisplayMode::Grayscale4: {
    // 2 bits per pixel - pack 4 pixels per byte
    // Supports 4 gray levels: 00=black, 01=dark gray, 10=light gray, 11=white
    stride_ = (width_ + GRAYSCALE_PIXELS_PER_BYTE - 1) / GRAYSCALE_PIXELS_PER_BYTE;
    const std::size_t size = stride_ * height_;
    // Initialize to 0xFF (all white pixels - value 11 in each 2-bit field)
    buffer_.assign(size, std::byte{0xFF});

    break;
  }
  case DisplayMode::Spectra6: {
    // 3 bits per pixel - supports 6 colors (uses values 0-5 of possible 0-7)
    // Pixels are tightly packed - may span byte boundaries
    const std::size_t size = (width_ * height_ * SPECTRA_BITS_PER_PIXEL + BYTE_BITS - 1) / BYTE_BITS;
    // Initialize to 0x00, then set all pixels to white explicitly
    buffer_.assign(size, std::byte{0x00});
    clear(Color::White); // Set proper white encoding

    break;
  }
  case DisplayMode::BWR:
  case DisplayMode::BWY:
    // Multi-plane modes should not use MonoFramebuffer
    std::unreachable();
  }
}

auto MonoFramebuffer::width() const -> std::size_t { return width_; }

auto MonoFramebuffer::height() const -> std::size_t { return height_; }

auto MonoFramebuffer::mode() const -> DisplayMode { return mode_; }

auto MonoFramebuffer::data() const -> std::span<const std::byte> { return buffer_; }

auto MonoFramebuffer::get_planes() const -> std::vector<std::span<const std::byte>> { return {buffer_}; }

auto MonoFramebuffer::set_pixel(std::size_t x, std::size_t y, Color color, Orientation orientation) -> void {
  // Transform logical coordinates to physical based on orientation
  auto [px, py] = internal::transform_coordinates(x, y, width_, height_, orientation);
  if (px >= width_ || py >= height_) {
    return; // Out of bounds - silently ignore
  }

  switch (mode_) {
  case DisplayMode::BlackWhite: {
    // 1 bpp: 8 pixels packed per byte, MSB is leftmost pixel
    // Bit position within byte: 7 6 5 4 3 2 1 0 (bit 7 = leftmost pixel)
    const std::size_t index = (py * stride_) + (px / BYTE_BITS);
    const auto bit = static_cast<std::uint8_t>(1U << (BYTE_BITS - 1U - (px % BYTE_BITS)));
    const bool is_white = (color == Color::White);

    if (is_white) {
      buffer_[index] |= std::byte{bit}; // Set bit to 1 (white)
    } else {
      buffer_[index] &= std::byte{static_cast<std::uint8_t>(~bit)}; // Clear bit to 0 (black)
    }
    break;
  }
  case DisplayMode::Grayscale4: {
    // 2 bpp: 4 pixels per byte, 2 bits each
    // Byte layout: [p0 p1 p2 p3] where each p is 2 bits
    const std::size_t byte_index = (py * stride_) + (px / GRAYSCALE_PIXELS_PER_BYTE);
    const std::size_t pixel_index = px % GRAYSCALE_PIXELS_PER_BYTE;
    // Calculate bit shift: leftmost pixel uses bits 6-7, rightmost uses bits 0-1
    const int shift = static_cast<int>((GRAYSCALE_PIXELS_PER_BYTE - 1 - pixel_index) * GRAYSCALE_BITS_PER_PIXEL);

    // Map Color enum to 2-bit gray value
    std::uint8_t val = 0x03; // Default to white
    switch (color) {
    case Color::Black:
      val = 0x00; // Darkest
      break;
    case Color::Gray2:
      val = 0x01; // Dark gray
      break;
    case Color::Gray1:
      val = 0x02; // Light gray
      break;
    case Color::White:
      val = 0x03; // Lightest
      break;
    default:
      val = 0x03;
      break;
    }

    // Clear target 2-bit field and set new value
    const auto mask = static_cast<std::uint8_t>(~(0x03U << shift));
    const std::byte current = buffer_[byte_index];
    buffer_[byte_index] = (current & std::byte{mask}) | std::byte{static_cast<std::uint8_t>(val << shift)};
    break;
  }
  case DisplayMode::Spectra6: {
    // 3 bpp: Pixels packed tightly - can span byte boundaries
    // Calculate linear pixel index and convert to bit-level addressing
    const std::size_t pixel_index = (py * width_) + px;
    const std::size_t byte_index = (pixel_index * SPECTRA_BITS_PER_PIXEL) / BYTE_BITS;
    const std::size_t bit_offset = (pixel_index * SPECTRA_BITS_PER_PIXEL) % BYTE_BITS;

    // Map Color enum to 3-bit Spectra6 value (0-5 used, 6-7 reserved)
    std::uint8_t val = 0;
    switch (color) {
    case Color::Black:
      val = 0;
      break;
    case Color::White:
      val = 1;
      break;
    case Color::Red:
      val = 2;
      break;
    case Color::Yellow:
      val = 3;
      break;
    case Color::Blue:
      val = 4;
      break;
    case Color::Green:
      val = 5;
      break;
    default:
      val = 0;
      break;
    }

    // Check if 3-bit value fits within current byte
    if (bit_offset <= (BYTE_BITS - SPECTRA_BITS_PER_PIXEL)) {
      // Simple case: all 3 bits fit in one byte
      const int shift = static_cast<int>(BYTE_BITS - SPECTRA_BITS_PER_PIXEL - bit_offset);
      const auto mask = static_cast<std::uint8_t>(0x07U << shift); // 0x07 = 3-bit mask
      buffer_[byte_index] =
          static_cast<std::byte>((static_cast<std::uint8_t>(buffer_[byte_index]) & ~mask) | ((val & 0x07U) << shift));
    } else {
      // Complex case: 3-bit value spans two bytes
      // Split value into high bits (current byte) and low bits (next byte)
      const int high_bits = static_cast<int>(BYTE_BITS - bit_offset);
      const int low_bits = static_cast<int>(SPECTRA_BITS_PER_PIXEL - high_bits);

      // Write high bits to current byte (LSBs)
      const auto high_mask = static_cast<std::uint8_t>((1U << high_bits) - 1U);
      buffer_[byte_index] = static_cast<std::byte>((static_cast<std::uint8_t>(buffer_[byte_index]) & ~high_mask) |
                                                   ((val >> low_bits) & high_mask));

      // Write low bits to next byte (MSBs)
      if (byte_index + 1 < buffer_.size()) {
        const auto low_mask = static_cast<std::uint8_t>(((1U << low_bits) - 1U) << (BYTE_BITS - low_bits));
        buffer_[byte_index + 1] =
            static_cast<std::byte>((static_cast<std::uint8_t>(buffer_[byte_index + 1]) & ~low_mask) |
                                   ((val & ((1U << low_bits) - 1U)) << (BYTE_BITS - low_bits)));
      }
    }
    break;
  }
  case DisplayMode::BWR:
  case DisplayMode::BWY:
    std::unreachable();
  }
}

auto MonoFramebuffer::get_pixel(std::size_t x, std::size_t y, Orientation orientation) const -> Color {
  auto [px, py] = internal::transform_coordinates(x, y, width_, height_, orientation);
  if (px >= width_ || py >= height_) {
    return Color::White;
  }

  switch (mode_) {
  case DisplayMode::BlackWhite: {
    const std::size_t index = (py * stride_) + (px / BYTE_BITS);
    const auto bit = static_cast<std::uint8_t>(1U << (BYTE_BITS - 1U - (px % BYTE_BITS)));
    return ((static_cast<std::uint8_t>(buffer_[index]) & bit) != 0U) ? Color::White : Color::Black;
  }
  case DisplayMode::Grayscale4: {
    const std::size_t byte_index = (py * stride_) + (px / GRAYSCALE_PIXELS_PER_BYTE);
    const std::size_t pixel_index = px % GRAYSCALE_PIXELS_PER_BYTE;
    const int shift = static_cast<int>((GRAYSCALE_PIXELS_PER_BYTE - 1 - pixel_index) * GRAYSCALE_BITS_PER_PIXEL);
    const std::uint8_t val = (static_cast<std::uint8_t>(buffer_[byte_index]) >> shift) & 0x03U;

    switch (val) {
    case 0x00:
      return Color::Black;
    case 0x01:
      return Color::Gray2;
    case 0x02:
      return Color::Gray1;
    case 0x03:
      return Color::White;
    default:
      return Color::White;
    }
  }
  case DisplayMode::Spectra6: {
    const std::size_t pixel_index = (py * width_) + px;
    const std::size_t byte_index = (pixel_index * SPECTRA_BITS_PER_PIXEL) / BYTE_BITS;
    const std::size_t bit_offset = (pixel_index * SPECTRA_BITS_PER_PIXEL) % BYTE_BITS;

    std::uint8_t val = 0;
    if (bit_offset <= (BYTE_BITS - SPECTRA_BITS_PER_PIXEL)) {
      const int shift = static_cast<int>(BYTE_BITS - SPECTRA_BITS_PER_PIXEL - bit_offset);
      val = (static_cast<std::uint8_t>(buffer_[byte_index]) >> shift) & 0x07U;
    } else {
      const int high_bits = static_cast<int>(BYTE_BITS - bit_offset);
      const int low_bits = static_cast<int>(SPECTRA_BITS_PER_PIXEL - high_bits);
      const std::uint8_t high = static_cast<std::uint8_t>(buffer_[byte_index]) & ((1U << high_bits) - 1U);
      std::uint8_t low = 0;
      if (byte_index + 1 < buffer_.size()) {
        low = (static_cast<std::uint8_t>(buffer_[byte_index + 1]) >> (BYTE_BITS - low_bits)) & ((1U << low_bits) - 1U);
      }
      val = static_cast<std::uint8_t>((high << low_bits) | low);
    }

    switch (val) {
    case 0:
      return Color::Black;
    case 1:
      return Color::White;
    case 2:
      return Color::Red;
    case 3:
      return Color::Yellow;
    case 4:
      return Color::Blue;
    case 5:
      return Color::Green;
    default:
      return Color::Black;
    }
  }
  case DisplayMode::BWR:
  case DisplayMode::BWY:
    std::unreachable();
  }
  std::unreachable();
}

auto MonoFramebuffer::clear(Color color) -> void {
  switch (mode_) {
  case DisplayMode::BlackWhite: {
    const std::byte fill = (color == Color::Black) ? std::byte{0x00} : std::byte{0xFF};
    std::ranges::fill(buffer_, fill);
    break;
  }
  case DisplayMode::Grayscale4: {
    auto fill = std::byte{0xFF};
    switch (color) {
    case Color::Black:
      fill = std::byte{0x00};
      break;
    case Color::Gray2:
      fill = std::byte{0x55};
      break;
    case Color::Gray1:
      fill = std::byte{0xAA};
      break;
    case Color::White:
      fill = std::byte{0xFF};
      break;
    default:
      break;
    }
    std::ranges::fill(buffer_, fill);
    break;
  }
  case DisplayMode::Spectra6: {
    if (color == Color::White) {
      const std::array<std::byte, 3> pattern = {std::byte{0x24}, std::byte{0x92}, std::byte{0x49}};
      for (std::size_t i = 0; i < buffer_.size(); ++i) {
        buffer_[i] = pattern.at(i % pattern.size());
      }
      break;
    }
    if (color == Color::Black) {
      std::ranges::fill(buffer_, std::byte{0x00});
      break;
    }
    for (std::size_t y = 0; y < height_; ++y) {
      for (std::size_t x = 0; x < width_; ++x) {
        set_pixel(x, y, color, Orientation::Portrait0);
      }
    }
    break;
  }
  case DisplayMode::BWR:
  case DisplayMode::BWY:
    std::unreachable();
  }
}

} // namespace epaper
