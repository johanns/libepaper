#pragma once

/**
 * @file image_io.hpp
 * @brief Image loading/saving utilities using stb_image library.
 *
 * Provides high-level interface for loading images from disk and saving
 * framebuffers as PNG files. Uses stb_image for format support.
 *
 * **Supported Formats:**
 * ```
 * Loading (stb_image):
 * ├─ PNG (Portable Network Graphics) - lossless, alpha support
 * ├─ JPEG/JPG (Joint Photographic Experts Group) - lossy, no alpha
 * ├─ BMP (Windows Bitmap) - uncompressed
 * ├─ TGA (Truevision Targa) - gaming formats
 * ├─ GIF (Graphics Interchange Format) - animation (first frame only)
 * ├─ PSD (Photoshop Document) - composited image only
 * └─ HDR (Radiance RGBE) - high dynamic range
 *
 * Saving (stb_image_write):
 * └─ PNG (Portable Network Graphics) - lossless, alpha support
 * ```
 *
 * **Pixel Format Conversions:**
 * ```
 * Input Image → RGB/RGBA → Color Enum → DisplayMode → Framebuffer
 *     ↓            ↓           ↓             ↓            ↓
 *   PNG/JPG    stb_image  ColorManager  PixelCodec   set_pixel()
 * ```
 *
 * **Channel Modes:**
 * - 1 channel: Grayscale (Y)
 * - 3 channels: RGB (interleaved: RGBRGBRGB...)
 * - 4 channels: RGBA (interleaved: RGBARGBARGBA...)
 *
 * **Framebuffer Export:**
 * - `framebuffer_to_rgb()`: Convert any FramebufferLike to RGB image
 * - Used by MockDriver for test verification
 * - Enables visual debugging (save framebuffer state as PNG)
 *
 * **Use Cases:**
 * - Load logos/icons for display on e-paper
 * - Export framebuffer state for debugging
 * - Batch convert images to e-paper color modes
 * - Generate test fixtures (compare rendered vs expected)
 *
 * **Error Handling:**
 * - All operations return `std::expected<T, Error>` for fallible I/O
 * - Common errors: File not found, unsupported format, out of memory
 *
 * @example
 * ```cpp
 * // Load and display image
 * auto result = ImageIO::load_image("logo.png", 3);  // Force RGB
 * if (result) {
 *   auto [w, h, ch, data] = *result;
 *   // Convert RGB to framebuffer color mode
 *   for (size_t y = 0; y < h; ++y) {
 *     for (size_t x = 0; x < w; ++x) {
 *       size_t idx = (y * w + x) * 3;
 *       RGB rgb{data[idx], data[idx+1], data[idx+2]};
 *       Color c = ColorManager::to_color(rgb, DisplayMode::BlackWhite);
 *       framebuffer.set_pixel(x, y, c, Orientation::Portrait0);
 *     }
 *   }
 * }
 *
 * // Save framebuffer as PNG
 * auto rgb = ImageIO::framebuffer_to_rgb(framebuffer);
 * ImageIO::save_png("output.png", fb.width(), fb.height(), 3, rgb);
 * ```
 *
 * @see ColorManager, FramebufferLike, stb_image (third_party)
 */

#include "epaper/color/color_manager.hpp"
#include "epaper/core/errors.hpp"
#include "epaper/core/framebuffer_concepts.hpp"
#include "epaper/core/types.hpp"
#include <cstddef>
#include <expected>
#include <span>
#include <string_view>
#include <vector>

namespace epaper {

/**
 * @brief Utilities for loading/saving images and converting formats.
 *
 * Static-only utility class - all methods are static, no instantiation needed.
 *
 * **Design Philosophy:**
 * - Thin wrapper around stb_image library
 * - RAII: Automatic memory management for loaded images
 * - Type-safe: std::vector instead of raw pointers
 * - Error propagation: std::expected for I/O failures
 *
 * @see stb_image.h, stb_image_write.h (third_party)
 */
class ImageIO {
public:
  /**
   * @brief Result structure for image loading.
   *
   * Contains decoded image data and metadata.
   *
   * **Member Semantics:**
   * - `width`: Image width in pixels
   * - `height`: Image height in pixels
   * - `channels`: Number of color channels (1=Y, 3=RGB, 4=RGBA)
   * - `data`: Pixel data in row-major, channel-interleaved format
   *
   * **Data Layout:**
   * ```
   * RGB (3 channels):
   * [R0 G0 B0][R1 G1 B1][R2 G2 B2]... ← Row 0
   * [R0 G0 B0][R1 G1 B1][R2 G2 B2]... ← Row 1
   * ...
   *
   * RGBA (4 channels):
   * [R0 G0 B0 A0][R1 G1 B1 A1]... ← Row 0
   * ```
   *
   * **Size Invariants:**
   * - `data.size() == width × height × channels`
   * - All pixels present (no compression, no missing data)
   *
   * @example
   * ```cpp
   * auto result = ImageIO::load_image("test.png", 3);
   * if (result) {
   *   auto [w, h, ch, data] = *result;  // Structured binding
   *   assert(data.size() == w * h * ch);
   *   // Access pixel (x, y):
   *   size_t idx = (y * w + x) * ch;
   *   uint8_t r = data[idx + 0];
   *   uint8_t g = data[idx + 1];
   *   uint8_t b = data[idx + 2];
   * }
   * ```
   */
  struct ImageResult {
    std::size_t width;
    std::size_t height;
    std::size_t channels;
    std::vector<std::uint8_t> data; // Row-major, interleaved channels
  };

