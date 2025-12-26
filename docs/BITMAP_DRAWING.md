# Bitmap Drawing Guide

This guide explains how to use the bitmap drawing functionality in the e-Paper library.

## Overview

The library now supports drawing bitmaps (images) to the e-paper display with the following features:

- **Raw pixel data**: Draw from arrays of `Color` values
- **Image file loading**: Load PNG, BMP, JPEG, and other formats
- **Scaling**: Resize images using nearest-neighbor interpolation
- **Automatic color conversion**: RGB/RGBA images are converted to grayscale
- **Bounds checking**: Automatic clipping for images that extend beyond screen boundaries

## API Reference

### Drawing from Raw Pixel Data

```cpp
auto draw_bitmap(std::size_t x, std::size_t y,
                 std::span<const Color> pixels,
                 std::size_t bitmap_width, std::size_t bitmap_height,
                 std::size_t target_width = 0, std::size_t target_height = 0) -> void;
```

**Parameters:**
- `x`, `y`: Top-left corner coordinates on the screen
- `pixels`: Span of `Color` values representing the bitmap (row-major order)
- `bitmap_width`, `bitmap_height`: Original dimensions of the bitmap
- `target_width`, `target_height`: Target dimensions for scaling (0 = no scaling)

**Example:**

```cpp
// Create a simple 8x8 checkerboard pattern
std::vector<Color> pattern;
for (std::size_t y = 0; y < 8; ++y) {
    for (std::size_t x = 0; x < 8; ++x) {
        bool is_white = ((x / 2) + (y / 2)) % 2 == 0;
        pattern.push_back(is_white ? Color::White : Color::Black);
    }
}

// Draw at original size
draw.draw_bitmap(10, 10, pattern, 8, 8);

// Draw scaled 4x
draw.draw_bitmap(30, 10, pattern, 8, 8, 32, 32);
```

### Drawing from Image Files

```cpp
auto draw_bitmap_from_file(std::size_t x, std::size_t y,
                           std::string_view file_path,
                           std::size_t target_width = 0, std::size_t target_height = 0)
    -> std::expected<void, BitmapError>;
```

**Parameters:**
- `x`, `y`: Top-left corner coordinates on the screen
- `file_path`: Path to the image file
- `target_width`, `target_height`: Target dimensions for scaling (0 = original size)

**Returns:**
- `std::expected<void, BitmapError>`: Success or error code

**Error Codes:**
- `BitmapError::FileNotFound`: Image file doesn't exist
- `BitmapError::InvalidFormat`: Unsupported image format
- `BitmapError::LoadFailed`: Failed to load image (corrupted file, etc.)
- `BitmapError::InvalidDimensions`: Image has invalid dimensions

**Example:**

```cpp
// Load and draw an image at original size
if (auto result = draw.draw_bitmap_from_file(10, 10, "logo.png"); !result) {
    std::cerr << "Failed to load image\n";
}

// Load and draw scaled to 100x100 pixels
if (auto result = draw.draw_bitmap_from_file(10, 10, "photo.jpg", 100, 100); !result) {
    std::cerr << "Failed to load image\n";
}
```

## Supported Image Formats

