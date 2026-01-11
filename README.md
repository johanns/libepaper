# Modern C++ E-Paper Display Library

A modern C++23 library for controlling Waveshare e-paper displays on Raspberry Pi, featuring transparent sleep/wake management and a fluent builder API.

[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://en.cppreference.com/w/cpp/23)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

> **See [docs](https://docs.jsg.io/libepaper) for complete documentation.**

## ✨ Quick Example

```cpp
#include <epaper/device.hpp>
#include <epaper/display.hpp>
#include <epaper/drivers/epd27.hpp>
#include <epaper/font.hpp>

using namespace epaper;

int main() {
    Device device;
    device.init().value();

    auto display = create_display<EPD27>(device, DisplayMode::BlackWhite,
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

- **Modern C++23**: `std::expected`, `std::span`, concepts, and ranges
- **Transparent Sleep/Wake**: Auto-sleep prevents burn-in, auto-wake on refresh—no manual management
- **Clean Architecture**: Modular design with RAII and composition over inheritance
- **Full Drawing API**: Points, lines, rectangles, circles, text, bitmaps with PNG/JPEG support
- **Automatic Color Conversion**: RGB images automatically converted to grayscale with intelligent quantization
- **Multiple Display Modes**: Black/white (1-bit) and 4-level grayscale (2-bit)
- **Display Orientation**: Rotate 0°, 90°, 180°, 270° with automatic coordinate transformation
- **Extensible Drivers**: Abstract driver interface for easy support of new displays
- **Type Safety**: Strong typing with `std::expected` error handling
- **Zero-Cost Abstractions**: Modern C++ with no runtime overhead

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
./bin/build                      # Release (default)
./bin/build --type Debug         # Debug
./bin/build examples --type Release
```

Using raw CMake:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

## 📚 Documentation

| Document | Description |
|----------|-------------|
| **[API Reference](docs/API.md)** | Complete API guide with usage patterns |
| **[Architecture Guide](docs/ARCHITECTURE.md)** | Deep dive into design and implementation |
| **[Driver Development](docs/DRIVER.md)** | Guide to writing new display drivers |
| **[Examples](examples/README.md)** | Tutorials and working examples |

## 🤝 Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for our commit format and PR process.

## 🏗️ Supported Hardware

**Currently Implemented:**
- Waveshare 2.7" e-Paper Display (176×264 pixels)

**Easy to Extend:**
The abstract driver interface makes adding support for additional displays straightforward. See [docs/DRIVER.md](docs/DRIVER.md) for a guide to writing new drivers.

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
