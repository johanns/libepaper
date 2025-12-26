# Modern C++ E-Paper Display Library

A modern C++23 library for controlling Waveshare e-paper displays on Raspberry Pi.

## Features

- **Modern C++23**: Uses latest C++ features including `std::expected`, `std::span`, concepts, and ranges
- **Clean Architecture**: Modular design with clear separation of concerns
- **Extensible Design**: Abstract driver interface for easy addition of new displays
- **RAII Throughout**: Automatic resource management with no manual memory handling
- **Composition Over Inheritance**: Simple, maintainable design
- **Full Drawing API**: Points, lines, rectangles, circles, text, numbers, and bitmaps
- **Multiple Display Modes**: Both black/white and 4-level grayscale support
- **Type Safety**: Strong typing with enums and type-safe wrappers
- **Zero-Cost Abstractions**: Modern C++ with no runtime overhead

## Architecture

```
┌─────────────┐
│  demo.cpp   │ Example Application
└──────┬──────┘
       │
   ┌───▼────┐
   │  Draw  │ High-level Drawing API
   └───┬────┘
       │
   ┌───▼────────┐
   │   Screen   │ Framebuffer Management
   └───┬────────┘
       │
   ┌───▼────────┐
   │  EPD27     │ 2.7" Display Driver
   └───┬────────┘
       │
   ┌───▼────────┐
   │  Device    │ BCM2835 Hardware Layer
   └────────────┘
```

## Supported Hardware & Modes

### Display Hardware

**Currently Implemented:**
- Waveshare 2.7" e-Paper Display (176x264 pixels)

**Easy to Extend:**
The library's clean driver architecture makes it straightforward to add support for other Waveshare e-paper displays. Simply implement the `Driver` interface for your specific display model. See the [Extension Guide](ARCHITECTURE.md#adding-new-display-drivers) for details.

### Display Modes

**Currently Supported:**
- **Black & White Mode** (1 bit per pixel): Pure black and white output
- **4-Level Grayscale Mode** (2 bits per pixel): Black, white, and two gray levels

**Not Currently Supported:**
- Multi-color displays (red, yellow, etc.) would require additional development
- Higher grayscale levels (16-level, etc.) would require driver modifications

Note: The current architecture focuses on monochrome and grayscale displays. Adding color support would require extending the `Color` enum and potentially modifying the display protocol implementation.

