# API Reference

A comprehensive guide to using the libepaper API for e-paper display control.

## Table of Contents

- [Getting Started](#getting-started)
- [Display Creation & Management](#display-creation--management)
- [Drawing Operations](#drawing-operations)
- [Color & Grayscale](#color--grayscale)
- [Display Refresh & Power Management](#display-refresh--power-management)
- [Error Handling](#error-handling)
- [Advanced Topics](#advanced-topics)
- [Best Practices](#best-practices)
- [Common Patterns](#common-patterns)
- [Troubleshooting](#troubleshooting)

## Getting Started

### Requirements

- Raspberry Pi with SPI enabled
- C++23 capable compiler (GCC 14+ or Clang 18+)
- libgpiod library installed (`sudo apt install libgpiod-dev`)
- User in `gpio` and `spi` groups (no sudo required)

### Basic Setup

```cpp
#include <epaper/device.hpp>
#include <epaper/display.hpp>
#include <epaper/drivers/epd27.hpp>
#include <epaper/font.hpp>

using namespace epaper;

int main() {
    // 1. Initialize hardware device (with optional custom config)
    Device device;  // Uses default DeviceConfig
    // Or customize:
    // DeviceConfig config{
    //     .gpio_chip = "/dev/gpiochip0",
    //     .spi_device = "/dev/spidev0.0",
    //     .spi_speed_hz = 1953125
    // };
    // Device device{config};

    if (auto result = device.init(); !result) {
        std::cerr << "Device init failed: " << result.error().what() << "\n";
        return 1;
    }

    // 2. Create display (auto-sleep enabled by default)
    auto display = create_display<EPD27>(
        device,
        DisplayMode::BlackWhite,    // or DisplayMode::Grayscale4
        Orientation::Portrait0,      // or Landscape90, Portrait180, Landscape270
        true                         // auto-sleep enabled (recommended)
    );

    if (!display) {
        std::cerr << "Display creation failed: " << display.error().what() << "\n";
        return 1;
    }

    // 3. Draw something using builder pattern
    display->clear(Color::White);
    display->draw(
        display->text("Hello World!")
            .at(10, 10)
            .font(&Font::font16())
            .foreground(Color::Black)
            .background(Color::White)
            .build()
    );

    // 4. Refresh display (will auto-sleep after)
    if (auto result = display->refresh(); !result) {
        std::cerr << "Refresh failed: " << result.error().what() << "\n";
        return 1;
    }

    return 0;
}
```

### Your First Program

Here's a complete minimal example that draws a rectangle with text:

```cpp
#include <epaper/device.hpp>
#include <epaper/display.hpp>
#include <epaper/drivers/epd27.hpp>
#include <epaper/font.hpp>
#include <iostream>

using namespace epaper;

int main() {
    // Initialize device
    Device device;
    if (auto result = device.init(); !result) {
        return 1;
    }

    // Create display in landscape orientation
    auto display = create_display<EPD27>(
        device,
        DisplayMode::BlackWhite,
        Orientation::Landscape90
    );

    if (!display) {
        return 1;
    }

    // Clear screen to white
    display->clear(Color::White);

    // Draw a border rectangle
    display->draw(
        display->rectangle()
            .top_left(0, 0)
            .bottom_right(display->effective_width() - 1,
                         display->effective_height() - 1)
            .color(Color::Black)
            .fill(DrawFill::Empty)
            .build()
    );

    // Draw title
    display->draw(
        display->text("My First E-Paper App")
            .at(10, 10)
            .font(&Font::font20())
            .foreground(Color::Black)
            .background(Color::White)
            .build()
    );

    // Draw a filled rectangle
    display->draw(
        display->rectangle()
            .top_left(10, 40)
            .bottom_right(100, 80)
            .color(Color::Black)
            .fill(DrawFill::Full)
            .build()
    );

    // Draw text on black background
    display->draw(
        display->text("libepaper")
            .at(15, 50)
            .font(&Font::font12())
            .foreground(Color::White)
            .background(Color::Black)
            .build()
    );

    // Refresh display
    if (auto result = display->refresh(); !result) {
        std::cerr << "Refresh failed\n";
        return 1;
    }

    std::cout << "Display updated successfully!\n";
    return 0;
}
```

**Compile and run:**
```bash
# Build
cmake --build build

# Run (no sudo required with proper group membership)
./build/your_program
```

## Display Creation & Management

### Creating a Display

Use the factory function `create_display<Driver>()` to create displays:

```cpp
#include <epaper/display.hpp>
#include <epaper/drivers/epd27.hpp>

auto display = create_display<EPD27>(
    device,                      // Hardware device
    DisplayMode::BlackWhite,     // Display mode
    Orientation::Landscape90,    // Orientation
    true                         // Auto-sleep enabled
);

// Returns std::expected<std::unique_ptr<Display>, Error>
if (!display) {
    std::cerr << "Failed: " << display.error().what() << "\n";
    return;
}

// Use with -> operator
display->draw(display->point().at(10, 10).color(Color::Black).build());
```

### Display Modes

The library supports two display modes:

#### BlackWhite Mode (1 bit per pixel)

Pure binary output - only black and white pixels.

```cpp
auto display = create_display<EPD27>(
    device,
    DisplayMode::BlackWhite,
    Orientation::Portrait0
);

// Only these colors work in BlackWhite mode:
display->draw(display->point().at(10, 10).color(Color::White).build());  // White pixel
display->draw(display->point().at(20, 20).color(Color::Black).build());  // Black pixel

// Gray colors will be converted to black
display->draw(display->point().at(30, 30).color(Color::Gray1).build());  // → Black
display->draw(display->point().at(40, 40).color(Color::Gray2).build());  // → Black
```

**When to use:**
- Highest contrast output
- Text-heavy displays
- Smaller memory footprint (5,808 bytes for 2.7")
- Faster SPI transfers

#### Grayscale4 Mode (2 bits per pixel)

Four-level grayscale output.

```cpp
auto display = create_display<EPD27>(
    device,
    DisplayMode::Grayscale4,
    Orientation::Portrait0
);

// All four gray levels available:
display->draw(display->point().at(10, 10).color(Color::White).build());  // Lightest
display->draw(display->point().at(20, 20).color(Color::Gray1).build());  // Light gray
display->draw(display->point().at(30, 30).color(Color::Gray2).build());  // Dark gray
display->draw(display->point().at(40, 40).color(Color::Black).build());  // Darkest
```

**When to use:**
- Images and photographs
- Charts with multiple data series
- Anti-aliased graphics
- More nuanced visual output

**Trade-offs:**
- Double memory usage (11,616 bytes for 2.7")
- Slightly slower SPI transfer
- Better visual quality

### Orientation

The library supports four orientations with automatic coordinate transformation:

```cpp
// Portrait (default) - 176x264 pixels
auto display1 = create_display<EPD27>(device, DisplayMode::BlackWhite,
                                     Orientation::Portrait0);

// Landscape (90° clockwise) - 264x176 pixels
auto display2 = create_display<EPD27>(device, DisplayMode::BlackWhite,
                                     Orientation::Landscape90);

// Upside-down portrait (180°) - 176x264 pixels
auto display3 = create_display<EPD27>(device, DisplayMode::BlackWhite,
                                     Orientation::Portrait180);

// Landscape counter-clockwise (270°) - 264x176 pixels
auto display4 = create_display<EPD27>(device, DisplayMode::BlackWhite,
                                     Orientation::Landscape270);
```

**Coordinate Transformation:**

The coordinate system is automatically transformed. Point (0, 0) is always the top-left corner in the *rotated* coordinate space.

```cpp
auto display = create_display<EPD27>(device, DisplayMode::BlackWhite,
                                     Orientation::Landscape90);

// Get effective dimensions (accounts for rotation)
auto width = display->effective_width();    // 264 (rotated!)
auto height = display->effective_height();  // 176 (rotated!)

// Draw at "top-left" in rotated space
display->draw(display->point().at(0, 0).color(Color::Black).build());  // Appears at actual top-left

// All drawing operations use rotated coordinates
display->draw(
    display->rectangle()
        .top_left(0, 0)
        .bottom_right(width - 1, height - 1)
        .color(Color::Black)
        .fill(DrawFill::Empty)
        .build()
);
```

**Orientation Summary:**

| Orientation | Rotation | Width×Height | Top-Left (0,0) Maps To |
|-------------|----------|--------------|----------------------|
| Portrait0 | 0° | 176×264 | Physical top-left |
| Landscape90 | 90° CW | 264×176 | Physical top-right |
| Portrait180 | 180° | 176×264 | Physical bottom-right |
| Landscape270 | 270° CW | 264×176 | Physical bottom-left |

## Drawing Operations

All drawing operations are **noexcept** and never fail. Out-of-bounds coordinates are silently clipped.

### Points

```cpp
// Draw a single pixel
display->draw(display->point().at(x, y).color(Color::Black).build());

// Draw a larger point (2x2, 3x3, 4x4, 5x5 pixels)
display->draw(display->point().at(x, y).color(Color::Black).size(DotPixel::Pixel2x2).build());
display->draw(display->point().at(x, y).color(Color::Black).size(DotPixel::Pixel3x3).build());
display->draw(display->point().at(x, y).color(Color::Black).size(DotPixel::Pixel4x4).build());
display->draw(display->point().at(x, y).color(Color::Black).size(DotPixel::Pixel5x5).build());
```

**DotPixel enum:**
- `Pixel1x1`: Single pixel (default)
- `Pixel2x2`: 2×2 pixel block
- `Pixel3x3`: 3×3 pixel block
- `Pixel4x4`: 4×4 pixel block
- `Pixel5x5`: 5×5 pixel block

### Lines

```cpp
// Draw a line from (x1, y1) to (x2, y2)
display->draw(
    display->line()
        .from(x1, y1)
        .to(x2, y2)
        .color(Color::Black)
        .build()
);

// Thicker lines
display->draw(
    display->line()
        .from(x1, y1)
        .to(x2, y2)
        .color(Color::Black)
        .width(DotPixel::Pixel2x2)
        .build()
);

// Dotted lines
display->draw(
    display->line()
        .from(x1, y1)
        .to(x2, y2)
        .color(Color::Black)
        .width(DotPixel::Pixel1x1)
        .style(LineStyle::Dotted)
        .build()
);
```

**LineStyle enum:**
- `Solid`: Continuous line (default)
- `Dotted`: Dot every 4 pixels

**Examples:**
```cpp
// Draw a grid
for (size_t x = 0; x < display->effective_width(); x += 20) {
    display->draw(
        display->line()
            .from(x, 0)
            .to(x, display->effective_height() - 1)
            .color(Color::Black)
            .build()
    );
}
for (size_t y = 0; y < display->effective_height(); y += 20) {
    display->draw(
        display->line()
            .from(0, y)
            .to(display->effective_width() - 1, y)
            .color(Color::Black)
            .build()
    );
}
```

### Rectangles

```cpp
// Draw a rectangle outline
display->draw(
    display->rectangle()
        .top_left(x, y)
        .bottom_right(width, height)
        .color(Color::Black)
        .fill(DrawFill::Empty)
        .build()
);

// Draw a filled rectangle
display->draw(
    display->rectangle()
        .top_left(x, y)
        .bottom_right(width, height)
        .color(Color::Black)
        .fill(DrawFill::Full)
        .build()
);

// Thicker borders
display->draw(
    display->rectangle()
        .top_left(x, y)
        .bottom_right(width, height)
        .color(Color::Black)
        .fill(DrawFill::Empty)
        .border_width(DotPixel::Pixel2x2)
        .build()
);
```

**Parameters:**
- `x, y`: Top-left corner
- `width, height`: Rectangle dimensions
- `color`: Color to draw
- `fill`: `DrawFill::Empty` (outline) or `DrawFill::Full` (filled)
- `pixel`: Line thickness (default: `Pixel1x1`)

**Examples:**
```cpp
// Draw a button
display->draw(
    display->rectangle()
        .top_left(10, 10)
        .bottom_right(100, 40)
        .color(Color::Black)
        .fill(DrawFill::Empty)
        .build()
);
display->draw(
    display->text("OK")
        .at(30, 20)
        .font(&Font::font16())
        .foreground(Color::Black)
        .background(Color::White)
        .build()
);

// Draw a filled header bar
display->draw(
    display->rectangle()
        .top_left(0, 0)
        .bottom_right(display->effective_width(), 30)
        .color(Color::Black)
        .fill(DrawFill::Full)
        .build()
);
display->draw(
    display->text("Dashboard")
        .at(10, 8)
        .font(&Font::font16())
        .foreground(Color::White)
        .background(Color::Black)
        .build()
);
```

### Circles

```cpp
// Draw a circle outline
display->draw(
    display->circle()
        .center(center_x, center_y)
        .radius(radius)
        .color(Color::Black)
        .fill(DrawFill::Empty)
        .build()
);

// Draw a filled circle
display->draw(
    display->circle()
        .center(center_x, center_y)
        .radius(radius)
        .color(Color::Black)
        .fill(DrawFill::Full)
        .build()
);

// Thicker outline
display->draw(
    display->circle()
        .center(center_x, center_y)
        .radius(radius)
        .color(Color::Black)
        .fill(DrawFill::Empty)
        .border_width(DotPixel::Pixel2x2)
        .build()
);
```

**Examples:**
```cpp
// Draw a bullseye
display->draw(display->circle().center(88, 132).radius(50).color(Color::Black).fill(DrawFill::Empty).build());
display->draw(display->circle().center(88, 132).radius(40).color(Color::Black).fill(DrawFill::Empty).build());
display->draw(display->circle().center(88, 132).radius(30).color(Color::Black).fill(DrawFill::Empty).build());
display->draw(display->circle().center(88, 132).radius(10).color(Color::Black).fill(DrawFill::Full).build());

// Draw a status indicator (filled dot)
if (connected) {
    display->draw(
        display->circle()
            .center(250, 10)
            .radius(5)
            .color(Color::Black)
            .fill(DrawFill::Full)
            .build()
    );
}
```

### Text & Fonts

```cpp
// Draw text
display->draw(
    display->text("Hello World!")
        .at(x, y)
        .font(&Font::font16())
        .foreground(Color::Black)
        .background(Color::White)
        .build()
);

// Different font sizes
display->draw(display->text("Tiny").at(10, 10).font(&Font::font8()).foreground(Color::Black).background(Color::White).build());
display->draw(display->text("Small").at(10, 30).font(&Font::font12()).foreground(Color::Black).background(Color::White).build());
display->draw(display->text("Medium").at(10, 50).font(&Font::font16()).foreground(Color::Black).background(Color::White).build());
display->draw(display->text("Large").at(10, 80).font(&Font::font20()).foreground(Color::Black).background(Color::White).build());
display->draw(display->text("XLarge").at(10, 110).font(&Font::font24()).foreground(Color::Black).background(Color::White).build());
```

**Available Fonts:**
- `Font::font8()`: 5×8 pixels per character
- `Font::font12()`: 7×12 pixels per character
- `Font::font16()`: 11×16 pixels per character
- `Font::font20()`: 14×20 pixels per character
- `Font::font24()`: 17×24 pixels per character

**Foreground/Background Colors:**
```cpp
// Black text on white background
display->draw(
    display->text("Normal")
        .at(10, 10)
        .font(&Font::font16())
        .foreground(Color::Black)
        .background(Color::White)
        .build()
);

// White text on black background (inverse)
display->draw(
    display->text("Inverse")
        .at(10, 30)
        .font(&Font::font16())
        .foreground(Color::White)
        .background(Color::Black)
        .build()
);

// Gray text (in grayscale mode)
display->draw(
    display->text("Gray")
        .at(10, 50)
        .font(&Font::font16())
        .foreground(Color::Gray2)
        .background(Color::White)
        .build()
);
```

**Multi-line Text:**
```cpp
// Manual line spacing
const size_t x = 10;
size_t y = 10;
const size_t line_height = 20;

display->draw(display->text("Line 1").at(x, y).font(&Font::font16()).foreground(Color::Black).background(Color::White).build());
y += line_height;
display->draw(display->text("Line 2").at(x, y).font(&Font::font16()).foreground(Color::Black).background(Color::White).build());
y += line_height;
display->draw(display->text("Line 3").at(x, y).font(&Font::font16()).foreground(Color::Black).background(Color::White).build());
```

### Numbers

```cpp
// Draw an integer using TextBuilder
int value = 42;
display->draw(
    display->text()
        .at(x, y)
        .number(value)
        .font(&Font::font16())
        .foreground(Color::Black)
        .background(Color::White)
        .build()
);

// Draw a decimal with precision
double pi = 3.14159;
display->draw(
    display->text()
        .at(x, y)
        .decimal(pi, 3)  // 3 decimal places
        .font(&Font::font16())
        .foreground(Color::Black)
        .background(Color::White)
        .build()
);
// Displays: "3.141"

// Right-aligned numbers (for tables)
int price = 199;
display->draw(
    display->text()
        .at(200, y)
        .number(price)
        .font(&Font::font12())
        .foreground(Color::Black)
        .background(Color::White)
        .build()
);
```

**Formatting Examples:**
```cpp
// Display sensor readings
double temperature = 23.5;
display->draw(
    display->text("Temperature:")
        .at(10, 10)
        .font(&Font::font12())
        .foreground(Color::Black)
        .background(Color::White)
        .build()
);
display->draw(
    display->text()
        .at(110, 10)
        .decimal(temperature, 1)
        .font(&Font::font12())
        .foreground(Color::Black)
        .background(Color::White)
        .build()
);
display->draw(
    display->text("C")
        .at(150, 10)
        .font(&Font::font12())
        .foreground(Color::Black)
        .background(Color::White)
        .build()
);

// Display prices
double price = 19.99;
display->draw(
    display->text("$")
        .at(10, 30)
        .font(&Font::font16())
        .foreground(Color::Black)
        .background(Color::White)
        .build()
);
display->draw(
    display->text()
        .at(20, 30)
        .decimal(price, 2)
        .font(&Font::font16())
        .foreground(Color::Black)
        .background(Color::White)
        .build()
);
```

### Bitmaps

```cpp
// Draw from raw pixel data
std::vector<Color> pixels = {
    Color::Black, Color::White, Color::Black, Color::White,
    Color::White, Color::Black, Color::White, Color::Black,
    Color::Black, Color::White, Color::Black, Color::White,
    Color::White, Color::Black, Color::White, Color::Black
};
display->draw_bitmap(10, 10, pixels, 4, 4);  // 4×4 checkerboard

// Draw from image file (PNG, JPEG, BMP, etc.)
auto result = display->draw_bitmap_from_file(10, 50, "logo.png");
if (!result) {
    std::cerr << "Failed to load image: " << result.error().what() << "\n";
}

// Draw with scaling
display->draw_bitmap_from_file(10, 100, "photo.jpg", 200, 150);  // Scale to 200×150
```

**Supported Image Formats:**
- PNG (including transparency)
- JPEG
- BMP
- TGA
- GIF (first frame only)
- PSD (composited view)
- HDR

**Automatic Color Conversion:**

The library automatically converts color images to your display's mode:

1. **RGB to Grayscale**: Uses standard perceptual formula (0.299×R + 0.587×G + 0.114×B)
2. **Quantization**:
   - **BlackWhite mode**: 50% brightness threshold (gray ≥ 128 → White)
   - **Grayscale4 mode**: 4-level mapping (0-63→Black, 64-127→Gray2, 128-191→Gray1, 192-255→White)

This means you can load **any color image** (photos, graphics, screenshots) and the library intelligently converts it to work with your e-paper display:

```cpp
// Load a full-color photograph - automatically converted!
display->draw_bitmap_from_file(10, 10, "vacation_photo.jpg");

// Load a colorful logo - automatically converted!
display->draw_bitmap_from_file(100, 50, "company_logo.png");

// No manual color processing needed!
```

## Color & Grayscale

### Color Enum

```cpp
enum class Color {
    White,  // Value: 0 (lightest)
    Gray1,  // Value: 1 (light gray, grayscale mode only)
    Gray2,  // Value: 2 (dark gray, grayscale mode only)
    Black   // Value: 3 (darkest)
};
```

### Color Mapping by Display Mode

| Color | BlackWhite Mode | Grayscale4 Mode |
|-------|----------------|----------------|
| White | White (0) | White (0) |
| Gray1 | Black (1) | Light gray (1) |
| Gray2 | Black (1) | Dark gray (2) |
| Black | Black (1) | Black (3) |

### Working with Grayscale

```cpp
auto display = create_display<EPD27>(device, DisplayMode::Grayscale4,
                                     Orientation::Portrait0);

// Draw a grayscale gradient
for (size_t x = 0; x < display->effective_width(); x++) {
    Color shade;
    if (x < 44) shade = Color::White;
    else if (x < 88) shade = Color::Gray1;
    else if (x < 132) shade = Color::Gray2;
    else shade = Color::Black;

    display->draw(
        display->line()
            .from(x, 0)
            .to(x, display->effective_height() - 1)
            .color(shade)
            .build()
    );
}

// Draw anti-aliased graphics
// (manually, by using intermediate gray levels at edges)
```

## Display Refresh & Power Management

### Refresh Behavior

Call `refresh()` to update the physical display with the framebuffer contents.

```cpp
// Draw operations modify the framebuffer
display->draw(
    display->text("Hello")
        .at(10, 10)
        .font(&Font::font16())
        .foreground(Color::Black)
        .background(Color::White)
        .build()
);
display->draw(
    display->circle()
        .center(88, 132)
        .radius(30)
        .color(Color::Black)
        .fill(DrawFill::Full)
        .build()
);

// Nothing appears on screen yet...

// Refresh to display
auto result = display->refresh();
if (!result) {
    std::cerr << "Refresh failed: " << result.error().what() << "\n";
}

// Now the screen shows the updates!
```

**Important:** `refresh()` is a **blocking operation** that takes ~2 seconds for the EPD27.

### Auto-Sleep (IMPORTANT!)

**The library automatically manages sleep/wake cycles to prevent screen burn-in.**

```cpp
// Create display with auto-sleep enabled (default and recommended)
auto display = create_display<EPD27>(device, DisplayMode::BlackWhite,
                                     Orientation::Portrait0, true);

// First render
display->draw(display->text("Frame 1").at(10, 10).font(&Font::font16()).foreground(Color::Black).background(Color::White).build());
display->refresh();  // Display automatically enters sleep mode after refresh

// ... minutes or hours later ...

// Second render - transparent wake!
display->draw(display->text("Frame 2").at(10, 10).font(&Font::font16()).foreground(Color::Black).background(Color::White).build());
display->refresh();  // Library auto-wakes if needed, then refreshes ✨

// No manual wake() calls needed!
```

**How It Works:**
1. After `refresh()`, display automatically sleeps (if auto-sleep enabled)
2. Driver internally tracks sleep state with `is_asleep_` flag
3. Next `refresh()` automatically calls `wake()` if asleep
4. `wake()` re-initializes the EPD27 hardware (~2 second delay)
5. Refresh proceeds normally

**Why Auto-Sleep?**
- **Prevents burn-in**: E-paper displays degrade if left in high voltage state
- **Follows manufacturer recommendation**: Waveshare recommends sleep between refreshes
- **Transparent**: Library handles everything automatically
- **Safe**: No risk of forgetting to sleep

**Benefits of Transparent Wake:**
- **Ergonomic**: No manual sleep/wake management
- **Correct**: Impossible to forget to wake
- **Simple**: Same code works for all scenarios

### Disabling Auto-Sleep

For special use cases where you need manual control:

```cpp
// Disable auto-sleep
auto display = create_display<EPD27>(device, DisplayMode::BlackWhite,
                                     Orientation::Portrait0, false);  // false = disabled

// Now you must manually manage sleep
display->refresh();  // Does NOT auto-sleep

// Manual sleep (when you're done)
display->sleep();

// Manual wake before next refresh
display->wake();  // Required! EPD27 needs re-init (~2s)
display->refresh();
```

**When to disable auto-sleep:**
- Rapid successive refreshes (e.g., animation, though not recommended for e-paper)
- Custom power management logic
- Benchmarking/testing

**Recommendation:** Keep auto-sleep **enabled** (default). It prevents hardware damage and the library handles wake automatically.

### Manual Power Control

The library also provides explicit power control methods:

```cpp
// Put display to sleep (usually automatic with auto-sleep)
display->sleep();

// Wake from sleep (usually automatic on next refresh)
display->wake();  // EPD27: ~2 second re-initialization

// Complete power off (if supported by hardware)
if (display->supports_power_control()) {
    display->power_off();  // Cut power
    // ... later ...
    display->power_on();   // Restore power, requires re-init
}
```

## Error Handling

### Using `std::expected`

All operations that can fail return `std::expected<T, Error>`:

```cpp
// Check for errors
auto result = display->refresh();
if (!result) {
    // Error occurred
    std::cerr << "Error: " << result.error().what() << "\n";
    std::cerr << "Code: " << static_cast<int>(result.error().code) << "\n";
} else {
    // Success
    std::cout << "Refresh successful!\n";
}

// Or use .value() (throws if error)
try {
    display->refresh().value();
} catch (const std::bad_expected_access<Error>& e) {
    std::cerr << "Error: " << e.error().what() << "\n";
}
```

### Common Errors

| Error Code | Description | Recovery Strategy |
|------------|-------------|-------------------|
| `DeviceNotInitialized` | Hardware not initialized | Call `device.init()` first |
| `DeviceInitFailed` | Device initialization failed | Check permissions, hardware connection, group membership |
| `DriverNotInitialized` | Driver not initialized | Call `driver.init(mode)` |
| `InvalidDisplayMode` | Unsupported display mode | Use `BlackWhite` or `Grayscale4` |
| `HardwareTimeout` | Display BUSY timeout | Check wiring, try again |
| `RefreshFailed` | Display refresh failed | Retry with backoff |
| `FileNotFound` | Image file not found | Check file path |
| `InvalidFormat` | Unsupported image format | Use PNG/JPEG/BMP |

### Error Recovery Strategies

**1. Retry with Exponential Backoff:**
```cpp
auto retry_refresh = [&](int max_attempts = 3) -> std::expected<void, Error> {
    for (int attempt = 1; attempt <= max_attempts; ++attempt) {
        auto result = display->refresh();
        if (result) {
            return {};  // Success
        }

        if (attempt < max_attempts) {
            std::cerr << "Attempt " << attempt << " failed, retrying...\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100 * attempt));
        } else {
            return result;  // Give up, return error
        }
    }
    return {};
};
```

**2. Fallback to Error Screen:**
```cpp
auto result = display->refresh();
if (!result) {
    // Clear and show error
    display->clear(Color::White);
    display->draw(
        display->text("Error:")
            .at(10, 10)
            .font(&Font::font16())
            .foreground(Color::Black)
            .background(Color::White)
            .build()
    );
    display->draw(
        display->text(result.error().what())
            .at(10, 30)
            .font(&Font::font12())
            .foreground(Color::Black)
            .background(Color::White)
            .build()
    );
    display->refresh();  // Try again with error message
}
```

**3. Graceful Degradation:**
```cpp
auto result = display->draw_bitmap_from_file(10, 10, "logo.png");
if (!result) {
    // Fall back to text
    display->draw(
        display->text("LOGO")
            .at(10, 10)
            .font(&Font::font24())
            .foreground(Color::Black)
            .background(Color::White)
            .build()
    );
}
```

## Advanced Topics

### Button Handling

The `Device` class provides GPIO operations that can be used to handle buttons on your e-paper display (e.g., Waveshare 2.7" includes KEY1, KEY2, KEY3 buttons).

**Basic Button Reading:**

```cpp
#include <epaper/device.hpp>
#include <thread>
#include <chrono>
#include <iostream>

using namespace epaper;

// Define button pins (check your hardware documentation for actual pin numbers)
namespace button_pins {
    constexpr Pin KEY1{5};   // Example: GPIO 5
    constexpr Pin KEY2{6};   // Example: GPIO 6
    constexpr Pin KEY3{13};  // Example: GPIO 13
}

int main() {
    Device device;
    device.init().value();

    // Configure buttons as inputs
    device.set_pin_input(button_pins::KEY1);
    device.set_pin_input(button_pins::KEY2);
    device.set_pin_input(button_pins::KEY3);

    // Poll button state
    while (true) {
        if (device.read_pin(button_pins::KEY1)) {
            std::cout << "KEY1 pressed\n";
            // Handle button press
        }
        if (device.read_pin(button_pins::KEY2)) {
            std::cout << "KEY2 pressed\n";
        }
        if (device.read_pin(button_pins::KEY3)) {
            std::cout << "KEY3 pressed\n";
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Debounce
    }
}
```

**Note:** Button pin numbers vary by hardware. Check your Waveshare display documentation for the correct GPIO pin assignments. The library provides the GPIO API; you implement the button handling logic.

### Framebuffer Direct Access

For advanced users who need direct pixel manipulation:

```cpp
// Access framebuffer directly
for (size_t y = 0; y < display->effective_height(); ++y) {
    for (size_t x = 0; x < display->effective_width(); ++x) {
        Color color = display->get_pixel(x, y);
        // ... process or modify ...
        display->set_pixel(x, y, color);
    }
}
```

**Use Cases:**
- Custom image processing
- Algorithmic art generation
- Performance-critical operations

**Warning:** Direct access bypasses orientation transforms. Use carefully!

### Custom Drawing Functions

Build your own drawing primitives on top of `set_pixel()`:

```cpp
// Draw a triangle
void draw_triangle(Display& display, size_t x1, size_t y1,
                  size_t x2, size_t y2, size_t x3, size_t y3, Color color) {
    display.draw(display.line().from(x1, y1).to(x2, y2).color(color).build());
    display.draw(display.line().from(x2, y2).to(x3, y3).color(color).build());
    display.draw(display.line().from(x3, y3).to(x1, y1).color(color).build());
}

// Draw a star
void draw_star(Display& display, size_t cx, size_t cy, size_t r, Color color) {
    // ... calculate star points ...
    // ... use display.line().from().to().color().build() ...
}
```

### Performance Optimization

**Batch Drawing Operations:**
```cpp
// Good: Batch all drawing, then refresh once
display->clear(Color::White);
for (const auto& item : items) {
    display->draw(
        display->text(item.text)
            .at(10, item.y)
            .font(&Font::font12())
            .foreground(Color::Black)
            .background(Color::White)
            .build()
    );
}
display->refresh();  // Single refresh

// Bad: Refresh after each draw (very slow!)
for (const auto& item : items) {
    display->draw(
        display->text(item.text)
            .at(10, item.y)
            .font(&Font::font12())
            .foreground(Color::Black)
            .background(Color::White)
            .build()
    );
    display->refresh();  // ❌ Don't do this! ~2s each!
}
```

**Minimize Refreshes:**
```cpp
// Update display every 5 seconds
auto last_refresh = std::chrono::steady_clock::now();
const auto refresh_interval = std::chrono::seconds(5);

while (running) {
    auto now = std::chrono::steady_clock::now();
    if (now - last_refresh >= refresh_interval) {
        // Fetch new data
        auto data = fetch_data();

        // Update display
        display->clear(Color::White);
        render_data(display, data);
        display->refresh();

        last_refresh = now;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
```

## Best Practices

### 1. Keep Auto-Sleep Enabled

```cpp
// ✅ Good: Auto-sleep enabled (default)
auto display = create_display<EPD27>(device, DisplayMode::BlackWhite,
                                     Orientation::Portrait0, true);

// ❌ Bad: Disabled without good reason
auto display = create_display<EPD27>(device, DisplayMode::BlackWhite,
                                     Orientation::Portrait0, false);
```

### 2. Always Check Initialization

```cpp
// ✅ Good: Check every expected result
auto device_result = device.init();
if (!device_result) {
    std::cerr << "Init failed: " << device_result.error().what() << "\n";
    return 1;
}

// ❌ Bad: Ignore errors
device.init();  // What if this fails?
```

### 3. Batch Drawing Operations

```cpp
// ✅ Good: Draw everything, then refresh once
display->clear(Color::White);
draw_header(display);
draw_content(display);
draw_footer(display);
display->refresh();  // Single refresh

// ❌ Bad: Multiple refreshes (~2s each!)
draw_header(display);
display->refresh();
draw_content(display);
display->refresh();
```

### 4. Use Appropriate Fonts

```cpp
// ✅ Good: Font size matches purpose
display->draw(display->text("Title").at(10, 10).font(&Font::font24())...);      // Large title
display->draw(display->text("Body text").at(10, 40).font(&Font::font12())...);  // Body text
display->draw(display->text("Footer").at(10, 150).font(&Font::font8())...);     // Small footer

// ❌ Bad: Wrong font sizes
display->draw(display->text("Title").at(10, 10).font(&Font::font8())...);  // Too small!
```

### 5. Handle Errors Gracefully

```cpp
// ✅ Good: Fallback on error
auto result = display->draw_bitmap_from_file(10, 10, "icon.png");
if (!result) {
    // Fallback to text
    display->draw(display->text("[IMG]").at(10, 10).font(&Font::font16())...);
}

// ❌ Bad: Crash on error
display->draw_bitmap_from_file(10, 10, "icon.png").value();  // May throw!
```

## Common Patterns

### Pattern: Status Dashboard

```cpp
void render_dashboard(Display& display, const SystemStatus& status) {
    display->clear(Color::White);

    // Title bar
    display->draw(
        display->rectangle()
            .top_left(0, 0)
            .bottom_right(display->effective_width(), 30)
            .color(Color::Black)
            .fill(DrawFill::Full)
            .build()
    );
    display->draw(
        display->text("System Status")
            .at(10, 8)
            .font(&Font::font16())
            .foreground(Color::White)
            .background(Color::Black)
            .build()
    );

    // Status indicators
    size_t y = 40;
    display->draw(display->text("CPU:").at(10, y).font(&Font::font12()).foreground(Color::Black).background(Color::White).build());
    display->draw(display->text("").at(80, y).decimal(status.cpu_usage, 1).font(&Font::font12()).foreground(Color::Black).background(Color::White).build());
    display->draw(display->text("%").at(120, y).font(&Font::font12()).foreground(Color::Black).background(Color::White).build());

    y += 20;
    display->draw(display->text("Temp:").at(10, y).font(&Font::font12()).foreground(Color::Black).background(Color::White).build());
    display->draw(display->text("").at(80, y).decimal(status.temperature, 1).font(&Font::font12()).foreground(Color::Black).background(Color::White).build());
    display->draw(display->text("C").at(120, y).font(&Font::font12()).foreground(Color::Black).background(Color::White).build());

    // Connection indicator
    if (status.connected) {
        display->draw(
            display->circle()
                .center(250, 10)
                .radius(5)
                .color(Color::White)
                .fill(DrawFill::Full)
                .build()
        );
    }

    display->refresh();
}
```

### Pattern: Multi-Screen Rotation

```cpp
enum class Screen { Overview, Details, Settings };

void render_current_screen(Display& display, Screen current, const AppState& state) {
    display->clear(Color::White);

    switch (current) {
        case Screen::Overview:
            render_overview(display, state);
            break;
        case Screen::Details:
            render_details(display, state);
            break;
        case Screen::Settings:
            render_settings(display, state);
            break;
    }

    display->refresh();
}

// Main loop
Screen current_screen = Screen::Overview;
const auto flip_interval = std::chrono::seconds(10);
auto last_flip = std::chrono::steady_clock::now();

while (running) {
    auto now = std::chrono::steady_clock::now();
    if (now - last_flip >= flip_interval) {
        // Rotate to next screen
        current_screen = static_cast<Screen>((static_cast<int>(current_screen) + 1) % 3);
        render_current_screen(display, current_screen, state);
        last_flip = now;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
```

### Pattern: Periodic Updates

See the [crypto dashboard example](../examples/crypto_dashboard/) for a complete real-world implementation.

## Debugging & Troubleshooting

### BMP Export for Debugging

**NEW:** Save your framebuffer to a BMP file for instant visual inspection without waiting for slow display refresh!

```cpp
// Draw your content
display->clear(Color::White);
display->draw(
    display->text("Layout Test")
        .at(10, 10)
        .font(&Font::font16())
        .foreground(Color::Black)
        .background(Color::White)
        .build()
);
display->draw(
    display->rectangle()
        .top_left(5, 5)
        .bottom_right(250, 170)
        .color(Color::Black)
        .fill(DrawFill::Empty)
        .build()
);

// Save to BMP for debugging (instant!)
auto result = display->save_framebuffer_to_bmp("debug_layout.bmp");
if (!result) {
    std::cerr << "BMP export failed: " << result.error().what() << "\n";
}

// Now refresh to display (slow, ~2 seconds)
display->refresh();
```

**Benefits:**
- **Instant feedback**: See your layout immediately without 2-second display refresh
- **Works anywhere**: View BMP on your computer, no display hardware needed
- **Perfect accuracy**: Shows exactly what will appear on display (with orientation)
- **Iterative development**: Quickly test different layouts

**Color mapping:**
- **BlackWhite mode**: Black→RGB(0,0,0), White→RGB(255,255,255)
- **Grayscale4 mode**: Black→RGB(0,0,0), Gray2→RGB(85,85,85), Gray1→RGB(170,170,170), White→RGB(255,255,255)

**Example workflow:**
```cpp
// Rapid layout iteration without hardware
for (int layout = 0; layout < 5; ++layout) {
    display->clear(Color::White);
    render_layout(display, layout);

    // Save each layout variant
    display->save_framebuffer_to_bmp("layout_" + std::to_string(layout) + ".bmp");
}

// Review all BMPs on computer, pick best one
// Then deploy to hardware
display->clear(Color::White);
render_layout(display, best_layout);
display->refresh();  // Only one slow refresh needed!
```

See `examples/bmp_debug_example.cpp` for a complete demonstration.

### Display Not Responding

**Symptoms:** No output on display, or refresh hangs indefinitely.

**Solutions:**
1. Check hardware connections (especially BUSY pin)
2. Verify SPI is enabled: `sudo raspi-config` → Interface Options → SPI
3. Ensure user is in `gpio` and `spi` groups: `groups` (should show both)
4. If not in groups, add with: `sudo usermod -a -G gpio,spi $USER` (then log out/in)
4. Check display power supply (3.3V, sufficient current)

### Garbled Output

**Symptoms:** Random pixels, incorrect rendering.

**Solutions:**
1. Check SPI wiring (MOSI, SCLK must be correct)
2. Verify ground connection
3. Ensure display mode matches drawing colors
4. Check framebuffer isn't corrupted (clear before drawing)

### Slow Performance

**Symptoms:** Long delays, sluggish updates.

**Solutions:**
1. Batch drawing operations (single `refresh()` at end)
2. Reduce refresh frequency (e.g., 5-10 second intervals)
3. Profile code to find bottlenecks
4. Remember: e-paper refresh is inherently slow (~2s for EPD27)

### Memory Issues

**Symptoms:** Out of memory errors, crashes on Raspberry Pi Zero.

**Solutions:**
1. Use BlackWhite mode instead of Grayscale4 (half memory)
2. Don't load large images unnecessarily
3. Clear framebuffer when switching contexts
4. Consider using Raspberry Pi 3/4 (more RAM)

### Colors Not Displaying

**Symptoms:** Gray colors appear as black.

**Solutions:**
1. Verify using `Grayscale4` mode (not `BlackWhite`)
2. Check display supports grayscale (EPD27 does)
3. Ensure using `Color::Gray1` or `Color::Gray2` (not just `Black`/`White`)

---

**For architecture details, see [ARCHITECTURE.md](ARCHITECTURE.md).**
**For driver development, see [DRIVER.md](DRIVER.md).**
**For examples, see [examples/README.md](../examples/README.md).**
