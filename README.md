# Modern C++ E-Paper Display Library

A modern C++23 library for controlling Waveshare e-paper displays on Raspberry Pi with transparent sleep/wake management and clean architecture.

[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://en.cppreference.com/w/cpp/23)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

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
    display->draw_string(20, 70, "Hello, libepaper!",
                        Font::font24(), Color::Black, Color::White);
    display->refresh();

    return 0;
}
```

👉 **See [examples/](examples/README.md) for complete applications including a real-time crypto dashboard!**

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

## 📚 Documentation

**[Full API Reference](https://docs.jsg.io/libepaper/index.html)**

| Document | Description |
|----------|-------------|
| **[Examples & Tutorials](examples/README.md)** | Get started quickly with working examples |
| **[API Reference](docs/API.md)** | Complete API guide with usage patterns |
| **[Architecture Guide](docs/ARCHITECTURE.md)** | Deep dive into design, decisions, and diagrams |
| **[Driver Development](docs/DRIVER.md)** | Write drivers for new displays |

## 🚀 Quick Start

### 1. Installation

Run the automated setup script:

```bash
./bin/setup
```

Or manually install dependencies:

```bash
sudo apt install cmake g++-14 libgpiod-dev libcurl4-openssl-dev nlohmann-json3-dev pkg-config
```

### 2. Enable SPI and Configure Permissions

```bash
# Enable SPI interface
sudo raspi-config
# Interface Options → SPI → Enable

# Add user to gpio and spi groups (for GPIO/SPI access without sudo)
sudo usermod -a -G gpio,spi $USER
# Log out and back in for group changes to take effect
```

### 3. Hardware Connection

| E-Paper Pin | Raspberry Pi GPIO |
|-------------|-------------------|
| RST | GPIO 17 |
| DC | GPIO 25 |
| CS | GPIO 8 (CE0) |
| BUSY | GPIO 24 |
| MOSI | GPIO 10 (MOSI) |
| SCLK | GPIO 11 (SCLK) |
| GND | GND |
| VCC | 3.3V |

### 4. Build

```bash
./bin/build
```

Or manually:

```bash
cmake -S . -B build -DCMAKE_CXX_COMPILER=g++-14
cmake --build build -j$(nproc)
```

### 5. Run Example

```bash
# Run crypto dashboard (live prices with charts)
./build/examples/crypto_dashboard/crypto_dashboard

# Run tests
./build/tests/test_auto_sleep
```

**Note:** No sudo required! The library uses user-space GPIO access via libgpiod. Make sure you're in the `gpio` and `spi` groups (see step 2).

## 🧪 Testing

```bash
# Run all tests
cd build/tests
./run_all_tests.sh

# Run specific test
./test_auto_sleep
```

See [tests/README.md](tests/README.md) for details.

## 🤝 Contributing

We welcome contributions! This project uses **Conventional Commits** for clear history.

### Commit Format

```
<type>(<scope>): <subject>

<body>

<footer>
```

**Types:**
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code formatting
- `refactor`: Code refactoring
- `perf`: Performance improvements
- `test`: Test additions/updates
- `chore`: Maintenance tasks

**Scopes:** `device`, `driver`, `display`, `docs`, `examples`, `tests`

### Examples

```
feat(driver): add EPD42 display driver support

Implements the Driver interface for Waveshare 4.2" displays.
Supports both black/white and 4-level grayscale modes.

fix(display): correct auto-wake behavior in refresh()

The previous implementation didn't check is_asleep_ before refresh.
Now transparently wakes display if needed.

docs(readme): restructure documentation for clarity
```

### Pull Request Process

1. Fork and create a feature branch
2. Follow project coding standards (run `./bin/format`)
3. Add/update tests as needed
4. Use conventional commit messages
5. Update documentation
6. Submit PR with clear description

## 🏗️ Supported Hardware

**Currently Implemented:**
- Waveshare 2.7" e-Paper Display (176×264 pixels)

**Easy to Extend:**
The abstract driver interface makes it straightforward to add support for other displays. See [docs/DRIVER.md](docs/DRIVER.md) for a guide to writing new drivers.

## 🔧 Development

### Requirements

- C++23 capable compiler (GCC 14+ or Clang 18+)
- CMake 3.25+
- libgpiod library (user-space GPIO access)
- Raspberry Pi with SPI enabled
- User in `gpio` and `spi` groups (no sudo required)

### Project Structure

```
libepaper/
├── include/epaper/        # Public API headers
│   ├── device.hpp         # Hardware abstraction
│   ├── display.hpp        # High-level display API
│   ├── drivers/           # Driver interface & implementations
│   └── font.hpp           # Font rendering
├── src/                   # Implementation files
├── examples/              # Example applications
│   ├── bmp_debug_example/ # BMP export for debugging
│   └── crypto_dashboard/  # Real-time crypto price dashboard
├── tests/                 # Test suite
├── docs/                  # Comprehensive documentation
└── bin/                   # Build and setup scripts
```

## 🙏 Acknowledgments

- **Waveshare** for the e-Paper hardware and original library
- **libgpiod developers** for modern Linux GPIO access
- **stb_image library authors** for image loading

## 📄 License

MIT License - see [LICENSE](LICENSE) file for details.

Copyright (c) 2025 Johanns Gregorian

---

**Getting Started?** → [Examples](examples/README.md)
**API Questions?** → [API Reference](docs/API.md)
**Architecture Deep Dive?** → [Architecture Guide](docs/ARCHITECTURE.md)
**Writing a Driver?** → [Driver Development](docs/DRIVER.md)