  /**
   * @brief Loads an image file and decodes to raw pixel data.
   *
   * Supports PNG, JPEG, BMP, GIF, TGA, PSD, HDR formats via stb_image.
   *
   * **Channel Conversion:**
   * - `desired_channels = 0`: Auto-detect from file (preserve original)
   * - `desired_channels = 1`: Force grayscale (RGB → luminance)
   * - `desired_channels = 3`: Force RGB (grayscale → replicate, RGBA → drop alpha)
   * - `desired_channels = 4`: Force RGBA (RGB → add opaque alpha=255)
   *
   * **Automatic Conversions:**
   * - stb_image handles all format decoding
   * - Color space: sRGB assumed (no ICC profile support)
   * - Bit depth: All images converted to 8-bit per channel
   * - Orientation: EXIF orientation NOT applied (loaded as-is)
   *
   * **Error Conditions:**
   * - File not found or unreadable
   * - Unsupported format (e.g., WEBP, AVIF not supported)
   * - Corrupted/truncated file data
   * - Out of memory for large images
   *
   * **Performance Notes:**
   * - Blocking I/O (may take milliseconds for large files)
   * - Allocates vector proportional to image size (width × height × channels)
   * - No image caching (each call reloads from disk)
   *
   * @param path File path (relative or absolute)
   * @param desired_channels 0=auto, 1=Y, 3=RGB, 4=RGBA
   * @return ImageResult on success, Error on failure
   *
   * @example
   * ```cpp
   * // Auto-detect channels
   * auto img = ImageIO::load_image("photo.jpg");  // Usually 3 (RGB)
   *
   * // Force grayscale
   * auto gray = ImageIO::load_image("icon.png", 1);  // Convert to Y
   *
   * // Force RGB (discard alpha)
   * auto rgb = ImageIO::load_image("logo.png", 3);  // Alpha dropped
   *
   * // Error handling
   * auto result = ImageIO::load_image("missing.png");
   * if (!result) {
   *   std::cerr << "Load failed: " << result.error().message << std::endl;
   * }
   * ```
   *
   * @see ImageResult, stb_image.h
   */
  [[nodiscard]] static auto load_image(std::string_view path, int desired_channels = 0)
      -> std::expected<ImageResult, Error>;

  /**
   * @brief Saves raw pixel data to PNG file.
   *
   * Writes uncompressed PNG using stb_image_write (deflate compression applied).
   *
   * **Supported Channels:**
   * - 1 channel: Grayscale PNG
   * - 3 channels: RGB PNG (no alpha)
   * - 4 channels: RGBA PNG (with alpha)
   * - 2 channels: NOT supported (use 1 or 3)
   *
   * **Data Format:**
   * - Row-major layout (top to bottom)
   * - Channel-interleaved (RGBRGBRGB... not planar)
   * - 8 bits per channel (no HDR/16-bit support)
   * - Size must equal `width × height × channels`
   *
   * **PNG Encoding:**
   * - Color type: Auto-selected based on channels
   * - Bit depth: 8 (standard LDR)
   * - Compression: zlib deflate (PNG standard)
   * - Interlacing: None (progressive loading not used)
   *
   * **Error Conditions:**
   * - File path unwritable (permissions, directory doesn't exist)
   * - Disk full or I/O error during write
   * - Invalid channel count (not 1, 3, or 4)
   * - Data size mismatch (data.size() != width × height × channels)
   *
   * **Performance Notes:**
   * - Blocking I/O (may take 10-100ms for large images)
   * - Compression is single-threaded
   * - Memory peak: ~2× image size (input + compressed buffer)
   *
   * @param path Output file path (will overwrite if exists)
   * @param width Image width in pixels
   * @param height Image height in pixels
   * @param channels Number of channels (1, 3, or 4)
   * @param data Pixel data (size must be width × height × channels)
   * @return Success (void) or Error
   *
   * @example
   * ```cpp
   * // Save RGB image
   * std::vector<uint8_t> rgb_data(width * height * 3);
   * // ... fill rgb_data ...
   * auto result = ImageIO::save_png("output.png", width, height, 3, rgb_data);
   * if (!result) {
   *   std::cerr << "Save failed: " << result.error().message << std::endl;
   * }
   *
   * // Save framebuffer
   * auto rgb = ImageIO::framebuffer_to_rgb(framebuffer);
   * ImageIO::save_png("framebuffer.png", fb.width(), fb.height(), 3, rgb);
   * ```
   *
   * @see framebuffer_to_rgb(), stb_image_write.h
   */
  [[nodiscard]] static auto save_png(std::string_view path, std::size_t width, std::size_t height, int channels,
                                     std::span<const std::uint8_t> data) -> std::expected<void, Error>;