The library uses [stb_image](https://github.com/nothings/stb) for image loading, which supports:

- PNG
- JPEG/JPG
- BMP
- TGA
- GIF (first frame only)
- PSD (composited view only)
- HDR
- PIC
- PNM (PPM and PGM)

## Color Conversion

Images are automatically converted to match the display mode:

### Black & White Mode

RGB pixels are converted to grayscale using the standard luminance formula:
```
gray = 0.299 * R + 0.587 * G + 0.114 * B
```

Then thresholded at 128:
- `gray >= 128` → `Color::White`
- `gray < 128` → `Color::Black`

### 4-Level Grayscale Mode

The grayscale value is mapped to four levels:
- `gray >= 192` → `Color::White`
- `128 <= gray < 192` → `Color::Gray1`
- `64 <= gray < 128` → `Color::Gray2`
- `gray < 64` → `Color::Black`

## Scaling

Scaling uses **nearest-neighbor interpolation**, which is fast and produces sharp edges suitable for e-paper displays.

### Scaling Up (Enlarging)

```cpp
// Original 32x32 image scaled to 64x64
draw.draw_bitmap(10, 10, pixels, 32, 32, 64, 64);
```

### Scaling Down (Shrinking)

```cpp
// Original 200x200 image scaled to 100x100
draw.draw_bitmap(10, 10, pixels, 200, 200, 100, 100);
```

### Aspect Ratio

The library does not automatically preserve aspect ratio. To maintain aspect ratio:

```cpp
// Calculate scaled dimensions maintaining aspect ratio
std::size_t original_width = 200;
std::size_t original_height = 150;
std::size_t max_width = 100;

double scale = static_cast<double>(max_width) / original_width;
std::size_t scaled_height = static_cast<std::size_t>(original_height * scale);

draw.draw_bitmap_from_file(10, 10, "image.png", max_width, scaled_height);
```

## Performance Considerations

1. **Memory**: Loading large images requires memory for the entire decoded image
2. **Scaling**: Nearest-neighbor is fast but may produce blocky results for photographic images
3. **Refresh Rate**: E-paper displays are slow; minimize full-screen refreshes
4. **File I/O**: Loading from files is slower than using pre-loaded pixel data

## Best Practices

### For Icons and UI Elements

```cpp
// Pre-load small icons as const arrays
const std::array<Color, 16*16> battery_icon = {
    // ... icon data ...
};

draw.draw_bitmap(150, 5, battery_icon, 16, 16);
```

### For Photos and Complex Images

```cpp
// Load once, draw multiple times if needed
auto load_and_cache = [](const std::string& path) {
    int width, height, channels;
    auto* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    // ... convert to Color vector ...
    return std::make_tuple(pixels, width, height);
};

auto [pixels, w, h] = load_and_cache("photo.jpg");
draw.draw_bitmap(10, 10, pixels, w, h, 100, 100);
```

### For Animations or Sequences

```cpp
// Pre-scale images to display size to avoid runtime scaling
std::vector<std::vector<Color>> frames;
for (const auto& frame_path : frame_paths) {
    // Load and pre-scale each frame
    // ...
}

// Fast playback
for (const auto& frame : frames) {
    draw.draw_bitmap(0, 0, frame, width, height);
    screen.refresh();
}
```

## Complete Example

```cpp
#include <epaper/device.hpp>
#include <epaper/draw.hpp>
#include <epaper/drivers/epd27.hpp>
#include <epaper/screen.hpp>
#include <iostream>

using namespace epaper;

int main() {
    // Initialize hardware
    Device device;
    device.init().value();

    EPD27 epd27(device);
    epd27.init(DisplayMode::Grayscale).value();
    epd27.clear();

    Screen screen(epd27);
    Draw draw(screen);

    // Draw a logo from file
    if (auto result = draw.draw_bitmap_from_file(10, 10, "logo.png", 80, 80); !result) {
        std::cerr << "Failed to load logo\n";
        return 1;
    }

    // Create a custom pattern
    std::vector<Color> gradient;
    for (std::size_t y = 0; y < 50; ++y) {
        for (std::size_t x = 0; x < 100; ++x) {
            // Horizontal gradient
            std::uint8_t intensity = (x * 255) / 100;
            Color c = intensity < 64 ? Color::Black :
                     intensity < 128 ? Color::Gray2 :
                     intensity < 192 ? Color::Gray1 : Color::White;
            gradient.push_back(c);
        }
    }

    draw.draw_bitmap(10, 100, gradient, 100, 50);

    // Refresh display
    screen.refresh();

    // Cleanup
    device.delay_ms(5000);
    epd27.sleep();

    return 0;
}
```

## Troubleshooting

### Image Not Appearing

1. Check file path is correct and accessible
2. Verify image format is supported
3. Ensure coordinates are within screen bounds
4. Call `screen.refresh()` after drawing

### Image Appears Corrupted

1. Verify pixel data size matches `width * height`
2. Check that pixels are in row-major order
3. Ensure Color values are valid

### Poor Image Quality

1. Try different scaling factors
2. Pre-process images for better contrast
3. Use dithering for photographic images (external tool)
4. Consider using 4-level grayscale mode instead of black/white

### Performance Issues

1. Pre-scale images to display size offline
2. Cache loaded images instead of reloading
3. Use raw pixel data instead of file loading for frequently used images
4. Minimize the number of `screen.refresh()` calls

## Integration with Existing Code

The bitmap drawing functions integrate seamlessly with existing drawing operations:

```cpp
// Clear screen
screen.clear(Color::White);

// Draw shapes
draw.draw_rectangle(0, 0, 175, 50, Color::Black, DotPixel::Pixel2x2);

// Draw bitmap
draw.draw_bitmap_from_file(10, 10, "icon.png", 32, 32);

// Draw text
draw.draw_string(50, 20, "Status: OK", Font::font16(), Color::Black, Color::White);

// Refresh once
screen.refresh();
```

## License

The bitmap drawing functionality uses [stb_image](https://github.com/nothings/stb) by Sean Barrett, which is public domain or MIT licensed.

