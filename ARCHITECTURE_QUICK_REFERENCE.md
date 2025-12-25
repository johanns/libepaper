# E-Paper Library - Quick Reference Guide

## Component Dependency Graph

```mermaid
graph TD
    App[Application<br/>demo.cpp]

    Draw[Draw<br/>High-level graphics API]
    Font[Font<br/>Text rendering]
    Screen[Screen<br/>Framebuffer]

    Driver[Driver<br/>Abstract interface]
    EPD27[EPD27<br/>2.7 display driver]

    Device[Device<br/>BCM2835 HAL]

    BCM[bcm2835 library]
    Hardware[Hardware<br/>GPIO/SPI/Display]

    App --> Draw
    App --> Screen
    App --> EPD27
    App --> Device

    Draw --> Font
    Draw --> Screen

    Screen --> EPD27
    EPD27 -.-> Driver
    EPD27 --> Device

    Device --> BCM
    BCM --> Hardware

    classDef app fill:#e3f2fd,stroke:#1976d2,stroke-width:3px
    classDef api fill:#fff9c4,stroke:#f57c00,stroke-width:2px
    classDef buffer fill:#c8e6c9,stroke:#388e3c,stroke-width:2px
    classDef driver fill:#ffccbc,stroke:#d84315,stroke-width:2px
    classDef hal fill:#f8bbd0,stroke:#c2185b,stroke-width:2px
    classDef external fill:#cfd8dc,stroke:#455a64,stroke-width:2px

    class App app
    class Draw,Font api
    class Screen buffer
    class Driver,EPD27 driver
    class Device hal
    class BCM,Hardware external
```

---

## Typical Usage Flow

```mermaid
flowchart TD
    Start([Start Application]) --> Init1[Create Device]
    Init1 --> Init2[Initialize Device]
    Init2 --> Init3[Create EPD27 Driver]
    Init3 --> Init4[Initialize Display Mode]
    Init4 --> Init5[Clear Display]

    Init5 --> Create1[Create Screen]
    Create1 --> Create2[Create Draw]

    Create2 --> Draw1{Drawing Operations}

    Draw1 -->|Text| DrawText[draw_string/draw_char]
    Draw1 -->|Shapes| DrawShapes[draw_rectangle/draw_circle]
    Draw1 -->|Lines| DrawLines[draw_line/draw_point]
    Draw1 -->|Numbers| DrawNumbers[draw_number/draw_decimal]

    DrawText --> More{More Drawing?}
    DrawShapes --> More
    DrawLines --> More
    DrawNumbers --> More

    More -->|Yes| Draw1
    More -->|No| Refresh[screen.refresh]

    Refresh --> Display[Physical Display Updates<br/>2-3 seconds]

    Display --> Continue{Continue?}
    Continue -->|Yes| Draw1
    Continue -->|No| Sleep[epd27.sleep]

    Sleep --> End([End Application])

    style Start fill:#c8e6c9
    style End fill:#c8e6c9
    style Init2 fill:#fff9c4
    style Init4 fill:#fff9c4
    style Refresh fill:#ffccbc
    style Display fill:#ffccbc
    style Sleep fill:#f8bbd0
```

---

## Display Modes Comparison

```mermaid
graph TB
    subgraph "Black & White Mode"
        BW_Header[DisplayMode::BlackWhite]
        BW_Bits[1 bit per pixel]
        BW_Colors[2 colors]
        BW_Size[5,808 bytes buffer]
        BW_Fast[Faster refresh]

        BW_Header --> BW_Bits
        BW_Bits --> BW_Colors
        BW_Colors --> BW_Size
        BW_Size --> BW_Fast
    end

    subgraph "4-Level Grayscale Mode"
        GS_Header[DisplayMode::Grayscale4]
        GS_Bits[2 bits per pixel]
        GS_Colors[4 gray levels]
        GS_Size[11,616 bytes buffer]
        GS_Quality[Better quality]

        GS_Header --> GS_Bits
        GS_Bits --> GS_Colors
        GS_Colors --> GS_Size
        GS_Size --> GS_Quality
    end

    style BW_Header fill:#ffffff,stroke:#000000,stroke-width:3px
    style GS_Header fill:#cccccc,stroke:#000000,stroke-width:3px
```

### Color Values by Mode

| Color | B/W Value | Grayscale Value | Visual |
|-------|-----------|-----------------|--------|
| `Color::White` | 0xFF | 0xFF | ⬜ Lightest |
| `Color::Gray1` | N/A | 0x80 | ◻️ Light gray |
| `Color::Gray2` | N/A | 0x40 | ◽ Dark gray |
| `Color::Black` | 0x00 | 0x00 | ⬛ Darkest |

