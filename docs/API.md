# API Reference

A comprehensive guide to using the libepaper2 API for e-paper display control.

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
- BCM2835 library installed
- Root privileges for GPIO/SPI access

### Basic Setup

```cpp
#include <epaper/device.hpp>
#include <epaper/display.hpp>
#include <epaper/drivers/epd27.hpp>
#include <epaper/font.hpp>

using namespace epaper;

int main() {
    // 1. Initialize hardware device
    Device device;
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

    // 3. Draw something
    display->clear(Color::White);
    display->draw_string(10, 10, "Hello World!", Font::font16(), 
                        Color::Black, Color::White);

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
    display->draw_rectangle(0, 0, 
        display->effective_width() - 1, 
        display->effective_height() - 1, 
        Color::Black, 
        DrawFill::Empty);

    // Draw title
    display->draw_string(10, 10, "My First E-Paper App", 
        Font::font20(), Color::Black, Color::White);

    // Draw a filled rectangle
    display->draw_rectangle(10, 40, 100, 80, 
        Color::Black, DrawFill::Full);

    // Draw text on black background
    display->draw_string(15, 50, "libepaper2", 
        Font::font12(), Color::White, Color::Black);

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

# Run (requires sudo for GPIO/SPI)
sudo ./build/your_program
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
display->draw_point(10, 10, Color::Black);
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
display->draw_point(10, 10, Color::White);  // White pixel
display->draw_point(20, 20, Color::Black);  // Black pixel

// Gray colors will be converted to black
display->draw_point(30, 30, Color::Gray1);  // → Black
display->draw_point(40, 40, Color::Gray2);  // → Black
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
display->draw_point(10, 10, Color::White);  // Lightest
display->draw_point(20, 20, Color::Gray1);  // Light gray
display->draw_point(30, 30, Color::Gray2);  // Dark gray
display->draw_point(40, 40, Color::Black);  // Darkest
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
display->draw_point(0, 0, Color::Black);  // Appears at actual top-left

// All drawing operations use rotated coordinates
display->draw_rectangle(0, 0, width - 1, height - 1, Color::Black, DrawFill::Empty);
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
display->draw_point(x, y, Color::Black);

// Draw a larger point (2x2, 3x3, 4x4, 5x5 pixels)
display->draw_point(x, y, Color::Black, DotPixel::Pixel2x2);
display->draw_point(x, y, Color::Black, DotPixel::Pixel3x3);
display->draw_point(x, y, Color::Black, DotPixel::Pixel4x4);
display->draw_point(x, y, Color::Black, DotPixel::Pixel5x5);
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
display->draw_line(x1, y1, x2, y2, Color::Black);

// Thicker lines
display->draw_line(x1, y1, x2, y2, Color::Black, DotPixel::Pixel2x2);

// Dotted or dashed lines
display->draw_line(x1, y1, x2, y2, Color::Black, 
                  DotPixel::Pixel1x1, LineStyle::Dotted);

display->draw_line(x1, y1, x2, y2, Color::Black, 
                  DotPixel::Pixel1x1, LineStyle::Dashed);
```

**LineStyle enum:**
- `Solid`: Continuous line (default)
- `Dotted`: Dot every 4 pixels
- `Dashed`: Dash pattern (8 on, 4 off)

**Examples:**
```cpp
// Draw a grid
for (size_t x = 0; x < display->effective_width(); x += 20) {
    display->draw_line(x, 0, x, display->effective_height() - 1, Color::Black);
}
for (size_t y = 0; y < display->effective_height(); y += 20) {
    display->draw_line(0, y, display->effective_width() - 1, y, Color::Black);
}
```

### Rectangles

```cpp
// Draw a rectangle outline
display->draw_rectangle(x, y, width, height, Color::Black, DrawFill::Empty);

// Draw a filled rectangle
display->draw_rectangle(x, y, width, height, Color::Black, DrawFill::Full);

// Thicker borders
display->draw_rectangle(x, y, width, height, Color::Black, 
                       DrawFill::Empty, DotPixel::Pixel2x2);
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
display->draw_rectangle(10, 10, 100, 40, Color::Black, DrawFill::Empty);
display->draw_string(30, 20, "OK", Font::font16(), Color::Black, Color::White);

// Draw a filled header bar
display->draw_rectangle(0, 0, display->effective_width(), 30, 
                       Color::Black, DrawFill::Full);
display->draw_string(10, 8, "Dashboard", Font::font16(), 
                    Color::White, Color::Black);
```