> **Documentation Available**: This README provides a quick overview. For detailed architecture diagrams, design patterns, deployment guides, and more, see the [Documentation](#documentation) section below or visit [DOCUMENTATION_INDEX.md](docs/DOCUMENTATION_INDEX.md).

## Components

### Device (`device.hpp`)
Low-level BCM2835 hardware interface with RAII wrappers for GPIO and SPI operations.

### Driver (`drivers/driver.hpp`)
Abstract interface defining the contract for e-paper display drivers. This clean abstraction makes it easy to add support for additional Waveshare displays by implementing this interface.

### EPD27 (`drivers/epd27.hpp`)
Concrete driver implementation for Waveshare 2.7" displays (176x264 pixels). Currently the only implemented driver, but serves as a reference for implementing additional display drivers. Located in the `drivers/` subdirectory for better organization.

### Screen (`screen.hpp`)
Framebuffer management with pixel-level operations and bounds checking. Works with any display driver implementation.

### Draw (`draw.hpp`)
High-level drawing primitives: points, lines, rectangles, circles, text, and numbers. Display-agnostic API that works with all drivers.

### Font (`font.hpp`)
Font rendering system supporting Font8, Font12, Font16, Font20, and Font24.

### Bitmap Drawing
Bitmap/image drawing functionality supporting both raw pixel data and image file loading (PNG, JPEG, BMP, etc.) with scaling capabilities. Uses [stb_image](https://github.com/nothings/stb) for file loading.

## Requirements

- C++23 capable compiler (GCC 14+, Clang 18+)
- CMake 3.25+
- BCM2835 library
- Raspberry Pi with SPI enabled
- Linux kernel with GPIO/SPI support

## Installation

### Install Dependencies

```bash
# Install BCM2835 library
sudo apt-get install libbcm2835-dev

# Install CMake (if not already installed)
sudo apt-get install cmake

# Install a modern compiler
sudo apt-get install g++-14  # or clang-18
```

### Build the Library

```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. -DCMAKE_CXX_COMPILER=g++-14

# Build
cmake --build . -j$(nproc)

# Install (optional)
sudo cmake --install .

# Run the demo
sudo ./examples/epaper_demo
```

## Usage

### Basic Example

```cpp
#include <epaper/device.hpp>
#include <epaper/draw.hpp>
#include <epaper/drivers/epd27.hpp>
#include <epaper/font.hpp>
#include <epaper/screen.hpp>

using namespace epaper;

int main() {
    // Initialize device
    Device device;
    if (auto result = device.init(); !result) {
        return 1;
    }

    // Create driver for 2.7" display
    EPD27 epd27(device);

    // Initialize in black/white mode
    epd27.init(DisplayMode::BlackWhite);
    epd27.clear();

    // Create screen and drawing interface
    Screen screen(epd27);
    Draw draw(screen);

    // Draw shapes
    draw.draw_rectangle(10, 10, 100, 100, Color::Black);
    draw.draw_circle(55, 55, 30, Color::Black);

    // Draw text
    draw.draw_string(10, 110, "Hello World!", Font::font16(),
                    Color::Black, Color::White);

    // Refresh display
    screen.refresh();

    // Put display to sleep
    epd27.sleep();

    return 0;
}
```

### Running the Demo

```bash
# From build directory
sudo ./examples/epaper_demo
```

Note: Root privileges are required for GPIO/SPI access.

## API Overview

### Drawing Operations

```cpp
Draw draw(screen);

// Points
draw.draw_point(x, y, Color::Black, DotPixel::Pixel2x2);

// Lines
draw.draw_line(x1, y1, x2, y2, Color::Black,
              DotPixel::Pixel1x1, LineStyle::Solid);

// Rectangles
draw.draw_rectangle(x1, y1, x2, y2, Color::Black,
                   DotPixel::Pixel1x1, DrawFill::Full);

// Circles
draw.draw_circle(cx, cy, radius, Color::Black,
                DotPixel::Pixel1x1, DrawFill::Empty);

// Text
draw.draw_string(x, y, "Text", Font::font16(),
                Color::Black, Color::White);

// Numbers
draw.draw_number(x, y, 42, Font::font12(),
                Color::Black, Color::White);

// Decimals
draw.draw_decimal(x, y, 3.14159, 3, Font::font12(),
                 Color::Black, Color::White);

// Bitmaps (raw pixel data)
std::vector<Color> pixels = { /* ... */ };
draw.draw_bitmap(x, y, pixels, width, height);

// Bitmaps (from image files - PNG, JPEG, BMP, etc.)
if (auto result = draw.draw_bitmap_from_file(x, y, "image.png",
                                             target_width, target_height);
    !result) {
    // Handle error
}
```

### Bitmap Drawing

The library supports drawing bitmaps from both raw pixel data and image files (PNG, JPEG, BMP, etc.) with optional scaling:

```cpp
// Draw from raw pixel data
std::vector<Color> checkerboard;
for (std::size_t y = 0; y < 32; ++y) {
    for (std::size_t x = 0; x < 32; ++x) {
        bool is_white = ((x / 4) + (y / 4)) % 2 == 0;
        checkerboard.push_back(is_white ? Color::White : Color::Black);
    }
}
draw.draw_bitmap(10, 10, checkerboard, 32, 32);

// Draw from image file with scaling
draw.draw_bitmap_from_file(10, 50, "logo.png", 100, 100);
```

For detailed information on bitmap drawing, including supported formats, color conversion, and scaling, see [BITMAP_DRAWING.md](docs/BITMAP_DRAWING.md).

**Test Images:** A collection of 30+ test images in various formats is included. Generate them with:
```bash
python3 generate_test_images.py
```

See [TEST_IMAGES_GUIDE.md](docs/TEST_IMAGES_GUIDE.md) for a complete visual guide to all test images.

### Display Modes

**Black & White Mode** (1 bit per pixel):
- `Color::White`
- `Color::Black`

**4-Level Grayscale Mode** (2 bits per pixel):
- `Color::White` (lightest)
- `Color::Gray1`
- `Color::Gray2`
- `Color::Black` (darkest)

### Screen Operations

```cpp
Screen screen(driver);

// Pixel operations
screen.set_pixel(x, y, Color::Black);
auto color = screen.get_pixel(x, y);

// Clear operations
screen.clear(Color::White);
screen.clear_region(x1, y1, x2, y2, Color::Black);

// Refresh display
screen.refresh();
```

## Project Structure

```
e-Paper/
├── CMakeLists.txt              # Root CMake configuration
├── README.md                   # This file
├── include/epaper/             # Public headers
│   ├── device.hpp              # BCM2835 hardware layer
│   ├── drivers/                # Display driver interfaces
│   │   ├── driver.hpp          # Abstract driver interface
│   │   └── epd27.hpp           # 2.7" display driver
│   ├── screen.hpp              # Framebuffer management
│   ├── draw.hpp                # Drawing primitives
│   └── font.hpp                # Font rendering
├── src/                        # Implementation files
│   ├── device.cpp
│   ├── drivers/                # Display driver implementations
│   │   └── epd27.cpp           # 2.7" display driver
│   ├── screen.cpp
│   ├── draw.cpp
│   └── font.cpp
├── examples/                   # Example applications
│   ├── CMakeLists.txt
│   └── demo.cpp
└── fonts/                      # Font data files
    ├── fonts.h
    ├── font8.c
    ├── font12.c
    ├── font16.c
    ├── font20.c
    └── font24.c
```

## Design Principles

1. **KISS (Keep It Simple, Stupid)**: Simple, straightforward implementations
2. **Composition Over Inheritance**: Minimal use of inheritance
3. **RAII Everywhere**: Automatic resource management
4. **No Manual Memory Management**: Uses modern containers and smart pointers
5. **Type Safety**: Strong typing with enums and type-safe wrappers
6. **Const Correctness**: Proper use of const throughout
7. **Move Semantics**: Efficient resource transfers

## Hardware Connections

Default GPIO pin assignments for Raspberry Pi:

| E-Paper Pin | GPIO Pin | BCM Pin |
|-------------|----------|---------|
| RST         | GPIO 17  | 17      |
| DC          | GPIO 25  | 25      |
| CS          | GPIO 8   | CE0     |
| BUSY        | GPIO 24  | 24      |
| MOSI        | GPIO 10  | MOSI    |
| SCLK        | GPIO 11  | SCLK    |
| GND         | GND      | -       |
| VCC         | 3.3V     | -       |

## Extending to Other Waveshare Displays

The library is designed to make adding support for other Waveshare e-paper displays straightforward. The abstract `Driver` interface provides a clean contract that any display implementation must fulfill.

**Important Note:** The current implementation supports **monochrome and grayscale displays only** (black/white and 4-level grayscale). Adding support for multi-color displays (red, yellow, etc.) would require:
- Extending the `Color` enum with additional color values
- Modifying the framebuffer encoding in `Screen` class
- Implementing color-specific LUT tables and display protocols in drivers

### Adding a New Display Driver

To add support for a new Waveshare **monochrome/grayscale** display (e.g., 4.2" EPD42):

1. **Create a new driver class** implementing the `Driver` interface:

```cpp
// include/epaper/drivers/epd42.hpp
class EPD42 : public Driver {
public:
    static constexpr std::size_t WIDTH = 400;
    static constexpr std::size_t HEIGHT = 300;

    explicit EPD42(Device& device);

    auto init(DisplayMode mode) -> std::expected<void, DriverError> override;
    auto clear() -> void override;
    auto display(std::span<const std::byte> buffer) -> void override;
    auto sleep() -> void override;

    auto width() const noexcept -> std::size_t override { return WIDTH; }
    auto height() const noexcept -> std::size_t override { return HEIGHT; }
    auto mode() const noexcept -> DisplayMode override { return current_mode_; }
    auto buffer_size() const noexcept -> std::size_t override;

private:
    // Hardware-specific implementation details
    auto reset() -> void;
    auto send_command(std::uint8_t command) -> void;
    auto send_data(std::uint8_t data) -> void;
    auto wait_busy() -> void;
    auto set_lut() -> void;

    Device& device_;
    DisplayMode current_mode_;
    bool initialized_;
};
```

2. **Implement the driver** in `src/drivers/epd42.cpp` following the display's datasheet specifications.

3. **Use with existing components** - no changes needed to `Screen`, `Draw`, or `Font`:

```cpp
Device device;
device.init().value();

EPD42 epd42(device);  // New display driver
epd42.init(DisplayMode::BlackWhite).value();

Screen screen(epd42);  // Works automatically!
Draw draw(screen);     // All drawing APIs work!
```

### What Makes This Easy

- **Separation of Concerns**: Display protocol is isolated in the driver
- **Dependency Injection**: Higher layers receive drivers via reference
- **Interface Segregation**: Minimal, focused interface contract
- **No Coupling**: `Screen` and `Draw` don't know about specific drivers

See [ARCHITECTURE.md](docs/ARCHITECTURE.md#adding-new-display-drivers) for detailed information about the extension architecture.

## License

This project builds upon the Waveshare e-Paper library while using modern C++ practices.
Please refer to the original Waveshare license in the `lib/` directory for font and display driver heritage.

## Troubleshooting

### Permission Denied
Run with `sudo` to access GPIO/SPI:
```bash
sudo ./epaper_demo
```

### SPI Not Enabled
Enable SPI on Raspberry Pi:
```bash
sudo raspi-config
# Interface Options -> SPI -> Enable
```

### Compilation Errors
Ensure you're using a C++23-capable compiler:
```bash
g++ --version  # Should be 14+
clang++ --version  # Should be 18+
```

## Documentation

This project includes comprehensive architecture documentation with 46+ diagrams covering all aspects of the system.

### Available Documentation

| Document | Description | Best For |
|----------|-------------|----------|
| **[README.md](README.md)** | Getting started guide (this file) | Installation, basic usage, quick reference |
| **[ARCHITECTURE.md](docs/ARCHITECTURE.md)** | Complete architecture guide (850+ lines, 15+ diagrams) | Understanding system design, design patterns, deep dive |
| **[ARCHITECTURE_QUICK_REFERENCE.md](docs/ARCHITECTURE_QUICK_REFERENCE.md)** | Developer cheat sheet (650+ lines, 12+ diagrams) | API lookup, common patterns, troubleshooting |
| **[ARCHITECTURE_VISUAL_SUMMARY.md](docs/ARCHITECTURE_VISUAL_SUMMARY.md)** | High-level visual overview (520+ lines, 15+ diagrams) | Big picture understanding, visual learners |
| **[DEPLOYMENT_ARCHITECTURE.md](docs/DEPLOYMENT_ARCHITECTURE.md)** | System integration guide (750+ lines, 18+ diagrams) | Production deployment, system integration |
| **[DOCUMENTATION_INDEX.md](docs/DOCUMENTATION_INDEX.md)** | Navigation guide for all docs | Finding specific topics, learning paths |

### Quick Links by Purpose

**Getting Started?**
1. Start here: [README.md](README.md)
2. See the big picture: [ARCHITECTURE_VISUAL_SUMMARY.md](docs/ARCHITECTURE_VISUAL_SUMMARY.md)
3. Try the examples: [examples/demo.cpp](examples/demo.cpp)

**Developing Code?**
- Keep open: [ARCHITECTURE_QUICK_REFERENCE.md](docs/ARCHITECTURE_QUICK_REFERENCE.md)
- API reference, wiring diagrams, common patterns

**Understanding Design?**
- Read: [ARCHITECTURE.md](docs/ARCHITECTURE.md)
- Complete system architecture with class diagrams, sequence diagrams, design patterns

**Deploying to Production?**
- Follow: [DEPLOYMENT_ARCHITECTURE.md](docs/DEPLOYMENT_ARCHITECTURE.md)
- System integration, deployment scenarios, security considerations

**Need Navigation Help?**
- Use: [DOCUMENTATION_INDEX.md](docs/DOCUMENTATION_INDEX.md)
- Find any topic quickly with learning paths and feature matrix

## Contributing

This is a demonstration project showing modern C++ practices for embedded systems.
Feel free to adapt and extend for your own projects.

**Contributions Welcome:**
- Additional Waveshare display drivers (4.2", 7.5", etc.)
- New drawing primitives
- Performance optimizations
- Documentation improvements

### Project Standards

#### C++ Standards

- **C++23**: This project requires C++23 features (`std::expected`, `std::span`, concepts, ranges)
- **Compiler Requirements**: GCC 14+ or Clang 18+
- **Standard Library**: Use modern C++23 standard library features where appropriate

#### Code Style

- **Formatting**: Code must be formatted using `clang-format` with the provided `.clang-format` configuration
  ```bash
  clang-format -i src/**/*.cpp include/**/*.hpp
  ```
- **Indentation**: 2 spaces (configured in `.clang-format`)
- **Line Length**: 120 characters maximum
- **Trailing Return Types**: Use trailing return type syntax (`auto func() -> return_type`)
- **Naming Conventions**:
  - Classes: `PascalCase` (e.g., `Device`, `Screen`)
  - Functions: `snake_case` (e.g., `draw_line`, `set_pixel`)
  - Variables: `snake_case` (e.g., `device_`, `screen_`)
  - Constants: `PascalCase` or `UPPER_CASE` depending on context
- **Includes**: Alphabetically sorted, with system headers before project headers
- **Const Correctness**: Use `const` and `constexpr` appropriately
- **RAII**: Prefer RAII patterns for resource management
- **No Raw Pointers**: Use modern C++ containers and smart pointers

#### Compiler Flags

The project uses strict compiler warnings:
- `-Wall -Wextra -Wpedantic`
- `-Wconversion -Wsign-conversion`
- `-Werror=return-type`

All code must compile without warnings.

#### Design Principles

Follow the project's design principles:
1. **KISS (Keep It Simple, Stupid)**: Simple, straightforward implementations
2. **Composition Over Inheritance**: Minimal use of inheritance
3. **RAII Everywhere**: Automatic resource management
4. **No Manual Memory Management**: Use modern containers and smart pointers
5. **Type Safety**: Strong typing with enums and type-safe wrappers
6. **Const Correctness**: Proper use of const throughout
7. **Move Semantics**: Efficient resource transfers

### Commit Messages

This project uses **Semantic Commits** (Conventional Commits format) for clear, consistent commit history.

#### Commit Format

```
<type>(<scope>): <subject>

<body>

<footer>
```

#### Commit Types

- `feat`: A new feature
- `fix`: A bug fix
- `docs`: Documentation only changes
- `style`: Code style changes (formatting, missing semicolons, etc.)
- `refactor`: Code refactoring without changing functionality
- `perf`: Performance improvements
- `test`: Adding or updating tests
- `chore`: Maintenance tasks, build system changes, dependency updates
- `build`: Changes to build system or external dependencies

#### Scope (Optional)

The scope should be the name of the component affected:
- `device`: BCM2835 hardware layer
- `driver`: Display driver implementations
- `screen`: Framebuffer management
- `draw`: Drawing primitives
- `font`: Font rendering
- `cmake`: Build system changes
- `docs`: Documentation

#### Examples

```
feat(driver): add EPD42 display driver support

Implements the Driver interface for Waveshare 4.2" displays.
Supports both black/white and 4-level grayscale modes.

fix(screen): correct buffer bounds checking in set_pixel

The previous implementation allowed out-of-bounds access.
Now properly validates coordinates before pixel operations.

docs(readme): add contribution guidelines for C++ standards

docs(draw): update API documentation for new circle drawing method

refactor(device): simplify SPI transfer implementation

perf(screen): optimize framebuffer refresh using bulk transfers

style: format code with clang-format

chore: update CMake minimum version to 3.25
```

#### Commit Message Guidelines

- Use the imperative mood ("add" not "added" or "adds")
- Keep the subject line under 72 characters
- Capitalize the first letter of the subject
- Do not end the subject with a period
- Separate subject from body with a blank line
- Wrap the body at 72 characters
- Use the body to explain *what* and *why* vs. *how*
- Reference issues and pull requests in the footer

### Pull Request Process

1. Ensure your code follows all project standards
2. Format code with `clang-format`
3. Verify code compiles without warnings
4. Update documentation as needed
5. Use semantic commit messages
6. Keep pull requests focused on a single feature or fix

## Acknowledgments

- Waveshare for the original e-Paper library and hardware
- BCM2835 library authors for Raspberry Pi GPIO access

