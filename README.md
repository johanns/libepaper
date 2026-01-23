# C++23 E-Paper Display Library

A C++23 library for controlling Waveshare e-paper displays on Raspberry Pi, featuring transparent sleep/wake management and a fluent builder API.

[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://en.cppreference.com/w/cpp/23)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

> **See [docs](https://docs.jsg.io/libepaper) for complete documentation.**

## ✨ Quick Example

```cpp
#include <epaper/core/device.hpp>
#include <epaper/core/display.hpp>
#include <epaper/drivers/epd27.hpp>  // Use your driver
#include <epaper/graphics/font.hpp>

using namespace epaper;

int main() {
    Device device;
    device.init().value();

    // Replace EPD27 with your driver (EPD27, MockDriver, etc.)
    auto display = create_display<EPD27, MonoFramebuffer>(device, DisplayMode::BlackWhite,
                                         Orientation::Landscape90).value();

    display->clear(Color::White);
    display->draw(
        display->text("Hello, libepaper!")
            .at(20, 70)
            .font(&Font::font24())
            .foreground(Color::Black)
            .background(Color::White)
            .build()
    );
    display->refresh();

    return 0;
}
```

> 👉 **See [examples/](examples/README.md) for complete applications.**

## 🎯 Features

- **Multiple Display Modes**: Black/white, 4-level grayscale, and color displays (BWR, BWY, Spectra 6-color)
- **Complete Drawing API**: Points, lines, rectangles, circles, text rendering with multiple font sizes
- **Image Support**: Load and display PNG/JPEG images with automatic color conversion and dithering
- **Flexible Orientation**: Rotate display 0°, 90°, 180°, 270° with automatic coordinate transformation
- **Transparent Power Management**: Auto-sleep prevents burn-in, auto-wake on refresh—zero manual intervention
- **Hardware Abstraction**: Compile-time driver selection with MockDriver for testing without physical hardware
- **Multiple Framebuffer Types**: Single-plane (mono/grayscale) and multi-plane (color) with automatic color mapping
- **Builder Pattern API**: Fluent, chainable drawing commands with sensible defaults
- **Comprehensive Error Handling**: Type-safe error propagation with `std::expected` (no exceptions)
- **C++23**: Built with concepts, `std::span`, ranges, and zero-cost abstractions

## Driver Selection

All drivers are compiled into the library. To use a driver:

1. **Include the driver header** for your display:
   ```cpp
   #include <epaper/drivers/epd27.hpp>       // Waveshare 2.7" B/W
   #include <epaper/drivers/mock_driver.hpp> // Software-only testing
   ```

2. **Use the driver class** in `create_display<>()`:
   ```cpp
    auto display = create_display<EPD27, MonoFramebuffer>(device, DisplayMode::BlackWhite,
                                        Orientation::Landscape90).value();
   ```

### Available Drivers

| Driver | Hardware | Dimensions | Color Support |
|--------|----------|------------|---------------|
| **EPD27** | Waveshare 2.7" | 176×264 | B/W, Grayscale4 |
| **MockDriver** | Software only | Configurable | B/W (outputs PNG) |

> 📖 See [Doxygen documentation](doxygen/html/index.html) for detailed driver API specifications

## 🚀 Quick Start

### Use as a dependency in your CMake project

Link the `libepaper` target via CMake FetchContent:

```cmake
include(FetchContent)

FetchContent_Declare(
     epaper
     GIT_REPOSITORY https://github.com/johanns/libepaper
     GIT_TAG        main
)
FetchContent_MakeAvailable(epaper)

add_executable(my_app src/main.cpp)
target_link_libraries(my_app PRIVATE libepaper)
```

Then build your project as usual (Debug or Release mode).

### Run examples from the repository

1. Install dependencies and set up the system:

    ```bash
    ./bin/setup
    ```

2. Build the library (Release by default, use `--type Debug` for debug builds):

    ```bash
    ./bin/build
    # ./bin/build --type Debug
    ```

3. Run an example:

    ```bash
    ./build/examples/crypto_dashboard/crypto_dashboard
    ```

## 🧩 Alternative Integration Options

For other integration methods:

### Option A — Vendored submodule with add_subdirectory

```bash
git submodule add https://github.com/johanns/libepaper external/libepaper
git submodule update --init --recursive
```

```cmake
add_subdirectory(external/libepaper)
add_executable(my_app src/main.cpp)
target_link_libraries(my_app PRIVATE libepaper)
```

### Option B — System-wide installation

```bash
cd libepaper
./bin/setup
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
sudo cmake --install build
```

```cmake
find_path(EPAPER_INCLUDE_DIR epaper/display.hpp PATH_SUFFIXES epaper)
find_library(EPAPER_LIBRARY NAMES epaper)

add_executable(my_app src/main.cpp)
target_include_directories(my_app PRIVATE ${EPAPER_INCLUDE_DIR})
target_link_libraries(my_app PRIVATE ${EPAPER_LIBRARY} gpiod m)
```

## ⚙️ Build Configuration

Build with Release (default) or Debug optimization:

Using our script:

```bash
./bin/build                      # Release (default, EPD27 driver)
./bin/build --type Debug         # Debug build
./bin/build --driver Mock tests  # Build tests with MockDriver (no hardware)
./bin/build examples --type Release
```

Using raw CMake:

```bash
# Standard build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j

# Build with MockDriver for testing without hardware
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DLIBEPAPER_DRIVER=Mock
cmake --build build -j
./build/tests/test_drawing_primitives  # Outputs PNG to mock_output/
```

## 📚 Documentation

The library provides multiple documentation resources:

### 📖 API Documentation

- **[Doxygen Reference](https://docs.jsg.io/libepaper/)** - Comprehensive API documentation with:
  - Detailed class and function documentation
  - Usage examples for major APIs
  - Type reference (enums, structs, concepts)
  - Implementation notes and algorithm details

  Generate locally with: `doxygen` (outputs to `doxygen/html/`)

### 📘 Guides

| Document | Description |
|----------|-------------|
| **[Examples](examples/README.md)** | Tutorials and working examples |
| **[Contributing Guide](CONTRIBUTING.md)** | Commit format and PR process |

### 💡 Quick Reference

**Core Concepts:**
- **Display**: Unified interface coordinating driver and framebuffer ([epaper::Display](include/epaper/core/display.hpp))
- **Driver**: Hardware abstraction (SPI/GPIO communication) ([Driver concept](include/epaper/drivers/driver_concepts.hpp))
- **Framebuffer**: Pixel buffer with color encoding ([MonoFramebuffer](include/epaper/core/framebuffer.hpp))
- **Builder API**: Fluent drawing commands ([Builders](include/epaper/draw/builders.hpp))

**Essential Headers:**
```cpp
#include <epaper/core/device.hpp>      // Device initialization
#include <epaper/core/display.hpp>     // Display interface
#include <epaper/drivers/epd27.hpp>    // Hardware driver (replace with your driver)
#include <epaper/graphics/font.hpp>    // Bitmap fonts (font8-font24)
#include <epaper/io/image_io.hpp>      // PNG/JPEG loading
```

**Common Patterns:**
```cpp
// 1. Initialize device
Device device;
device.init().value();

// 2. Create display with driver
auto display = create_display<EPD27>(device, DisplayMode::BlackWhite).value();

// 3. Drawing operations
display->clear(Color::White);
display->line({0, 0}, {100, 100}).color(Color::Black).draw();
display->rectangle({10, 10}, {50, 50}).fill(DrawFill::Full).draw();

// 4. Refresh to hardware
display->refresh();

// 5. Power management (optional - auto-sleep enabled by default)
display->sleep();  // Manual sleep
display->wake();   // Manual wake
```

## 🤝 Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for our commit format and PR process.

## 🏗️ Adding New Display Support

The abstract driver interface makes adding new displays straightforward:

1. Implement the `Driver` concept (see [driver_concepts.hpp](include/epaper/drivers/driver_concepts.hpp))
2. Create driver-specific pin configuration struct
3. Implement display protocol (init, refresh, sleep/wake)
4. Add comprehensive Doxygen documentation

**Reference Implementation:** See [EPD27 driver](include/epaper/drivers/epd27.hpp) for a complete example.

**Driver Interface Requirements:**
- Satisfy `Driver` concept constraints (width(), height(), init(), display(), etc.)
- Define `driver_traits` specialization for capability advertisement
- Use `std::expected<T, Error>` for all fallible operations
- Follow RAII patterns for resource management

**Testing:** Use [MockDriver](include/epaper/drivers/mock_driver.hpp) for development without hardware.

## 🔧 Development

### Requirements

- Raspberry Pi OS, Debian 12 (Bookworm), or Ubuntu 24.04 (Jammy Jellyfish)
- C++23 capable compiler (GCC 14+ or Clang 18+)
- CMake 3.25+
- libgpiod library (user-space GPIO access)
- Raspberry Pi with SPI enabled
- User in `gpio` and `spi` groups (no sudo required)

## 📄 License

MIT License - see [LICENSE](LICENSE) file for details.
Copyright (c) 2021 - 2026 Johanns Gregorian (https://github.com/johanns)