### Circles

```cpp
// Draw a circle outline
display->draw_circle(center_x, center_y, radius, Color::Black, DrawFill::Empty);

// Draw a filled circle
display->draw_circle(center_x, center_y, radius, Color::Black, DrawFill::Full);

// Thicker outline
display->draw_circle(center_x, center_y, radius, Color::Black, 
                    DrawFill::Empty, DotPixel::Pixel2x2);
```

**Examples:**
```cpp
// Draw a bullseye
display->draw_circle(88, 132, 50, Color::Black, DrawFill::Empty);
display->draw_circle(88, 132, 40, Color::Black, DrawFill::Empty);
display->draw_circle(88, 132, 30, Color::Black, DrawFill::Empty);
display->draw_circle(88, 132, 10, Color::Black, DrawFill::Full);

// Draw a status indicator (filled dot)
if (connected) {
    display->draw_circle(250, 10, 5, Color::Black, DrawFill::Full);
}
```

### Text & Fonts

```cpp
// Draw text
display->draw_string(x, y, "Hello World!", Font::font16(), 
                    Color::Black, Color::White);

// Different font sizes
display->draw_string(10, 10, "Tiny", Font::font8(), 
                    Color::Black, Color::White);
display->draw_string(10, 30, "Small", Font::font12(), 
                    Color::Black, Color::White);
display->draw_string(10, 50, "Medium", Font::font16(), 
                    Color::Black, Color::White);
display->draw_string(10, 80, "Large", Font::font20(), 
                    Color::Black, Color::White);
display->draw_string(10, 110, "XLarge", Font::font24(), 
                    Color::Black, Color::White);
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
display->draw_string(10, 10, "Normal", Font::font16(), 
                    Color::Black, Color::White);

// White text on black background (inverse)
display->draw_string(10, 30, "Inverse", Font::font16(), 
                    Color::White, Color::Black);

// Gray text (in grayscale mode)
display->draw_string(10, 50, "Gray", Font::font16(), 
                    Color::Gray2, Color::White);
```

**Multi-line Text:**
```cpp
// Manual line spacing
const size_t x = 10;
size_t y = 10;
const size_t line_height = 20;

display->draw_string(x, y, "Line 1", Font::font16(), Color::Black, Color::White);
y += line_height;
display->draw_string(x, y, "Line 2", Font::font16(), Color::Black, Color::White);
y += line_height;
display->draw_string(x, y, "Line 3", Font::font16(), Color::Black, Color::White);
```

### Numbers

```cpp
// Draw an integer
int value = 42;
display->draw_number(x, y, value, Font::font16(), 
                    Color::Black, Color::White);

// Draw a decimal with precision
double pi = 3.14159;
display->draw_decimal(x, y, pi, 3, Font::font16(),  // 3 decimal places
                     Color::Black, Color::White);
// Displays: "3.141"

// Right-aligned numbers (for tables)
int price = 199;
display->draw_number(200, y, price, Font::font12(), 
                    Color::Black, Color::White);
```

**Formatting Examples:**
```cpp
// Display sensor readings
display->draw_string(10, 10, "Temperature:", Font::font12(), 
                    Color::Black, Color::White);
display->draw_decimal(110, 10, temperature, 1, Font::font12(), 
                     Color::Black, Color::White);
display->draw_string(150, 10, "C", Font::font12(), 
                    Color::Black, Color::White);

// Display prices
display->draw_string(10, 30, "$", Font::font16(), 
                    Color::Black, Color::White);
display->draw_decimal(20, 30, price, 2, Font::font16(), 
                     Color::Black, Color::White);
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

**Image Conversion:**
Images are automatically converted to the display's color mode:
- **BlackWhite mode**: Brightness threshold at 50%
- **Grayscale4 mode**: 4-level quantization

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
    
    display->draw_line(x, 0, x, display->effective_height() - 1, shade);
}

// Draw anti-aliased graphics
// (manually, by using intermediate gray levels at edges)
```