  /**
   * @brief Converts a FramebufferLike to RGB (3-channel) image buffer.
   *
   * Exports framebuffer contents as standard RGB image for saving or display.
   * Used by MockDriver for test verification and debugging.
   *
   * **Conversion Process:**
   * 1. Iterate all pixels in Portrait0 orientation (canonical order)
   * 2. Get Color enum value via framebuffer.get_pixel()
   * 3. Convert Color → RGB via ColorManager::to_rgb()
   * 4. Append RGB triplet to output vector
   *
   * **Output Format:**
   * ```
   * Vector layout (row-major, RGB interleaved):
   * [R0 G0 B0][R1 G1 B1][R2 G2 B2]... ← Row 0
   * [R0 G0 B0][R1 G1 B1][R2 G2 B2]... ← Row 1
   * ...
   * Size: width × height × 3 bytes
   * ```
   *
   * **Color Mapping:**
   * - Color::Black → RGB(0, 0, 0)
   * - Color::White → RGB(255, 255, 255)
   * - Color::Gray1 → RGB(170, 170, 170)
   * - Color::Gray2 → RGB(85, 85, 85)
   * - Color::Red → RGB(255, 0, 0)
   * - Color::Yellow → RGB(255, 255, 0)
   * - Color::Blue → RGB(0, 0, 255)
   * - Color::Green → RGB(0, 255, 0)
   *
   * **Performance:**
   * - O(width × height) iteration
   * - Allocates width × height × 3 bytes
   * - No image compression (raw pixel data)
   *
   * @tparam FB FramebufferLike type (auto-deduced)
   * @param fb Framebuffer to export (const reference)
   * @return RGB data vector (size = width × height × 3)
   *
   * @example
   * ```cpp
   * // Export for debugging
   * MonoFramebuffer fb{176, 264, DisplayMode::BlackWhite};
   * // ... draw operations ...
   * auto rgb = ImageIO::framebuffer_to_rgb(fb);
   * ImageIO::save_png("debug.png", fb.width(), fb.height(), 3, rgb);
   *
   * // Compare in tests
   * auto actual_rgb = ImageIO::framebuffer_to_rgb(actual_fb);
   * auto expected_rgb = ImageIO::framebuffer_to_rgb(expected_fb);
   * ASSERT_EQ(actual_rgb, expected_rgb);
   * ```
   *
   * @see FramebufferLike, ColorManager::to_rgb(), save_png()
   */
  template <FramebufferLike FB> [[nodiscard]] static auto framebuffer_to_rgb(const FB &fb) -> std::vector<std::uint8_t>;
};

// ========== Template Implementation ==========

template <FramebufferLike FB> auto ImageIO::framebuffer_to_rgb(const FB &fb) -> std::vector<std::uint8_t> {
  std::size_t w = fb.width();
  std::size_t h = fb.height();
  std::vector<std::uint8_t> rgb;
  rgb.reserve(w * h * 3);

  for (std::size_t y = 0; y < h; ++y) {
    for (std::size_t x = 0; x < w; ++x) {
      Color c = fb.get_pixel(x, y, Orientation::Portrait0);
      auto rgb_val = ColorManager::to_rgb(c);
      rgb.push_back(rgb_val.r);
      rgb.push_back(rgb_val.g);
      rgb.push_back(rgb_val.b);
    }
  }
  return rgb;
}

} // namespace epaper
