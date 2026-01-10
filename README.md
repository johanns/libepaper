# Modern C++ E-Paper Display Library

A modern C++23 library for controlling Waveshare e-paper displays on Raspberry Pi with transparent sleep/wake management and a fluent builder API.

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
- **Extensible Drivers**: Abstract driver interface for easy addition of new displays
- **Type Safety**: Strong typing with `std::expected` error handling
- **Zero-Cost Abstractions**: Modern C++ with no runtime overhead

## 🚀 Quick Start

### 1. Installation

Run the automated setup script to install dependencies (cmake, g++-14, libgpiod, etc.):

```bash
./bin/setup
```

### 2. Build & Run

```bash
./bin/build
```

### 3. Run Example

```bash
# Run crypto dashboard (live prices with charts)
./build/examples/crypto_dashboard/crypto_dashboard
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
The abstract driver interface makes it straightforward to add support for other displays. See [docs/DRIVER.md](docs/DRIVER.md) for a guide to writing new drivers.

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