---

## API Cheat Sheet

### Initialization Pattern

```cpp
// 1. Hardware layer
Device device;
device.init().value();  // or handle error

// 2. Driver layer
EPD27 epd27(device);
epd27.init(DisplayMode::BlackWhite).value();
epd27.clear();

// 3. Application layers
Screen screen(epd27);
Draw draw(screen);
```

### Drawing Operations

```cpp
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
draw.draw_string(x, y, "Hello", Font::font16(),
    Color::Black, Color::White);

// Numbers
draw.draw_number(x, y, 42, Font::font12(),
    Color::Black, Color::White);

// Decimals
draw.draw_decimal(x, y, 3.14, 2, Font::font12(),
    Color::Black, Color::White);
```

### Screen Operations

```cpp
// Direct pixel access
screen.set_pixel(x, y, Color::Black);
Color c = screen.get_pixel(x, y);

// Clearing
screen.clear(Color::White);
screen.clear_region(x1, y1, x2, y2, Color::White);

// Update display
screen.refresh();  // Sends buffer to hardware
```

---

## Memory Layout Visualization

### Black & White Buffer (176x264 = 5,808 bytes)

```
Pixel Layout (left to right):
┌─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┐
│ P0  │ P1  │ P2  │ P3  │ P4  │ P5  │ P6  │ P7  │  = 1 byte
└─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┘
  bit7  bit6  bit5  bit4  bit3  bit2  bit1  bit0

Each bit: 0 = Black, 1 = White
```

### Grayscale Buffer (176x264 = 11,616 bytes)

```
Pixel Layout (left to right):
┌─────────┬─────────┬─────────┬─────────┐
│   P0    │   P1    │   P2    │   P3    │  = 1 byte
│ 2 bits  │ 2 bits  │ 2 bits  │ 2 bits  │
└─────────┴─────────┴─────────┴─────────┘
  bit7-6    bit5-4    bit3-2    bit1-0

2-bit values: 00=Black, 01=Gray2, 10=Gray1, 11=White
```

---

## Hardware Wiring Diagram

```mermaid
graph LR
    subgraph "Raspberry Pi"
        VCC_3V3[3.3V Power]
        GND[Ground]
        GPIO17[GPIO 17]
        GPIO25[GPIO 25]
        GPIO8[GPIO 8 CE0]
        GPIO24[GPIO 24]
        GPIO10[GPIO 10 MOSI]
        GPIO11[GPIO 11 SCLK]
    end

    subgraph "E-Paper Display"
        DISP_VCC[VCC]
        DISP_GND[GND]
        DISP_RST[RST]
        DISP_DC[DC]
        DISP_CS[CS]
        DISP_BUSY[BUSY]
        DISP_DIN[DIN]
        DISP_CLK[CLK]
    end

    VCC_3V3 -->|Red| DISP_VCC
    GND -->|Black| DISP_GND
    GPIO17 -->|White| DISP_RST
    GPIO25 -->|Yellow| DISP_DC
    GPIO8 -->|Orange| DISP_CS
    GPIO24 -->|Green| DISP_BUSY
    GPIO10 -->|Blue| DISP_DIN
    GPIO11 -->|Purple| DISP_CLK

    style VCC_3V3 fill:#ff6b6b
    style GPIO17 fill:#ffffff,stroke:#000000
    style GPIO25 fill:#ffeb3b
    style GPIO8 fill:#ff9800
    style GPIO24 fill:#4caf50
    style GPIO10 fill:#2196f3
    style GPIO11 fill:#9c27b0
```

### Pin Function Summary

| Pin | Type | Function |
|-----|------|----------|
| **VCC** | Power | 3.3V supply |
| **GND** | Power | Ground reference |
| **RST** | Output | Hardware reset (active LOW) |
| **DC** | Output | Data/Command select (LOW=cmd, HIGH=data) |
| **CS** | Output | SPI chip select (active LOW) |
| **BUSY** | Input | Display busy status (HIGH=busy) |
| **DIN** | Output | SPI data (MOSI) |
| **CLK** | Output | SPI clock |

---

## Error Handling Quick Reference

### Error Types

```mermaid
graph TB
    E[Error Handling]

    E --> D[DeviceError]
    E --> R[DriverError]

    D --> D1[InitializationFailed]
    D --> D2[SPIInitFailed]
    D --> D3[InvalidPin]
    D --> D4[TransferFailed]

    R --> R1[NotInitialized]
    R --> R2[InitializationFailed]
    R --> R3[InvalidMode]
    R --> R4[TransferFailed]
    R --> R5[Timeout]

    style E fill:#fff9c4
    style D fill:#ffcdd2
    style R fill:#ffcdd2
```