## Display Refresh & Power Management

### Refresh Behavior

Call `refresh()` to update the physical display with the framebuffer contents.

```cpp
// Draw operations modify the framebuffer
display->draw_string(10, 10, "Hello", Font::font16(), Color::Black, Color::White);
display->draw_circle(88, 132, 30, Color::Black, DrawFill::Full);

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
display->draw_string(10, 10, "Frame 1", Font::font16(), Color::Black, Color::White);
display->refresh();  // Display automatically enters sleep mode after refresh

// ... minutes or hours later ...

// Second render - transparent wake!
display->draw_string(10, 10, "Frame 2", Font::font16(), Color::Black, Color::White);
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
| `DeviceInitFailed` | BCM2835 init failed | Check permissions, hardware connection |
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
    display->draw_string(10, 10, "Error:", Font::font16(), 
                        Color::Black, Color::White);
    display->draw_string(10, 30, result.error().what(), Font::font12(), 
                        Color::Black, Color::White);
    display->refresh();  // Try again with error message
}
```

**3. Graceful Degradation:**
```cpp
auto result = display->draw_bitmap_from_file(10, 10, "logo.png");
if (!result) {
    // Fall back to text
    display->draw_string(10, 10, "LOGO", Font::font24(), 
                        Color::Black, Color::White);
}
```

## Advanced Topics

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
    display.draw_line(x1, y1, x2, y2, color);
    display.draw_line(x2, y2, x3, y3, color);
    display.draw_line(x3, y3, x1, y1, color);
}

// Draw a star
void draw_star(Display& display, size_t cx, size_t cy, size_t r, Color color) {
    // ... calculate star points ...
    // ... use draw_line() ...
}
```

### Performance Optimization

**Batch Drawing Operations:**
```cpp
// Good: Batch all drawing, then refresh once
display->clear(Color::White);
for (const auto& item : items) {
    display->draw_string(10, item.y, item.text, Font::font12(), 
                        Color::Black, Color::White);
}
display->refresh();  // Single refresh

// Bad: Refresh after each draw (very slow!)
for (const auto& item : items) {
    display->draw_string(10, item.y, item.text, Font::font12(), 
                        Color::Black, Color::White);
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
display->draw_string(10, 10, "Title", Font::font24(), ...);      // Large title
display->draw_string(10, 40, "Body text", Font::font12(), ...);  // Body text
display->draw_string(10, 150, "Footer", Font::font8(), ...);     // Small footer

// ❌ Bad: Wrong font sizes
display->draw_string(10, 10, "Title", Font::font8(), ...);  // Too small!
```

### 5. Handle Errors Gracefully

```cpp
// ✅ Good: Fallback on error
auto result = display->draw_bitmap_from_file(10, 10, "icon.png");
if (!result) {
    // Fallback to text
    display->draw_string(10, 10, "[IMG]", Font::font16(), ...);
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
    display->draw_rectangle(0, 0, display->effective_width(), 30, 
                           Color::Black, DrawFill::Full);
    display->draw_string(10, 8, "System Status", Font::font16(), 
                        Color::White, Color::Black);
    
    // Status indicators
    size_t y = 40;
    display->draw_string(10, y, "CPU:", Font::font12(), Color::Black, Color::White);
    display->draw_decimal(80, y, status.cpu_usage, 1, Font::font12(), 
                         Color::Black, Color::White);
    display->draw_string(120, y, "%", Font::font12(), Color::Black, Color::White);
    
    y += 20;
    display->draw_string(10, y, "Temp:", Font::font12(), Color::Black, Color::White);
    display->draw_decimal(80, y, status.temperature, 1, Font::font12(), 
                         Color::Black, Color::White);
    display->draw_string(120, y, "C", Font::font12(), Color::Black, Color::White);
    
    // Connection indicator
    if (status.connected) {
        display->draw_circle(250, 10, 5, Color::White, DrawFill::Full);
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

## Troubleshooting

### Display Not Responding

**Symptoms:** No output on display, or refresh hangs indefinitely.

**Solutions:**
1. Check hardware connections (especially BUSY pin)
2. Verify SPI is enabled: `sudo raspi-config` → Interface Options → SPI
3. Ensure running with `sudo` (GPIO access requires root)
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
