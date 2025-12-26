# Bitmap Drawing - Quick Reference

## Include Headers

```cpp
#include <epaper/draw.hpp>
#include <epaper/screen.hpp>
```

## Basic Usage

### Draw from Raw Pixel Data

```cpp
std::vector<Color> pixels = { /* your pixel data */ };
draw.draw_bitmap(x, y, pixels, width, height);
```

### Draw from Image File

```cpp
if (auto result = draw.draw_bitmap_from_file(x, y, "image.png"); !result) {
    // Handle error
}
```

### Draw with Scaling

```cpp
// Scale to specific size
draw.draw_bitmap(x, y, pixels, orig_width, orig_height, target_width, target_height);

// Scale from file
draw.draw_bitmap_from_file(x, y, "image.png", target_width, target_height);
```

## Error Handling

```cpp
auto result = draw.draw_bitmap_from_file(x, y, "image.png");
if (!result) {
    switch (result.error()) {
        case BitmapError::FileNotFound:
            // File doesn't exist
            break;
        case BitmapError::LoadFailed:
            // Failed to load (corrupted, etc.)
            break;
        case BitmapError::InvalidFormat:
            // Unsupported format
            break;
        case BitmapError::InvalidDimensions:
            // Invalid image dimensions
            break;
    }
}
```

## Supported Formats

- PNG
- JPEG/JPG
- BMP
- TGA
- GIF (first frame)
- PSD
- HDR
- PIC
- PNM (PPM/PGM)

## Color Values

```cpp
Color::White  // Lightest
Color::Gray1  // Light gray (grayscale mode only)
Color::Gray2  // Dark gray (grayscale mode only)
Color::Black  // Darkest
```

## Common Patterns

### Checkerboard

```cpp
std::vector<Color> pattern;
for (std::size_t y = 0; y < size; ++y) {
    for (std::size_t x = 0; x < size; ++x) {
        pattern.push_back(((x/4) + (y/4)) % 2 ? Color::Black : Color::White);
    }
}
draw.draw_bitmap(x, y, pattern, size, size);
```

### Gradient

```cpp
std::vector<Color> gradient;
for (std::size_t y = 0; y < height; ++y) {
    for (std::size_t x = 0; x < width; ++x) {
        std::uint8_t intensity = (x * 255) / width;
        Color c = intensity < 64 ? Color::Black :
                 intensity < 128 ? Color::Gray2 :
                 intensity < 192 ? Color::Gray1 : Color::White;
        gradient.push_back(c);
    }
}
draw.draw_bitmap(x, y, gradient, width, height);
```

### Load and Scale

```cpp
// Maintain aspect ratio
double scale = static_cast<double>(target_width) / original_width;
std::size_t scaled_height = static_cast<std::size_t>(original_height * scale);
draw.draw_bitmap_from_file(x, y, "image.png", target_width, scaled_height);
```

## Performance Tips

1. **Pre-scale images** offline when possible
2. **Cache loaded images** instead of reloading
3. **Use raw pixel data** for frequently used images
4. **Minimize refreshes** - draw multiple bitmaps before calling `screen.refresh()`

## Complete Example

```cpp
#include <epaper/device.hpp>
#include <epaper/draw.hpp>
#include <epaper/drivers/epd27.hpp>
#include <epaper/screen.hpp>

using namespace epaper;

int main() {
    Device device;
    device.init().value();

    EPD27 epd27(device);
    epd27.init(DisplayMode::Grayscale).value();

    Screen screen(epd27);
    Draw draw(screen);

    // Draw from file
    draw.draw_bitmap_from_file(10, 10, "logo.png", 80, 80);

    // Draw custom pattern
    std::vector<Color> pattern(64 * 64, Color::Black);
    draw.draw_bitmap(100, 10, pattern, 64, 64);

    screen.refresh();
    epd27.sleep();

    return 0;
}
```

## Build and Run

```bash
# Build
cmake --build build

# Run example
sudo ./build/examples/bitmap_example
```

## See Also

- **Full Documentation**: [BITMAP_DRAWING.md](BITMAP_DRAWING.md)
- **Implementation Details**: [BITMAP_IMPLEMENTATION_SUMMARY.md](BITMAP_IMPLEMENTATION_SUMMARY.md)
- **Main README**: [README.md](../README.md)