### Error Handling Pattern

```cpp
// Using std::expected (C++23)
if (auto result = device.init(); !result) {
    std::cerr << "Error: " << to_string(result.error()) << "\n";
    return EXIT_FAILURE;
}

// Alternative: value() throws if error
device.init().value();  // throws std::bad_expected_access
```

---

## Performance Tips

```mermaid
flowchart TD
    A[Performance Optimization]

    A --> B[Batch Drawing Operations]
    A --> C[Minimize Refreshes]
    A --> D[Use Appropriate Mode]
    A --> E[Pre-calculate Positions]

    B --> B1[Multiple draw calls<br/>Single refresh]
    C --> C1[Only refresh when complete<br/>~2-3s per refresh]
    D --> D1[B/W for simple graphics<br/>Grayscale for photos]
    E --> E1[Cache coordinates<br/>Avoid recalculation]

    style A fill:#fff9c4
    style B1 fill:#c8e6c9
    style C1 fill:#c8e6c9
    style D1 fill:#c8e6c9
    style E1 fill:#c8e6c9
```

### Timing Measurements

```mermaid
gantt
    title E-Paper Display Operation Timeline
    dateFormat X
    axisFormat %Ls

    section Init
    Hardware Reset    :0, 200
    LUT Configuration :200, 300

    section Drawing
    draw_string (10 chars) :500, 1
    draw_rectangle  :501, 1
    draw_circle     :502, 1
    Multiple operations :503, 5

    section Display
    SPI Transfer (5KB) :600, 400
    Display Refresh    :1000, 2000
```

**Key Takeaways**:
- Drawing operations: microseconds to milliseconds
- Display refresh: 2-3 seconds (hardware limitation)
- **Strategy**: Batch all drawing, then refresh once

---

## Font Size Reference

```mermaid
graph TB
    subgraph "Available Fonts"
        F8[Font8<br/>5x8 pixels<br/>Small labels]
        F12[Font12<br/>7x12 pixels<br/>Regular text]
        F16[Font16<br/>11x16 pixels<br/>Headers]
        F20[Font20<br/>14x20 pixels<br/>Large text]
        F24[Font24<br/>17x24 pixels<br/>Titles]
    end

    F8 --> Usage1[Compact displays]
    F12 --> Usage2[Body text]
    F16 --> Usage3[Emphasis]
    F20 --> Usage4[Headings]
    F24 --> Usage5[Large labels]

    style F8 fill:#e3f2fd
    style F12 fill:#bbdefb
    style F16 fill:#90caf9
    style F20 fill:#64b5f6
    style F24 fill:#42a5f5
```

### Font Usage

```cpp
// Access built-in fonts
const Font& f8  = Font::font8();   // Smallest
const Font& f12 = Font::font12();  // Default
const Font& f16 = Font::font16();  // Medium
const Font& f20 = Font::font20();  // Large
const Font& f24 = Font::font24();  // Largest

// Query font metrics
auto width  = f16.width();   // Character width
auto height = f16.height();  // Character height
```

---

## Common Patterns

### Pattern 1: Full Screen Update

```cpp
void update_display(Screen& screen, Draw& draw) {
    // Clear to white
    screen.clear(Color::White);

    // Draw all content
    draw.draw_string(10, 10, "Title", Font::font24(),
        Color::Black, Color::White);
    draw.draw_rectangle(10, 40, 166, 42, Color::Black);
    draw.draw_string(10, 50, "Content", Font::font16(),
        Color::Black, Color::White);

    // Single refresh at end
    screen.refresh();
}
```

### Pattern 2: Partial Region Update

```cpp
void update_region(Screen& screen, Draw& draw,
                   int x, int y, int w, int h) {
    // Clear region
    screen.clear_region(x, y, x+w, y+h, Color::White);

    // Draw in region
    draw.draw_string(x, y, "Updated", Font::font12(),
        Color::Black, Color::White);

    // Refresh (updates entire display)
    screen.refresh();
}
```

### Pattern 3: Mode Switching

```cpp
void switch_mode(EPD27& epd27, DisplayMode new_mode) {
    // Re-initialize with new mode
    epd27.init(new_mode).value();

    // Create new screen (buffer resized automatically)
    Screen screen(epd27);
    Draw draw(screen);

    // Continue with drawing operations
    // ...
}
```

### Pattern 4: Error-Safe Initialization

