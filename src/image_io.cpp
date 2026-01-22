#include "epaper/io/image_io.hpp"
#include "epaper/color/color_manager.hpp" // Added include
#include "epaper/core/types.hpp"
#include <format>
#include <string>

// Define STB implementation macros only here
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

// Suppress STB warnings
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

// Include local third_party headers
#include "third_party/stb_image.h"
#include "third_party/stb_image_write.h"

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

namespace epaper {

auto ImageIO::load_image(std::string_view path, int desired_channels) -> std::expected<ImageResult, Error> {
  int w = 0;
  int h = 0;
  int c = 0;

  // stbi_load expects const char* (C-style string)
  // Convert std::string_view to std::string for null termination
  std::string path_str(path);

  // stbi_load() decodes image from disk
  // Returns: pointer to decoded pixel data (caller must free with stbi_image_free)
  // Parameters:
  //   - path_str.c_str(): File path
  //   - &w, &h, &c: Output width, height, channels (modified by stbi_load)
  //   - desired_channels: Force specific channel count (0 = auto-detect)
  // Supported formats: PNG, JPEG, BMP, GIF (first frame), TGA, PSD, HDR
  unsigned char *data = stbi_load(path_str.c_str(), &w, &h, &c, desired_channels);

  if (data == nullptr) {
    // stbi_load returns nullptr on failure (file not found, unsupported format, etc.)
    return std::unexpected(Error(ErrorCode::LoadFailed, std::format("Failed to load image: {}", path)));
  }

  // Calculate total data size in bytes
  // If desired_channels specified, use that; otherwise use detected channels
  std::size_t size =
      static_cast<std::size_t>(w) * static_cast<std::size_t>(h) *
      ((desired_channels != 0) ? static_cast<std::size_t>(desired_channels) : static_cast<std::size_t>(c));

  // Copy raw pixel data to std::vector for RAII memory management
  // stbi_load allocates with malloc, but we want C++ ownership
  std::span<const unsigned char> raw_span(data, size);
  std::vector<std::uint8_t> vec(raw_span.begin(), raw_span.end());

  // Free stb_image allocated memory (required to avoid leak)
  stbi_image_free(data);

  // Return structured result with image metadata and pixel data
  // Channels = desired_channels if specified, otherwise auto-detected
  return ImageResult{.width = static_cast<std::size_t>(w),
                     .height = static_cast<std::size_t>(h),
                     .channels = (desired_channels != 0) ? static_cast<std::size_t>(desired_channels)
                                                         : static_cast<std::size_t>(c),
                     .data = std::move(vec)};
}

auto ImageIO::save_png(std::string_view path, std::size_t width, std::size_t height, int channels,
                       std::span<const std::uint8_t> data) -> std::expected<void, Error> {
  // Convert std::string_view to std::string for null termination (C API requirement)
  std::string path_str(path);

  // Calculate stride (bytes per row)
  // PNG format: row-major, interleaved channels (RGBRGBRGB...)
  // stride_in_bytes = width * channels (no padding/alignment)
  int stride = static_cast<int>(width) * channels;

  // stbi_write_png() encodes RGB/RGBA to PNG file
  // Parameters:
  //   - path_str.c_str(): Output file path (will overwrite if exists)
  //   - width, height: Image dimensions
  //   - channels: Number of channels (1=Y, 3=RGB, 4=RGBA)
  //   - data.data(): Pointer to pixel data
  //   - stride: Bytes per row (width * channels)
  // Returns: Non-zero on success, 0 on failure
  // Compression: zlib deflate (PNG standard)
  int result = stbi_write_png(path_str.c_str(), static_cast<int>(width), static_cast<int>(height), channels,
                              data.data(), stride);

  if (result == 0) {
    // stbi_write_png returns 0 on failure (disk full, permission denied, etc.)
    return std::unexpected(Error(ErrorCode::LoadFailed, std::format("Failed to save PNG: {}", path)));
  }

  // Success - PNG file written to disk
  return {};
}

} // namespace epaper
