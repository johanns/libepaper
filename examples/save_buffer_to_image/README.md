# BMP Debug Example

A demonstration of the BMP export feature for debugging display layouts without hardware.

## Overview

This example shows how to export framebuffer contents to BMP files for instant visual feedback during development. This is incredibly useful for:

- **Rapid iteration**: Test layouts without waiting for slow display refresh (~2 seconds)
- **Hardware-free development**: Work on layouts without e-paper display connected
- **Perfect accuracy**: BMP shows exactly what will appear on display (with orientation)
- **Easy sharing**: Review layouts on any device that can view images

## Features Demonstrated

- Exporting framebuffer to BMP format
- Text layout and positioning
- Graphics primitives (rectangles, circles, lines)
- Font sizes and rendering
- Number and decimal formatting

## Building

```bash
# From project root
cmake --build build --target bmp_debug_example

# Or build all examples
cmake --build build --target examples
```

## Running

```bash
# Run the example
./build/examples/bmp_debug_example/bmp_debug_example
```

**Output:**
- `test1_text.bmp` - Text layout example
- `test2_shapes.bmp` - Graphics and shapes
- `test3_fonts.bmp` - All font sizes
- `test4_numbers.bmp` - Number formatting

## Usage in Your Code

```cpp
#include <epaper/core/device.hpp>
#include <epaper/core/display.hpp>
#include <epaper/drivers/epd27.hpp>

// Initialize display
Device device;
device.init();
auto display = create_display<EPD27, MonoFramebuffer>(device, DisplayMode::BlackWhite,
                                     Orientation::Landscape90).value();

// Draw your layout
display.clear(Color::White);
display.draw(
    display.text("Test Layout")
        .at(10, 10)
        .font(&Font::font16())
        .foreground(Color::Black)
        .background(Color::White)
        .build()
);
display.draw(
    display.rectangle()
        .top_left(5, 5)
        .bottom_right(250, 170)
        .color(Color::Black)
        .fill(DrawFill::Empty)
        .build()
);

// Export to PNG for instant inspection (no hardware needed!)
auto result = display.save_framebuffer_to_png("debug_layout.png");
if (!result) {
    std::cerr << "PNG export failed: " << result.error().what() << "\n";
}

// Optional: refresh to physical display (slow, ~2 seconds)
display.refresh();
```

## Workflow

### Development Workflow

1. **Design**: Draw layout in code
2. **Export**: Save to PNG with one line
3. **Review**: Open PNG on your computer
4. **Iterate**: Adjust layout, repeat steps 2-3
5. **Deploy**: When perfect, `refresh()` to display

### Example: Testing Multiple Layouts

```cpp
// Rapidly test different layouts
for (int i = 0; i < 5; ++i) {
    display.clear(Color::White);
    render_layout_variant(display, i);
    display.save_framebuffer_to_png("layout_" + std::to_string(i) + ".png");
}

// Review all PNGs, pick the best one
// Then deploy to hardware with just ONE slow refresh
display.clear(Color::White);
render_layout_variant(display, best_layout);
display.refresh();  // Only refresh once!
```

## Color Mapping

The PNG export maps e-paper colors to RGB:

**BlackWhite mode:**
- `Color::Black` → RGB(0, 0, 0)
- `Color::White` → RGB(255, 255, 255)

**Grayscale4 mode:**
- `Color::Black` → RGB(0, 0, 0)
- `Color::Gray2` → RGB(85, 85, 85)
- `Color::Gray1` → RGB(170, 170, 170)
- `Color::White` → RGB(255, 255, 255)

**Color modes (BWR, BWY, Spectra6):**
- `Color::Red` → RGB(255, 0, 0)
- `Color::Yellow` → RGB(255, 255, 0)
- `Color::Blue` → RGB(0, 0, 255)
- `Color::Green` → RGB(0, 255, 0)

## Benefits

✅ **Instant feedback**: See layout immediately without hardware
✅ **Fast iteration**: Test multiple designs in seconds
✅ **Accurate**: Shows exactly what display will show
✅ **Universal**: View PNG on any device
✅ **Efficient**: Save hours of development time
✅ **Full color support**: Export color displays with accurate color rendering

## See Also

- [API Documentation](../../docs/API.md#debugging--troubleshooting) - Complete PNG export documentation
- [Examples README](../README.md) - Other example applications
- [Crypto Dashboard](../crypto_dashboard/) - Real-world usage example