```cpp
auto safe_init() -> std::expected<std::tuple<Device, EPD27, Screen, Draw>, std::string> {
    Device device;

    if (auto result = device.init(); !result) {
        return std::unexpected(std::string(to_string(result.error())));
    }

    EPD27 epd27(device);

    if (auto result = epd27.init(DisplayMode::BlackWhite); !result) {
        return std::unexpected(std::string(to_string(result.error())));
    }

    epd27.clear();

    Screen screen(epd27);
    Draw draw(screen);

    return std::make_tuple(
        std::move(device),
        std::move(epd27),
        std::move(screen),
        std::move(draw)
    );
}
```

---

## Display States

```mermaid
stateDiagram-v2
    [*] --> PowerOff
    PowerOff --> Init : device.init()

    Init --> Ready : epd27.init(mode)
    Init --> Error : initialization fails

    Ready --> Drawing : draw operations
    Drawing --> Drawing : more operations
    Drawing --> Ready : operations complete

    Ready --> Refreshing : screen.refresh()
    Refreshing --> Ready : refresh complete (2-3s)

    Ready --> Sleep : epd27.sleep()
    Sleep --> Init : wake up

    Ready --> ModeSwitch : epd27.init(new_mode)
    ModeSwitch --> Ready : mode changed

    Error --> [*]
    Sleep --> [*] : power off

    note right of Drawing
        All drawing operations
        update framebuffer only.
        No hardware access yet.
    end note

    note right of Refreshing
        BUSY pin HIGH
        SPI transfer active
        Physical display updating
        ~2-3 seconds
    end note
```

---

## Troubleshooting Guide

```mermaid
flowchart TD
    Problem{Problem?}

    Problem -->|Display not working| P1[Check connections]
    Problem -->|Permission denied| P2[Run with sudo]
    Problem -->|SPI error| P3[Enable SPI]
    Problem -->|Compilation error| P4[Check C++ version]
    Problem -->|Display frozen| P5[Check BUSY pin]

    P1 --> S1[Verify wiring matches<br/>pin configuration]
    P2 --> S2[sudo ./program<br/>or add user to gpio group]
    P3 --> S3[sudo raspi-config<br/>Interface -> SPI -> Enable]
    P4 --> S4[Ensure GCC 14+ or Clang 18+<br/>C++23 support required]
    P5 --> S5[Ensure BUSY pin connected<br/>Check wait_busy timeout]

    S1 --> Fix[Problem Solved]
    S2 --> Fix
    S3 --> Fix
    S4 --> Fix
    S5 --> Fix

    style Problem fill:#fff9c4
    style Fix fill:#c8e6c9
    style P1 fill:#ffcdd2
    style P2 fill:#ffcdd2
    style P3 fill:#ffcdd2
    style P4 fill:#ffcdd2
    style P5 fill:#ffcdd2
```

---

## Build System Overview

```mermaid
graph TD
    CMake[CMakeLists.txt<br/>Root] --> Lib[Library Target<br/>libepaper.a]
    CMake --> Example[Example Target<br/>epaper_demo]

    Lib --> Src1[device.cpp]
    Lib --> Src2[epd27.cpp]
    Lib --> Src3[screen.cpp]
    Lib --> Src4[draw.cpp]
    Lib --> Src5[font.cpp]
    Lib --> Fonts[Font data files<br/>font8-24.c]

    Example --> Demo[demo.cpp]
    Example --> Lib

    Lib --> Headers[Public Headers<br/>include/epaper/]

    style CMake fill:#fff9c4
    style Lib fill:#c8e6c9
    style Example fill:#e3f2fd
```

### Build Commands

```bash
# Configure
cmake -B build -DCMAKE_CXX_COMPILER=g++-14

# Build library
cmake --build build --target epaper

# Build demo
cmake --build build --target epaper_demo

# Build everything
cmake --build build -j$(nproc)

# Run demo
sudo ./build/examples/epaper_demo
```

---

## Key Concepts Summary

### Dependency Injection
Every component receives its dependencies through constructor parameters, enabling loose coupling and testability.

### RAII Pattern
Resources are acquired in constructors and released in destructors automatically, preventing leaks.

### std::expected
Modern error handling without exceptions, providing explicit success/failure paths.

### Type Safety
Enums and type-safe wrappers (like `Pin`) prevent misuse and improve code clarity.

### Zero-Cost Abstractions
High-level interfaces compile down to efficient machine code with no runtime overhead.

---

## Quick Links

- **Full Architecture**: See `ARCHITECTURE.md` for detailed diagrams and explanations
- **User Guide**: See `README.md` for usage examples and installation
- **API Reference**: See header files in `include/epaper/`
- **Examples**: See `examples/demo.cpp` for working code

---

*This quick reference is designed for fast lookup during development. Refer to the full architecture documentation for in-depth explanations.*

