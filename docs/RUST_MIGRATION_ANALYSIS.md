# Rust Migration Analysis

## Executive Summary

This document analyzes the effort required to migrate the libepaper C++ codebase to **idiomatic Rust**, focusing on design and architecture changes rather than direct translation. The migration requires approximately **4-6 weeks** for a skilled Rust developer, with significant architectural improvements possible through Rust's type system and ownership model.

**Key Statistics:**
- **Source Files**: 4 implementation files (~1,400 LOC)
- **Header Files**: 9 header files (~1,800 LOC)
- **Estimated Rust LOC**: ~2,500 LOC (includes additional safety guarantees)
- **Complexity**: Medium-High (hardware interfacing, embedded systems)

---

## Table of Contents

1. [Current Architecture Overview](#current-architecture-overview)
2. [Idiomatic Rust Design Principles](#idiomatic-rust-design-principles)
3. [Component-by-Component Analysis](#component-by-component-analysis)
4. [Architectural Changes for Rust](#architectural-changes-for-rust)
5. [Crate Structure & Dependencies](#crate-structure--dependencies)
6. [Effort Estimation](#effort-estimation)
7. [Risks & Challenges](#risks--challenges)
8. [Migration Strategy](#migration-strategy)
9. [Benefits of Rust Migration](#benefits-of-rust-migration)

---

## Current Architecture Overview

### C++ Architecture Layers

```
┌─────────────────────────────────────────┐
│   Display API (display.hpp/cpp)        │  High-level drawing operations
├─────────────────────────────────────────┤
│   Driver Interface (driver.hpp)        │  Abstract hardware interface
│   └── EPD27 Driver (epd27.hpp/cpp)     │  Concrete implementation
├─────────────────────────────────────────┤
│   Device Layer (device.hpp/cpp)        │  GPIO/SPI wrapper (libgpiod/SPIdev)
├─────────────────────────────────────────┤
│   Utilities (font.hpp/cpp, errors.hpp) │  Support types & error handling
└─────────────────────────────────────────┘
```

### Key Design Patterns in C++

1. **PImpl Pattern**: `Device` uses unique_ptr to hide implementation details
2. **RAII**: Resource cleanup via destructors
3. **std::expected**: Modern error handling (C++23)
4. **Virtual Interface**: Abstract `Driver` for polymorphism
5. **Composition**: Display owns driver, driver references device
6. **Move Semantics**: Non-copyable resources

---

## Idiomatic Rust Design Principles

### What Makes Rust Code "Idiomatic"?

Rather than translating C++ line-by-line, idiomatic Rust emphasizes:

1. **Ownership & Borrowing**: Replace raw pointers and references with Rust's ownership system
2. **Traits Instead of Inheritance**: Replace virtual classes with trait objects or generics
3. **Error Handling**: Use `Result<T, E>` extensively (similar to C++23's `std::expected`)
4. **Zero-Cost Abstractions**: Leverage generics and const generics for compile-time dispatch
5. **Type Safety**: Encode invariants in the type system (e.g., `NonZeroU8`, phantom types)
6. **Pattern Matching**: Replace switch/if-else chains with exhaustive match expressions
7. **Iterators & Combinators**: Functional programming with `.map()`, `.filter()`, `.collect()`
8. **Explicit Mutability**: Clear distinction between `&T` and `&mut T`
9. **No Hidden Allocations**: Explicit `Box<T>`, `Vec<T>`, `Arc<T>`
10. **Fearless Concurrency**: Thread safety enforced by compiler (Send/Sync traits)

---

## Component-by-Component Analysis

### 1. Device Layer (GPIO/SPI Interface)

**Current C++ Implementation:**
```cpp
// device.cpp (327 LOC)
struct DeviceImpl {
  gpiod_chip *chip;
  gpiod_line_request *line_request;
  unordered_map<uint8_t, PinConfig> pin_configs;
  int spi_fd;
  bool initialized;
};

class Device {
  unique_ptr<DeviceImpl> pimpl_;
  // Manual cleanup, move semantics
};
```

**Idiomatic Rust Design:**

```rust
// device.rs
use std::collections::HashMap;
use std::fs::File;
use std::os::unix::io::{AsRawFd, RawFd};

/// RAII wrapper for GPIO chip using gpiod crate
pub struct GpioChip {
    chip: gpiod::Chip,
    lines: gpiod::Lines,
}

/// RAII wrapper for SPI device using spidev crate
pub struct SpiDevice {
    device: spidev::Spidev,
}

/// Main hardware abstraction with automatic cleanup
pub struct Device {
    gpio: Option<GpioChip>,
    spi: Option<SpiDevice>,
    pin_configs: HashMap<u8, PinConfig>,
}

impl Device {
    /// Initialize hardware with idiomatic error handling
    pub fn init() -> Result<Self, DeviceError> {
        let gpio = GpioChip::open("/dev/gpiochip0")?;
        let spi = SpiDevice::open("/dev/spidev0.0")?
            .with_mode(spidev::SPI_MODE_0)
            .with_speed_hz(1_953_125)
            .with_bits_per_word(8)
            .build()?;
        
        Ok(Self {
            gpio: Some(gpio),
            spi: Some(spi),
            pin_configs: HashMap::new(),
        })
    }
    
    /// Set pin as output (returns Result for error handling)
    pub fn set_pin_output(&mut self, pin: Pin) -> Result<(), DeviceError> {
        // Rust prevents use-after-free and double-free automatically
    }
}

// Drop trait replaces C++ destructor
impl Drop for Device {
    fn drop(&mut self) {
        // Automatic cleanup - no manual gpiod_line_request_release needed
        // Rust's ownership ensures correct order of drops
    }
}
```

**Key Rust Improvements:**

1. **No PImpl Needed**: Rust's module system provides privacy without heap allocation
2. **Automatic Resource Management**: `Drop` trait replaces destructors, guaranteed to run
3. **No Manual Move Semantics**: Rust's ownership system handles this automatically
4. **Builder Pattern**: `SpiDevice::open().with_mode().build()` is more idiomatic than ioctl
5. **Interior Mutability**: Can use `RefCell` or `Mutex` if internal mutation needed
6. **Type-Safe Pins**: Use newtypes or enums to prevent invalid pin numbers at compile-time

**Migration Complexity:** Medium
- **LOC**: ~250 lines (simpler than C++)
- **Time**: 2-3 days (includes learning gpiod/spidev crates)
- **Challenges**: FFI bindings to gpiod library, SPI configuration API differences

---

### 2. Error Handling

**Current C++ Implementation:**
```cpp
// errors.hpp (158 LOC)
enum class ErrorCode { DeviceNotInitialized, GPIOInitFailed, ... };
struct Error { ErrorCode code; string message; };
```

**Idiomatic Rust Design:**

```rust
// error.rs
use thiserror::Error;

/// Comprehensive error types using thiserror for Display impl
#[derive(Debug, Error)]
pub enum DeviceError {
    #[error("GPIO initialization failed: {0}")]
    GpioInit(#[from] gpiod::Error),
    
    #[error("SPI device open failed: {0}")]
    SpiOpen(#[from] std::io::Error),
    
    #[error("SPI configuration failed")]
    SpiConfig,
    
    #[error("Invalid pin number: {0}")]
    InvalidPin(u8),
}

#[derive(Debug, Error)]
pub enum DriverError {
    #[error("Driver not initialized")]
    NotInitialized,
    
    #[error("Invalid display mode")]
    InvalidMode,
    
    #[error("Operation timed out")]
    Timeout,
    
    #[error("Device error: {0}")]
    Device(#[from] DeviceError),
}

#[derive(Debug, Error)]
pub enum DisplayError {
    #[error("Display refresh failed")]
    RefreshFailed,
    
    #[error("File not found: {0}")]
    FileNotFound(String),
    
    #[error("Image load failed: {0}")]
    ImageLoad(#[from] image::ImageError),
    
    #[error("Driver error: {0}")]
    Driver(#[from] DriverError),
}
```

**Key Rust Improvements:**

1. **Derived Error Traits**: `thiserror` crate auto-generates `Display` and `Error` impls
2. **Error Conversion**: `#[from]` attribute enables automatic error conversion via `?` operator
3. **Pattern Matching**: Exhaustive match on error types at compile-time
4. **No String Allocations**: Error messages generated on-demand
5. **Stack-Based**: Errors are stack-allocated, not heap (unlike C++ std::string)

**Migration Complexity:** Low
- **LOC**: ~80 lines (with thiserror)
- **Time**: 1 day
- **Challenges**: Choosing appropriate error granularity

---

### 3. Driver Interface & EPD27 Implementation

**Current C++ Implementation:**
```cpp
// driver.hpp (114 LOC)
class Driver {
  virtual auto init(DisplayMode) -> expected<void, Error> = 0;
  virtual auto display(span<const byte>) -> expected<void, Error> = 0;
  // ...8 more virtual methods
};

// epd27.cpp (470 LOC)
class EPD27 : public Driver {
  Device& device_;
  DisplayMode current_mode_;
  bool initialized_;
  bool is_asleep_;
  // Large LUT arrays as static constexpr
};
```

**Idiomatic Rust Design:**

```rust
// driver.rs
use std::time::Duration;

/// Driver trait (replaces virtual interface)
pub trait Driver {
    /// Initialize display with specified mode
    fn init(&mut self, mode: DisplayMode) -> Result<(), DriverError>;
    
    /// Display buffer on screen
    fn display(&mut self, buffer: &[u8]) -> Result<(), DriverError>;
    
    /// Get display dimensions
    fn dimensions(&self) -> (usize, usize);
    
    /// Get current mode
    fn mode(&self) -> DisplayMode;
    
    /// Put display to sleep
    fn sleep(&mut self);
    
    /// Wake display from sleep
    fn wake(&mut self) -> Result<(), DriverError>;
    
    /// Check capabilities
    fn capabilities(&self) -> DriverCapabilities;
}

// epd27.rs
pub struct Epd27<'a> {
    device: &'a mut Device,
    mode: DisplayMode,
    state: DisplayState,
}

impl<'a> Epd27<'a> {
    /// Create new EPD27 driver with borrowed device
    pub fn new(device: &'a mut Device) -> Self {
        Self {
            device,
            mode: DisplayMode::BlackWhite,
            state: DisplayState::Uninitialized,
        }
    }
    
    /// Hardware reset
    fn reset(&mut self) {
        self.device.write_pin(pins::RST, true);
        std::thread::sleep(Duration::from_millis(200));
        self.device.write_pin(pins::RST, false);
        std::thread::sleep(Duration::from_millis(2));
        self.device.write_pin(pins::RST, true);
        std::thread::sleep(Duration::from_millis(200));
    }
    
    /// Send command to display
    fn send_command(&mut self, cmd: Command) {
        self.device.write_pin(pins::DC, false);
        self.device.write_pin(pins::CS, false);
        self.device.spi_transfer(cmd as u8);
        self.device.write_pin(pins::CS, true);
    }
}

impl<'a> Driver for Epd27<'a> {
    fn init(&mut self, mode: DisplayMode) -> Result<(), DriverError> {
        self.reset();
        
        match mode {
            DisplayMode::BlackWhite => self.init_bw()?,
            DisplayMode::Grayscale4 => self.init_grayscale()?,
        }
        
        self.state = DisplayState::Awake;
        Ok(())
    }
    
    fn display(&mut self, buffer: &[u8]) -> Result<(), DriverError> {
        // Auto-wake if asleep (transparent state management)
        if matches!(self.state, DisplayState::Asleep) {
            self.wake()?;
        }
        
        // Display logic...
        Ok(())
    }
    
    // ... other trait methods
}

/// State machine for display lifecycle
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum DisplayState {
    Uninitialized,
    Awake,
    Asleep,
}
```

**Key Rust Improvements:**

1. **Lifetime Parameters**: `Epd27<'a>` ensures driver can't outlive device (compile-time safety)
2. **Borrowing Instead of References**: `&mut Device` is safer than C++ `Device&`
3. **Enum State Machine**: `DisplayState` enum is more explicit than bool flags
4. **Pattern Matching**: `match mode` is safer than if-else chains
5. **Trait Objects**: Can use `Box<dyn Driver>` for runtime polymorphism (like C++ virtual)
6. **Generic Dispatch**: Can use generics `<D: Driver>` for zero-cost compile-time dispatch
7. **const Arrays**: LUT tables can be `const` or `static` without allocation

**Migration Complexity:** Medium-High
- **LOC**: ~400 lines
- **Time**: 4-5 days (includes LUT table conversion, timing validation)
- **Challenges**: Hardware timing verification, SPI protocol correctness

---

### 4. Display API & Framebuffer

**Current C++ Implementation:**
```cpp
// display.cpp (711 LOC)
class Display {
  unique_ptr<Driver> driver_;
  vector<byte> buffer_;
  size_t width_, height_;
  DisplayMode mode_;
  Orientation orientation_;
  bool auto_sleep_enabled_;
  
  // Drawing methods: set_pixel, draw_line, draw_circle, etc.
};
```

**Idiomatic Rust Design:**

```rust
// display.rs
pub struct Display<D: Driver> {
    driver: D,
    framebuffer: Framebuffer,
    orientation: Orientation,
    auto_sleep: bool,
}

impl<D: Driver> Display<D> {
    /// Create display with generic driver (zero-cost abstraction)
    pub fn new(driver: D, orientation: Orientation, auto_sleep: bool) -> Self {
        let (width, height) = driver.dimensions();
        let mode = driver.mode();
        
        Self {
            driver,
            framebuffer: Framebuffer::new(width, height, mode),
            orientation,
            auto_sleep,
        }
    }
    
    /// Set pixel with bounds checking
    pub fn set_pixel(&mut self, x: usize, y: usize, color: Color) {
        // Transform coordinates based on orientation
        let (phys_x, phys_y) = self.transform_coordinates(x, y);
        self.framebuffer.set_pixel(phys_x, phys_y, color);
    }
    
    /// Draw line using Bresenham algorithm
    pub fn draw_line(&mut self, start: Point, end: Point, color: Color, width: DotPixel) {
        // Use iterator-based approach for idiomatic Rust
        LineIterator::new(start, end)
            .for_each(|point| self.draw_point(point, color, width));
    }
    
    /// Refresh display
    pub fn refresh(&mut self) -> Result<(), DisplayError> {
        self.driver.display(self.framebuffer.as_bytes())?;
        
        if self.auto_sleep {
            self.driver.sleep();
        }
        
        Ok(())
    }
}

// Framebuffer as separate struct for clarity
pub struct Framebuffer {
    buffer: Vec<u8>,
    width: usize,
    height: usize,
    mode: DisplayMode,
}

impl Framebuffer {
    pub fn new(width: usize, height: usize, mode: DisplayMode) -> Self {
        let size = match mode {
            DisplayMode::BlackWhite => (width * height + 7) / 8,
            DisplayMode::Grayscale4 => (width * height + 3) / 4,
        };
        
        Self {
            buffer: vec![0xFF; size], // Fill white
            width,
            height,
            mode,
        }
    }
    
    pub fn as_bytes(&self) -> &[u8] {
        &self.buffer
    }
    
    /// Clear to color
    pub fn clear(&mut self, color: Color) {
        let fill_byte = self.color_to_fill_byte(color);
        self.buffer.fill(fill_byte);
    }
}

/// Iterator for line drawing (idiomatic Rust)
struct LineIterator {
    current: Point,
    end: Point,
    dx: i32,
    dy: i32,
    error: i32,
    finished: bool,
}

impl Iterator for LineIterator {
    type Item = Point;
    
    fn next(&mut self) -> Option<Self::Item> {
        if self.finished {
            return None;
        }
        
        let current = self.current;
        
        // Bresenham logic...
        
        Some(current)
    }
}
```

**Key Rust Improvements:**

1. **Generic Driver**: `Display<D: Driver>` enables compile-time dispatch (faster than C++ virtual)
2. **Separation**: `Framebuffer` is separate type with clear ownership
3. **Iterators**: `LineIterator` is more idiomatic than loops
4. **Borrowing**: Methods take `&self` or `&mut self`, never need `const` keyword
5. **Builder Pattern**: Can add `DisplayBuilder` for fluent construction
6. **Const Generics**: Could use `const W: usize, const H: usize` for compile-time sizing
7. **No Allocations**: Can use `SmallVec` or stack arrays for small buffers

**Migration Complexity:** Medium-High
- **LOC**: ~600 lines
- **Time**: 5-6 days (includes drawing algorithm validation)
- **Challenges**: Coordinate transformation correctness, pixel encoding

---

### 5. Font Rendering

**Current C++ Implementation:**
```cpp
// font.cpp (47 LOC)
class Font {
  const uint8_t* table_;
  uint16_t width_, height_;
  
  static auto font8() -> const Font&;  // Static singletons
  auto char_data(char c) const -> span<const uint8_t>;
};

// fonts/*.c (raw arrays)
extern "C" {
  #include "../fonts/fonts.h"
}
```

**Idiomatic Rust Design:**

```rust
// font.rs
#[derive(Debug, Clone, Copy)]
pub struct Font {
    table: &'static [u8],
    width: u16,
    height: u16,
}

impl Font {
    /// Create font from static data
    pub const fn new(table: &'static [u8], width: u16, height: u16) -> Self {
        Self { table, width, height }
    }
    
    /// Get character bitmap data
    pub fn char_data(&self, c: char) -> Option<&[u8]> {
        if !c.is_ascii() || c < ' ' {
            return None;
        }
        
        let offset = (c as usize - 0x20) * self.bytes_per_char();
        self.table.get(offset..offset + self.bytes_per_char())
    }
    
    pub const fn width(&self) -> u16 { self.width }
    pub const fn height(&self) -> u16 { self.height }
    
    const fn bytes_per_char(&self) -> usize {
        let width_bytes = (self.width as usize + 7) / 8;
        width_bytes * self.height as usize
    }
}

// Constants instead of functions (zero runtime cost)
pub const FONT_8: Font = Font::new(include_bytes!("../fonts/font8.bin"), 5, 8);
pub const FONT_12: Font = Font::new(include_bytes!("../fonts/font12.bin"), 7, 12);
pub const FONT_16: Font = Font::new(include_bytes!("../fonts/font16.bin"), 11, 16);
pub const FONT_20: Font = Font::new(include_bytes!("../fonts/font20.bin"), 14, 20);
pub const FONT_24: Font = Font::new(include_bytes!("../fonts/font24.bin"), 17, 24);
```

**Key Rust Improvements:**

1. **Static Data**: `&'static [u8]` ensures fonts live forever, no allocation
2. **include_bytes!**: Embed font data at compile-time (no runtime file I/O)
3. **const fn**: `Font::new` is evaluated at compile-time
4. **No C FFI**: Direct Rust arrays, no `extern "C"` needed
5. **Option**: Returns `Option<&[u8]>` for invalid characters (safer than span)
6. **Copy Type**: Font is `Copy`, can be passed by value efficiently

**Migration Complexity:** Low
- **LOC**: ~60 lines
- **Time**: 1 day (includes converting C arrays to Rust format)
- **Challenges**: Font data conversion from C to Rust arrays

---

### 6. Color & Types

**Current C++ Implementation:**
```cpp
// display.hpp
enum class Color : uint8_t { White = 0xFF, Black = 0x00, Gray1 = 0x80, Gray2 = 0x40 };
enum class Orientation : uint8_t { Portrait0, Landscape90, Portrait180, Landscape270 };
enum class DotPixel : uint8_t { Pixel1x1 = 1, ...Pixel8x8 = 8 };
```

**Idiomatic Rust Design:**

```rust
// types.rs
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u8)]
pub enum Color {
    White = 0xFF,
    Gray1 = 0x80,
    Gray2 = 0x40,
    Black = 0x00,
}

impl Color {
    /// Convert RGB to color (using standard luminance formula)
    pub fn from_rgb(r: u8, g: u8, b: u8) -> Self {
        let gray = (0.299 * r as f32 + 0.587 * g as f32 + 0.114 * b as f32) as u8;
        
        match gray {
            192..=255 => Color::White,
            128..=191 => Color::Gray1,
            64..=127 => Color::Gray2,
            0..=63 => Color::Black,
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Orientation {
    Portrait0,
    Landscape90,
    Portrait180,
    Landscape270,
}

impl Orientation {
    /// Rotate orientation clockwise
    pub fn rotate_cw(self) -> Self {
        match self {
            Self::Portrait0 => Self::Landscape90,
            Self::Landscape90 => Self::Portrait180,
            Self::Portrait180 => Self::Landscape270,
            Self::Landscape270 => Self::Portrait0,
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub struct DotPixel(u8);

impl DotPixel {
    pub const PIXEL_1X1: Self = Self(1);
    pub const PIXEL_2X2: Self = Self(2);
    // ...
    pub const PIXEL_8X8: Self = Self(8);
    
    /// Create with validation
    pub const fn new(size: u8) -> Option<Self> {
        if size >= 1 && size <= 8 {
            Some(Self(size))
        } else {
            None
        }
    }
}

/// Point type for coordinates
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Point {
    pub x: usize,
    pub y: usize,
}

impl Point {
    pub const fn new(x: usize, y: usize) -> Self {
        Self { x, y }
    }
}

/// Rectangle for bounds
#[derive(Debug, Clone, Copy)]
pub struct Rect {
    pub origin: Point,
    pub width: usize,
    pub height: usize,
}
```

**Key Rust Improvements:**

1. **Derive Macros**: Auto-implement common traits (Debug, Clone, Copy, PartialEq)
2. **Pattern Matching**: Exhaustive match on enums (compiler error if cases missed)
3. **Const Functions**: Can create values at compile-time
4. **Newtype Pattern**: `DotPixel(u8)` prevents raw u8 usage (type safety)
5. **Rich Types**: `Point` and `Rect` as first-class types
6. **From/Into Traits**: Can implement conversions between types

**Migration Complexity:** Low
- **LOC**: ~100 lines
- **Time**: 1 day
- **Challenges**: None

---

## Architectural Changes for Rust

### 1. Replace Virtual Polymorphism with Traits

**C++ Approach:**
```cpp
unique_ptr<Driver> driver = make_unique<EPD27>(device);
Display display(move(driver));
```

**Rust Approach Option A (Trait Objects - Dynamic Dispatch):**
```rust
let driver: Box<dyn Driver> = Box::new(Epd27::new(&mut device));
let display = Display::new(driver, orientation, auto_sleep);
```

**Rust Approach Option B (Generics - Static Dispatch, Preferred):**
```rust
let driver = Epd27::new(&mut device);
let display = Display::new(driver, orientation, auto_sleep);
// Display<Epd27> is a concrete type at compile-time
```

**Recommendation:** Use generics (`Display<D: Driver>`) for zero-cost abstraction. Only use trait objects if runtime polymorphism is truly needed.

### 2. Eliminate PImpl Pattern

C++ PImpl hides implementation details and enables move semantics. Rust achieves this through:

1. **Module Privacy**: Private fields/methods without heap allocation
2. **Ownership**: Move semantics built into language
3. **Opaque Types**: Can return `impl Trait` to hide concrete types

**Before (C++):**
```cpp
class Device {
  unique_ptr<DeviceImpl> pimpl_;  // Heap allocation
};
```

**After (Rust):**
```rust
pub struct Device {
    // Private fields - implementation details hidden
    chip: gpiod::Chip,
    lines: gpiod::Lines,
}
```

### 3. Lifetime-Based Resource Management

**C++ Approach:**
```cpp
Device device;
EPD27 driver(device);  // Reference to device
// Must ensure device outlives driver manually
```

**Rust Approach:**
```rust
let mut device = Device::init()?;
let driver = Epd27::new(&mut device);
// Compiler enforces that device outlives driver at compile-time
```

Rust's borrow checker prevents use-after-free bugs that C++ can't catch.

### 4. Builder Pattern for Configuration

**Current C++:**
```cpp
auto display = create_display<EPD27>(device, mode, orientation, auto_sleep);
```

**Idiomatic Rust:**
```rust
let display = DisplayBuilder::new(device)
    .mode(DisplayMode::Grayscale4)
    .orientation(Orientation::Landscape90)
    .auto_sleep(true)
    .build()?;
```

### 5. Error Handling with `?` Operator

**C++ (explicit checks):**
```cpp
auto result = display->refresh();
if (!result) {
    std::cerr << result.error().what() << "\n";
    return;
}
```

**Rust (concise propagation):**
```rust
display.refresh()?;  // Automatically propagate error
```

---

## Crate Structure & Dependencies

### Recommended Crate Layout

```
epaper/
├── Cargo.toml
├── src/
│   ├── lib.rs              # Public API exports
│   ├── device.rs           # GPIO/SPI device layer
│   ├── driver/
│   │   ├── mod.rs          # Driver trait
│   │   └── epd27.rs        # EPD27 implementation
│   ├── display.rs          # High-level display API
│   ├── framebuffer.rs      # Pixel buffer management
│   ├── font.rs             # Font rendering
│   ├── drawing.rs          # Drawing primitives
│   ├── error.rs            # Error types
│   ├── types.rs            # Color, Orientation, etc.
│   └── prelude.rs          # Common imports
├── fonts/
│   ├── font8.bin
│   ├── font12.bin
│   └── ...
├── examples/
│   ├── hello_world.rs
│   ├── crypto_dashboard.rs
│   └── ...
└── tests/
    ├── integration_tests.rs
    └── ...
```

### Dependencies (Cargo.toml)

```toml
[package]
name = "epaper"
version = "0.1.0"
edition = "2021"
rust-version = "1.75"  # For latest features

[dependencies]
# Error handling
thiserror = "1.0"
anyhow = "1.0"  # For applications using the library

# Hardware interface
gpiod = "0.3"  # libgpiod bindings
spidev = "0.5"  # SPI device interface

# Image loading
image = "0.24"  # Supports PNG, JPEG, BMP, etc.

# Utilities
bitflags = "2.4"  # For capability flags
const_format = "0.2"  # Const string formatting

[dev-dependencies]
# Testing
criterion = "0.5"  # Benchmarking
proptest = "1.4"  # Property-based testing
mockall = "0.12"  # Mocking for tests

[features]
default = ["image-support"]
image-support = ["image"]
hardware-tests = []  # Enable tests requiring actual hardware
```

### Key Crate Choices

1. **gpiod** (0.3): Safe Rust bindings to libgpiod (same underlying library as C++)
2. **spidev** (0.5): Linux SPI device interface
3. **image** (0.24): Image decoding (replaces stb_image)
4. **thiserror** (1.0): Ergonomic error handling
5. **criterion** (0.5): Statistical benchmarking

---

## Effort Estimation

### Phase 1: Core Infrastructure (1 week)

| Component | LOC | Complexity | Time |
|-----------|-----|------------|------|
| Error types | 80 | Low | 1 day |
| Types & enums | 100 | Low | 1 day |
| Device layer | 250 | Medium | 2-3 days |
| Font system | 60 | Low | 1 day |

**Total:** ~490 LOC, 5 days

### Phase 2: Driver Implementation (1 week)

| Component | LOC | Complexity | Time |
|-----------|-----|------------|------|
| Driver trait | 50 | Low | 0.5 day |
| EPD27 init | 200 | Medium-High | 2 days |
| EPD27 display | 150 | High | 2 days |
| LUT tables | 100 | Low | 0.5 day |
| State management | 50 | Medium | 1 day |

**Total:** ~550 LOC, 6 days

### Phase 3: Display API (1.5 weeks)

| Component | LOC | Complexity | Time |
|-----------|-----|------------|------|
| Framebuffer | 200 | Medium | 2 days |
| Coordinate transform | 80 | Medium | 1 day |
| Drawing primitives | 300 | Medium | 3 days |
| Text rendering | 150 | Medium | 2 days |
| Image loading | 100 | Low | 1 day |

**Total:** ~830 LOC, 9 days

### Phase 4: Testing & Examples (1 week)

| Component | LOC | Complexity | Time |
|-----------|-----|------------|------|
| Unit tests | 400 | Medium | 2 days |
| Integration tests | 300 | High | 2 days |
| Examples | 200 | Low | 1 day |
| Documentation | - | Low | 2 days |

**Total:** ~900 LOC, 7 days

### Phase 5: Optimization & Polish (0.5 weeks)

| Task | Time |
|------|------|
| Performance tuning | 1 day |
| API refinement | 1 day |
| CI/CD setup | 0.5 day |

**Total:** 2.5 days

---

### Grand Total

- **Total LOC**: ~2,770 lines (includes tests)
- **Core Library**: ~2,000 LOC
- **Time Estimate**: 4-6 weeks for experienced Rust developer
- **Time Estimate**: 6-10 weeks for Rust beginner

---

## Risks & Challenges

### High Risk Items

1. **Hardware Timing Correctness**
   - **Risk**: Subtle timing bugs in SPI/GPIO operations
   - **Mitigation**: Extensive hardware testing, logic analyzer verification
   - **Impact**: Could cause display corruption or failure

2. **FFI Safety with gpiod**
   - **Risk**: Unsafe code required for C library interaction
   - **Mitigation**: Use established `gpiod` crate, minimize unsafe blocks
   - **Impact**: Memory safety issues if mishandled

3. **Pixel Encoding Bugs**
   - **Risk**: Off-by-one errors in bit manipulation
   - **Mitigation**: Comprehensive unit tests, visual verification
   - **Impact**: Display artifacts, wrong colors

### Medium Risk Items

4. **Performance Regression**
   - **Risk**: Rust version slower than C++
   - **Mitigation**: Benchmarking, profiling, optimization
   - **Impact**: Longer refresh times

5. **API Usability**
   - **Risk**: Rust API less ergonomic than C++
   - **Mitigation**: API review, examples, documentation
   - **Impact**: Poor developer experience

6. **Dependency Management**
   - **Risk**: Breaking changes in gpiod/spidev crates
   - **Mitigation**: Pin versions, consider vendoring
   - **Impact**: Build failures, maintenance burden

### Low Risk Items

7. **Toolchain Setup**
   - **Risk**: Difficulty setting up Rust toolchain on Pi
   - **Mitigation**: Provide setup guide, Docker container
   - **Impact**: Developer friction

8. **Learning Curve**
   - **Risk**: Team unfamiliar with Rust
   - **Mitigation**: Training, pair programming, gradual rollout
   - **Impact**: Slower development

---

## Migration Strategy

### Recommended Approach: Incremental Migration

**Option A: Parallel Implementation (Recommended)**

1. Keep C++ library functional
2. Implement Rust library alongside
3. Run both in parallel during development
4. Gradually port examples to Rust
5. Benchmark and validate correctness
6. Deprecate C++ version once stable

**Benefits:**
- Low risk (C++ fallback)
- Can compare behavior side-by-side
- Gradual team adoption

**Option B: Direct Replacement**

1. Freeze C++ development
2. Implement Rust library
3. Port all examples at once
4. Replace C++ entirely

**Benefits:**
- Faster completion
- No maintenance of two codebases

**Drawbacks:**
- High risk if issues found
- No fallback option

### Implementation Order

1. **Week 1-2: Foundation**
   - Types, errors, device layer
   - Basic hardware testing
   - Validate GPIO/SPI functionality

2. **Week 3-4: Driver**
   - EPD27 driver implementation
   - Hardware state management
   - Timing verification

3. **Week 5-6: Display API**
   - Framebuffer and drawing
   - Font rendering
   - Image loading

4. **Week 7: Testing & Examples**
   - Comprehensive test suite
   - Port crypto_dashboard example
   - Documentation

5. **Week 8: Polish (optional)**
   - Performance optimization
   - API refinement
   - Community feedback

---

## Benefits of Rust Migration

### Safety Improvements

1. **Memory Safety**: No use-after-free, double-free, or buffer overflows
2. **Thread Safety**: `Send`/`Sync` traits prevent data races
3. **Lifetime Tracking**: Compiler prevents dangling references
4. **Null Safety**: No null pointer dereferences (`Option<T>` instead)

### Performance Improvements

1. **Zero-Cost Abstractions**: Generics compiled away, no virtual dispatch overhead
2. **LLVM Optimization**: Modern compiler backend
3. **No Hidden Allocations**: Explicit `Box`/`Vec`, easier to profile
4. **Inline Hints**: Better control over inlining

### Developer Experience

1. **Cargo**: Modern build system, dependency management, testing
2. **rustfmt**: Automatic code formatting
3. **clippy**: Linting and best practices
4. **docs.rs**: Automatic documentation generation
5. **crates.io**: Package registry

### Maintainability

1. **Strong Type System**: Catch bugs at compile-time
2. **Pattern Matching**: Exhaustive case analysis
3. **Explicit Error Handling**: `Result<T, E>` forces error consideration
4. **Module System**: Clear boundaries, no header/impl split

---

## Comparison: C++ vs Rust

| Aspect | C++ | Rust |
|--------|-----|------|
| **LOC** | ~1,400 src + ~1,800 headers = 3,200 | ~2,500 (no header duplication) |
| **Memory Safety** | Manual (RAII, smart pointers) | Automatic (borrow checker) |
| **Error Handling** | `std::expected` (C++23) | `Result<T, E>` (stable since 1.0) |
| **Build System** | CMake (verbose) | Cargo (ergonomic) |
| **Dependencies** | Manual (git submodules) | Cargo.toml (automatic) |
| **Testing** | Catch2 (separate setup) | Built-in (`cargo test`) |
| **Documentation** | Doxygen (separate tool) | Rustdoc (built-in) |
| **Polymorphism** | Virtual functions (runtime) | Traits (compile-time or runtime) |
| **Null Safety** | None (raw pointers) | `Option<T>` |
| **Thread Safety** | Manual (locks, atomics) | Enforced (`Send`/`Sync`) |
| **Compilation Speed** | Fast (incremental) | Medium (slower but improving) |
| **Runtime Performance** | Excellent | Excellent (comparable) |

---

## Conclusion

Migrating from C++ to **idiomatic Rust** requires approximately **4-6 weeks** of focused effort. The key architectural changes involve:

1. **Traits instead of virtual inheritance**
2. **Lifetime annotations for safe borrowing**
3. **Ownership system replacing manual resource management**
4. **Generic dispatch for zero-cost abstractions**
5. **Rich type system with newtypes and pattern matching**

The migration offers significant benefits:
- **Compile-time safety guarantees** (memory, thread, lifetime)
- **Cleaner API** (no PImpl, no header/impl split)
- **Better tooling** (Cargo, rustfmt, clippy, docs)
- **Maintainability** (strong types, exhaustive matching)

The main challenges are:
- **Hardware timing validation** (critical for display correctness)
- **Team learning curve** (if new to Rust)
- **FFI safety** (unsafe code for C library interaction)

**Recommendation:** Proceed with migration using **parallel implementation** strategy. This minimizes risk while allowing gradual team adoption and thorough validation against the existing C++ implementation.

---

## Next Steps

If proceeding with migration:

1. **Set up Rust toolchain** on development Raspberry Pi
2. **Create basic project structure** with Cargo
3. **Implement device layer** and validate GPIO/SPI
4. **Port one driver** (EPD27) completely
5. **Implement display API** incrementally
6. **Port crypto_dashboard example** as validation
7. **Benchmark and optimize**
8. **Document and publish** to crates.io

For questions or discussion, please reach out to the team.
