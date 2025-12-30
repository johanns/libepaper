# Examples & Tutorials

A collection of example applications demonstrating libepaper2 features and best practices.

## Table of Contents

- [Overview](#overview)
- [Quick Start Example](#quick-start-example)
- [Available Examples](#available-examples)
- [Learning Path](#learning-path)
- [Building & Running](#building--running)
- [Example Structure](#example-structure)
- [Common Patterns](#common-patterns)
- [Tips & Tricks](#tips--tricks)
- [Contributing Examples](#contributing-examples)

## Overview

This directory contains complete, working examples that demonstrate various features of the libepaper2 library. Each example is self-contained and can serve as a starting point for your own projects.

**All examples require:**
- Raspberry Pi with SPI enabled
- E-paper display connected (2.7" EPD27)
- Root privileges (`sudo`) for GPIO/SPI access

## Quick Start Example

The simplest possible e-paper program:

```cpp
#include <epaper/device.hpp>
#include <epaper/display.hpp>
#include <epaper/drivers/epd27.hpp>
#include <epaper/font.hpp>
#include <iostream>

using namespace epaper;

int main() {
    // 1. Initialize hardware
    Device device;
    if (auto result = device.init(); !result) {
        std::cerr << "Failed to init device\n";
        return 1;
    }

    // 2. Create display
    auto display = create_display<EPD27>(
        device,
        DisplayMode::BlackWhite,
        Orientation::Landscape90,
        true  // auto-sleep enabled
    );

    if (!display) {
        std::cerr << "Failed to create display\n";
        return 1;
    }

    // 3. Draw something
    display->clear(Color::White);
    
    display->draw_rectangle(0, 0, 
        display->effective_width() - 1, 
        display->effective_height() - 1, 
        Color::Black, DrawFill::Empty);
    
    display->draw_string(20, 70, "Hello, libepaper2!", 
        Font::font24(), Color::Black, Color::White);

    // 4. Refresh to show
    if (auto result = display->refresh(); !result) {
        std::cerr << "Refresh failed\n";
        return 1;
    }

    std::cout << "Success! Check your display.\n";
    return 0;
}
```

**Save as `hello.cpp`, compile and run:**
```bash
# Compile
g++-14 -std=c++23 hello.cpp -o hello -lepaper -lbcm2835

# Run (requires sudo)
sudo ./hello
```

## Available Examples

### 1. Crypto Dashboard (`crypto_dashboard/`)

A real-time cryptocurrency price and chart dashboard with multi-screen rotation.

**Features Demonstrated:**
- HTTP API integration (CoinGecko)
- Multi-screen dashboard with rotation
- Line chart drawing
- Transparent sleep/wake management
- Error handling and recovery
- Periodic data fetching
- Signal handling (graceful shutdown)

**What it shows:**
- **Screen 1**: BTC and ETH prices with 30-day charts side-by-side
- **Screen 2**: BTC price and 30-day + 6-month charts stacked
- **Screen 3**: ETH price and 30-day + 6-month charts stacked
- Rotates every 15 seconds

**Run it:**
```bash
sudo ./build/examples/crypto_dashboard/crypto_dashboard
```

**Learn more:** [crypto_dashboard/README.md](crypto_dashboard/README.md)

**Code highlights:**
```cpp
// Main features showcased:
- CryptoAPI: HTTP client wrapper using cURL
- DashboardRenderer: Multi-screen layout management
- Transparent sleep/wake: Works across screen rotations
- Signal handling: Clean Ctrl-C shutdown
- Error display: Fallback screens when API fails
```

### 2. Basic Drawing Demo (`demo.cpp`)

**Status:** _TODO - To be created_

A comprehensive demonstration of all drawing primitives.

**Will demonstrate:**
- Points, lines, rectangles, circles
- All font sizes
- Text rendering
- Filled vs outlined shapes
- Different orientations
- Color modes

### 3. Image Gallery (`image_gallery/`)

**Status:** _TODO - To be created_

A slideshow application displaying images from a directory.

**Will demonstrate:**
- Bitmap loading from files
- Image scaling
- Automatic format detection (PNG, JPEG, BMP)
- Slideshow timing
- Directory scanning

### 4. Weather Station (`weather_station/`)

**Status:** _TODO - To be created_

A weather display fetching data from an API.

**Will demonstrate:**
- API integration
- Icon rendering
- Multi-line text layout
- Temperature graphs
- Forecast display

### 5. System Monitor (`system_monitor/`)

**Status:** _TODO - To be created_

A Raspberry Pi system monitor showing CPU, memory, and temperature.

**Will demonstrate:**
- System information gathering
- Bar graphs
- Real-time updates
- Threshold indicators
- Status icons

## Learning Path

### Beginner

Start here if you're new to libepaper2:

1. **Read the Quick Start** (above)
2. **Try the Crypto Dashboard**: `sudo ./build/examples/crypto_dashboard/crypto_dashboard`
3. **Read the API documentation**: [docs/API.md](../docs/API.md)
4. **Experiment**: Modify the crypto dashboard to display different data

### Intermediate

Ready for more control:

1. **Study crypto dashboard source**: See how multi-screen apps work
2. **Write your own example**: Adapt patterns to your use case
3. **Explore display modes**: Try both BlackWhite and Grayscale4
4. **Test orientations**: Use all four orientations

### Advanced

Deep dive into the library:

1. **Read the Architecture Guide**: [docs/ARCHITECTURE.md](../docs/ARCHITECTURE.md)
2. **Implement custom drawing functions**: Build on `set_pixel()`
3. **Write a driver**: Support a new display, see [docs/DRIVER.md](../docs/DRIVER.md)
4. **Contribute**: Submit your examples and drivers!

## Building & Running

### Build All Examples

```bash
# From project root
cmake -S . -B build -DCMAKE_CXX_COMPILER=g++-14
cmake --build build -j$(nproc)

# Examples are in:
ls build/examples/
```

### Build Specific Example

```bash
# Build just crypto dashboard
cmake --build build --target crypto_dashboard

# Run it
sudo ./build/examples/crypto_dashboard/crypto_dashboard
```

### Build Your Own Example

```cmake
# Add to examples/CMakeLists.txt
add_executable(my_example my_example.cpp)
target_link_libraries(my_example PRIVATE epaper)
```

## Example Structure

When creating a new example, follow this structure:

```
examples/
└── your_example/
    ├── CMakeLists.txt          # Build configuration
    ├── README.md               # Example documentation
    ├── include/                # Public headers (if needed)
    │   └── your_header.hpp
    └── src/
        ├── main.cpp            # Entry point
        └── implementation.cpp  # Additional source files
```

### Minimal CMakeLists.txt

```cmake
# examples/your_example/CMakeLists.txt
add_executable(your_example
  src/main.cpp
  src/implementation.cpp
)

target_include_directories(your_example PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(your_example PRIVATE
  epaper
  # Add other dependencies here (e.g., curl, json)
)
```

### Minimal README.md

```markdown
# Your Example Name

Brief description of what this example does.

## Features
- Feature 1
- Feature 2

## Running
\`\`\`bash
sudo ./build/examples/your_example/your_example
\`\`\`

## Code Structure
- `main.cpp`: Entry point
- `implementation.cpp`: Core logic

## Notes
- Special considerations
- Known issues
```

## Common Patterns

### Pattern: Periodic Display Updates

Update display at regular intervals:

```cpp
#include <chrono>
#include <thread>

const auto update_interval = std::chrono::seconds(10);
auto last_update = std::chrono::steady_clock::now();

while (running) {
    auto now = std::chrono::steady_clock::now();
    
    if (now - last_update >= update_interval) {
        // Fetch fresh data
        auto data = get_data();
        
        // Update display
        display->clear(Color::White);
        render_data(display, data);
        display->refresh();
        
        last_update = now;
    }
    
    // Sleep briefly to avoid busy-waiting
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
```

### Pattern: Multi-Screen Rotation

Rotate between different screens:

```cpp
enum class Screen { Overview, Details, Settings };

Screen current = Screen::Overview;
const auto flip_interval = std::chrono::seconds(15);
auto last_flip = std::chrono::steady_clock::now();

while (running) {
    auto now = std::chrono::steady_clock::now();
    
    if (now - last_flip >= flip_interval) {
        // Advance to next screen
        int screen_num = static_cast<int>(current);
        current = static_cast<Screen>((screen_num + 1) % 3);
        
        // Render new screen
        display->clear(Color::White);
        switch (current) {
            case Screen::Overview:  render_overview(display); break;
            case Screen::Details:   render_details(display); break;
            case Screen::Settings:  render_settings(display); break;
        }
        display->refresh();
        
        last_flip = now;
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
```

### Pattern: Error Screen Fallback

Show errors on display when operations fail:

```cpp
void render_error(Display& display, const std::string& message) {
    display->clear(Color::White);
    
    // Error border
    display->draw_rectangle(5, 5, 
        display->effective_width() - 6, 
        display->effective_height() - 6, 
        Color::Black, DrawFill::Empty);
    
    // Error title
    display->draw_string(20, 20, "Error", 
        Font::font20(), Color::Black, Color::White);
    
    // Error message (word-wrap manually)
    size_t y = 50;
    display->draw_string(20, y, message.substr(0, 30), 
        Font::font12(), Color::Black, Color::White);
    
    display->refresh();
}

// Usage:
auto result = fetch_data();
if (!result) {
    render_error(display, result.error().what());
    return;
}
```

### Pattern: Signal Handling for Graceful Shutdown

Handle Ctrl-C cleanly:

```cpp
#include <atomic>
#include <csignal>

std::atomic<bool> g_running{true};

void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        g_running = false;
        std::cout << "\nShutdown requested...\n";
    }
}

int main() {
    // Install signal handlers
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    
    // Main loop
    while (g_running) {
        // ... do work ...
        
        // Check g_running frequently for fast exit
        if (!g_running) break;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "Clean shutdown complete.\n";
    return 0;
}
```

### Pattern: Loading Screen

Show progress while loading:

```cpp
void show_loading(Display& display, const std::string& message) {
    display->clear(Color::White);
    
    // Center text
    size_t x = (display->effective_width() - message.length() * 7) / 2;
    size_t y = display->effective_height() / 2;
    
    display->draw_string(x, y, message, 
        Font::font12(), Color::Black, Color::White);
    
    display->refresh();
}

// Usage:
show_loading(display, "Fetching data...");
auto data = fetch_data();  // Long operation

show_loading(display, "Rendering...");
render(display, data);
```

## Tips & Tricks

### Layout Design for E-Paper

**Contrast is Key:**
- Use solid black text on white background
- Avoid light grays for text (hard to read)
- Leave margins (at least 5 pixels)

**Font Selection:**
- `font24()`: Titles and headings
- `font16()`: Body text, important data
- `font12()`: Secondary info, labels
- `font8()`: Tiny labels, fine print

**Screen Burn-In Prevention:**
- Keep auto-sleep enabled (default)
- Vary content (don't show same image for days)
- Update periodically (every 5-10 seconds minimum)

**Visual Hierarchy:**
```cpp
// Good: Clear hierarchy
display->draw_string(10, 5, "TITLE", Font::font24(), ...);       // Large
display->draw_string(10, 35, "Subtitle", Font::font16(), ...);   // Medium
display->draw_string(10, 60, "Detail text", Font::font12(), ...); // Small
```

### Debugging Examples

**Console Logging:**
```cpp
std::cout << "[Init] Starting...\n";
std::cout << "[Fetch] Requesting data...\n";
std::cout << "[Render] Drawing screen...\n";
std::cout << "[Refresh] Updating display...\n";
std::cout.flush();  // Ensure output appears immediately
```

**Error Display:**
```cpp
// Show errors on screen for debugging
try {
    risky_operation();
} catch (const std::exception& e) {
    display->clear(Color::White);
    display->draw_string(10, 10, "Exception:", Font::font12(), ...);
    display->draw_string(10, 30, e.what(), Font::font12(), ...);
    display->refresh();
}
```

**Timing Measurements:**
```cpp
#include <chrono>

auto start = std::chrono::steady_clock::now();
display->refresh();
auto end = std::chrono::steady_clock::now();
auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
std::cout << "Refresh took " << ms.count() << "ms\n";
```

### Performance Optimization

**Minimize Refreshes:**
```cpp
// ✅ Good: One refresh for everything
display->clear(Color::White);
draw_header(display);
draw_body(display);
draw_footer(display);
display->refresh();  // Single 2-second refresh

// ❌ Bad: Multiple refreshes (6+ seconds!)
draw_header(display);
display->refresh();  // 2s
draw_body(display);
display->refresh();  // 2s
draw_footer(display);
display->refresh();  // 2s
```

**Batch Drawing Operations:**
```cpp
// Draw all elements before refresh
for (const auto& item : items) {
    display->draw_string(item.x, item.y, item.text, ...);
}
display->refresh();  // Once at the end
```

**Cache Expensive Operations:**
```cpp
// Fetch data once, use multiple times
auto data = fetch_data();  // Expensive

// Use cached data for multiple screens
render_screen1(display, data);
// ... later ...
render_screen2(display, data);  // No refetch
```

## Contributing Examples

We welcome new examples! Good examples to contribute:

### Example Ideas

- **Weather Dashboard**: Fetch from OpenWeatherMap
- **Calendar Display**: Show upcoming events
- **News Reader**: RSS feed display
- **Stock Ticker**: Financial data
- **IoT Sensor Dashboard**: Display sensor data
- **QR Code Display**: Generate and show QR codes
- **Photo Frame**: Slideshow from folder
- **Task List**: Todo list display
- **Clock**: Time, date, sunrise/sunset

### Contribution Guidelines

**What makes a good example:**
1. **Self-contained**: All code in one directory
2. **Well-documented**: README explains what it does
3. **Commented code**: Explain non-obvious sections
4. **Error handling**: Graceful failure, not crashes
5. **Clean shutdown**: Handle Ctrl-C properly
6. **Demonstrates patterns**: Shows best practices

**Submission process:**
1. Create your example in `examples/your_example/`
2. Add `CMakeLists.txt` and `README.md`
3. Test thoroughly (multiple runs, error cases)
4. Update this file (`examples/README.md`) with your example
5. Submit pull request with conventional commit message

**Pull Request Template:**
```markdown
feat(examples): add weather dashboard example

Demonstrates:
- OpenWeatherMap API integration
- Icon rendering
- Multi-day forecast
- Temperature graphing

Tested on Raspberry Pi 4 with EPD27.
```

### Code Quality Standards

Examples should follow project standards:
- C++23 features where appropriate
- `std::expected` for error handling
- RAII for resources
- clang-format style
- No compiler warnings

---

**For API reference, see [docs/API.md](../docs/API.md).**  
**For architecture details, see [docs/ARCHITECTURE.md](../docs/ARCHITECTURE.md).**  
**For driver development, see [docs/DRIVER.md](../docs/DRIVER.md).**
