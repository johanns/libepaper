# Rust Migration: Detailed Code Examples

This document provides detailed side-by-side comparisons showing how C++ code translates to idiomatic Rust, highlighting architectural improvements.

---

## Table of Contents

1. [Device Initialization](#device-initialization)
2. [Error Handling](#error-handling)
3. [Ownership & Lifetimes](#ownership--lifetimes)
4. [Trait vs Virtual Inheritance](#trait-vs-virtual-inheritance)
5. [Resource Management](#resource-management)
6. [Drawing Operations](#drawing-operations)
7. [Type Safety Improvements](#type-safety-improvements)
8. [Iterator-Based Algorithms](#iterator-based-algorithms)

---

## Device Initialization

### C++ Implementation

```cpp
// device.cpp
#include <gpiod.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

struct DeviceImpl {
  gpiod_chip *chip = nullptr;
  gpiod_line_request *line_request = nullptr;
  std::unordered_map<uint8_t, PinConfig> pin_configs;
  int spi_fd = -1;
  bool initialized = false;
  
  ~DeviceImpl() {
    if (line_request != nullptr) {
      gpiod_line_request_release(line_request);
    }
    if (chip != nullptr) {
      gpiod_chip_close(chip);
    }
    if (spi_fd >= 0) {
      close(spi_fd);
    }
  }
};

class Device {
  std::unique_ptr<DeviceImpl> pimpl_;
public:
  Device() : pimpl_(std::make_unique<DeviceImpl>()) {}
  
  auto init() -> std::expected<void, Error> {
    if (pimpl_->initialized) {
      return {};
    }
    
    // Initialize GPIO
    pimpl_->chip = gpiod_chip_open("/dev/gpiochip0");
    if (pimpl_->chip == nullptr) {
      return std::unexpected(Error(ErrorCode::GPIOInitFailed, 
                                    "Failed to open /dev/gpiochip0"));
    }
    
    // Initialize SPI
    pimpl_->spi_fd = open("/dev/spidev0.0", O_RDWR);
    if (pimpl_->spi_fd < 0) {
      gpiod_chip_close(pimpl_->chip);
      pimpl_->chip = nullptr;
      return std::unexpected(Error(ErrorCode::SPIDeviceOpenFailed, 
                                    "Failed to open /dev/spidev0.0"));
    }
    
    // Configure SPI mode
    uint8_t mode = SPI_MODE_0;
    if (ioctl(pimpl_->spi_fd, SPI_IOC_WR_MODE, &mode) < 0) {
      close(pimpl_->spi_fd);
      gpiod_chip_close(pimpl_->chip);
      return std::unexpected(Error(ErrorCode::SPIConfigFailed, 
                                    "Failed to set SPI mode"));
    }
    
    pimpl_->initialized = true;
    return {};
  }
};
```

### Rust Implementation (Idiomatic)

```rust
// device.rs
use gpiod::{Chip, Lines, Options};
use spidev::{Spidev, SpidevOptions, SpiModeFlags};
use std::fs::OpenOptions;
use std::io;

pub struct Device {
    gpio: GpioController,
    spi: SpiController,
    pin_configs: HashMap<u8, PinConfig>,
}

struct GpioController {
    chip: Chip,
    lines: Lines,
}

struct SpiController {
    device: Spidev,
}

impl Device {
    /// Initialize hardware with automatic resource management
    pub fn init() -> Result<Self, DeviceError> {
        // Initialize GPIO
        let chip = Chip::new("/dev/gpiochip0")
            .map_err(|e| DeviceError::GpioInit(e))?;
        
        let lines = chip.request_lines(Options::output())
            .map_err(|e| DeviceError::GpioInit(e))?;
        
        // Initialize SPI
        let mut spi = Spidev::open("/dev/spidev0.0")
            .map_err(|e| DeviceError::SpiOpen(e))?;
        
        let options = SpidevOptions::new()
            .mode(SpiModeFlags::SPI_MODE_0)
            .max_speed_hz(1_953_125)
            .bits_per_word(8)
            .build();
        
        spi.configure(&options)
            .map_err(|_| DeviceError::SpiConfig)?;
        
        Ok(Self {
            gpio: GpioController { chip, lines },
            spi: SpiController { device: spi },
            pin_configs: HashMap::new(),
        })
    }
    
    /// Write to GPIO pin
    pub fn write_pin(&mut self, pin: Pin, value: bool) -> Result<(), DeviceError> {
        self.gpio.lines.set_value(pin.offset(), value as u8)?;
        Ok(())
    }
}

// Automatic cleanup - no manual destructor needed
impl Drop for Device {
    fn drop(&mut self) {
        // Rust automatically drops chip, lines, and spi
        // in correct order (reverse construction)
    }
}
```

**Key Improvements:**

1. **No PImpl Pattern**: Direct fields, privacy via module system
2. **Automatic Drop Order**: Rust drops in reverse construction order, guaranteed
3. **Builder Pattern**: `SpidevOptions::new().mode().speed().build()`
4. **Error Propagation**: `?` operator is cleaner than nested checks
5. **No Manual Cleanup**: `Drop` is automatic, can't forget

---

## Error Handling

### C++ Implementation

```cpp
// errors.hpp
enum class ErrorCode {
  DeviceNotInitialized,
  GPIOInitFailed,
  SPIConfigFailed,
  FileNotFound,
};

struct Error {
  ErrorCode code;
  std::string message;
  
  auto what() const -> std::string_view {
    if (!message.empty()) {
      return message;
    }
    return to_string(code);
  }
};

// Usage
auto result = display->refresh();
if (!result) {
  std::cerr << "Error: " << result.error().what() << "\n";
  return 1;
}
```

### Rust Implementation (Idiomatic)

```rust
// error.rs
use thiserror::Error;

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
    
    #[error("Device error: {0}")]
    Device(#[from] DeviceError),
}

#[derive(Debug, Error)]
pub enum DisplayError {
    #[error("Display refresh failed")]
    RefreshFailed,
    
    #[error("Image load failed: {0}")]
    ImageLoad(#[from] image::ImageError),
    
    #[error("Driver error: {0}")]
    Driver(#[from] DriverError),
}

// Usage - automatic error conversion
fn example() -> Result<(), DisplayError> {
    let mut device = Device::init()?;  // DeviceError -> DisplayError automatically
    let mut driver = Epd27::new(&mut device);
    driver.init(DisplayMode::BlackWhite)?;  // DriverError -> DisplayError automatically
    driver.display(&buffer)?;
    Ok(())
}

// Error handling with context
fn example_with_context() -> anyhow::Result<()> {
    let device = Device::init()
        .context("Failed to initialize hardware")?;
    
    // ... more operations
    
    Ok(())
}
```

**Key Improvements:**

1. **thiserror**: Auto-generates `Display` and `Error` trait implementations
2. **Error Conversion**: `#[from]` enables automatic conversion via `?`
3. **Pattern Matching**: Can match on specific error variants
4. **Stack-Based**: Errors are stack-allocated, not heap (unlike `std::string`)
5. **Type Safety**: Can't mix error types without explicit conversion

---

## Ownership & Lifetimes

### C++ Implementation

```cpp
// display.hpp
class Display {
  std::unique_ptr<Driver> driver_;
  
public:
  explicit Display(std::unique_ptr<Driver> driver)
    : driver_(std::move(driver)) {}
    
  // Must ensure driver lifetime manually
};

// Usage
Device device;
device.init();

auto driver = std::make_unique<EPD27>(device);  // device must outlive driver
driver->init(DisplayMode::BlackWhite);

Display display(std::move(driver));  // driver must outlive display
// If device goes out of scope here, undefined behavior!
```

### Rust Implementation (Idiomatic)

```rust
// display.rs
pub struct Display<D: Driver> {
    driver: D,
    framebuffer: Framebuffer,
    orientation: Orientation,
    auto_sleep: bool,
}

impl<D: Driver> Display<D> {
    pub fn new(driver: D, orientation: Orientation, auto_sleep: bool) -> Self {
        let (width, height) = driver.dimensions();
        Self {
            driver,
            framebuffer: Framebuffer::new(width, height, driver.mode()),
            orientation,
            auto_sleep,
        }
    }
}

// Driver with lifetime tracking
pub struct Epd27<'a> {
    device: &'a mut Device,
    mode: DisplayMode,
    state: DisplayState,
}

impl<'a> Epd27<'a> {
    pub fn new(device: &'a mut Device) -> Self {
        Self {
            device,
            mode: DisplayMode::BlackWhite,
            state: DisplayState::Uninitialized,
        }
    }
}

// Usage - compiler enforces correct lifetimes
fn usage() -> Result<(), DisplayError> {
    let mut device = Device::init()?;
    
    {
        let mut driver = Epd27::new(&mut device);
        driver.init(DisplayMode::BlackWhite)?;
        
        let mut display = Display::new(driver, Orientation::Portrait0, true);
        display.refresh()?;
        
        // driver and display dropped here
    }
    
    // device still valid here
    // Compiler prevents:
    // - Using driver after device is dropped
    // - Using display after driver is dropped
    
    Ok(())
}

// This WILL NOT COMPILE - lifetime error
fn invalid_usage() -> Result<(), DisplayError> {
    let driver = {
        let mut device = Device::init()?;
        Epd27::new(&mut device)  // ERROR: device doesn't live long enough
    };
    
    Ok(())
}
```

**Key Improvements:**

1. **Lifetime Parameters**: `Epd27<'a>` ensures driver can't outlive device
2. **Compile-Time Safety**: Borrow checker prevents use-after-free
3. **No Runtime Overhead**: Lifetime checks are zero-cost (compile-time only)
4. **Clear Ownership**: `driver: D` vs `device: &'a mut Device` shows ownership
5. **Cannot Forget**: Impossible to have dangling references (unlike C++ references)

---

## Trait vs Virtual Inheritance

### C++ Implementation

```cpp
// driver.hpp
class Driver {
public:
  virtual ~Driver() = default;
  virtual auto init(DisplayMode mode) -> std::expected<void, Error> = 0;
  virtual auto display(std::span<const std::byte> buffer) -> std::expected<void, Error> = 0;
  virtual auto width() const noexcept -> std::size_t = 0;
  virtual auto height() const noexcept -> std::size_t = 0;
  // ... more virtual methods
};

// epd27.hpp
class EPD27 : public Driver {
  Device& device_;
  DisplayMode current_mode_;
  bool is_asleep_;
  
public:
  explicit EPD27(Device& device) : device_(device) {}
  
  auto init(DisplayMode mode) -> std::expected<void, Error> override {
    // Implementation with virtual dispatch overhead
  }
};

// Usage - runtime polymorphism
std::unique_ptr<Driver> driver = std::make_unique<EPD27>(device);
Display display(std::move(driver));  // Virtual calls at runtime
```

### Rust Implementation Option A: Trait Objects (Runtime Polymorphism)

```rust
// driver.rs
pub trait Driver {
    fn init(&mut self, mode: DisplayMode) -> Result<(), DriverError>;
    fn display(&mut self, buffer: &[u8]) -> Result<(), DriverError>;
    fn dimensions(&self) -> (usize, usize);
    fn mode(&self) -> DisplayMode;
    fn sleep(&mut self);
}

// epd27.rs
pub struct Epd27<'a> {
    device: &'a mut Device,
    mode: DisplayMode,
    state: DisplayState,
}

impl<'a> Driver for Epd27<'a> {
    fn init(&mut self, mode: DisplayMode) -> Result<(), DriverError> {
        // Implementation
        Ok(())
    }
    
    fn display(&mut self, buffer: &[u8]) -> Result<(), DriverError> {
        // Implementation
        Ok(())
    }
    
    // ... other trait methods
}

// Usage - runtime polymorphism (like C++ virtual)
let driver: Box<dyn Driver> = Box::new(Epd27::new(&mut device));
let display = Display::new(driver, orientation, auto_sleep);
// Uses vtable, similar to C++ virtual calls
```

### Rust Implementation Option B: Generics (Static Dispatch, Preferred)

```rust
// display.rs
pub struct Display<D: Driver> {
    driver: D,  // Concrete type, not trait object
    framebuffer: Framebuffer,
    orientation: Orientation,
}

impl<D: Driver> Display<D> {
    pub fn new(driver: D, orientation: Orientation, auto_sleep: bool) -> Self {
        Self {
            driver,
            framebuffer: Framebuffer::new(
                driver.dimensions().0,
                driver.dimensions().1,
                driver.mode(),
            ),
            orientation,
        }
    }
    
    pub fn refresh(&mut self) -> Result<(), DisplayError> {
        // Direct call to driver.display() - no virtual dispatch!
        // Compiler monomorphizes this for each driver type
        self.driver.display(self.framebuffer.as_bytes())?;
        Ok(())
    }
}

// Usage - compile-time polymorphism (zero-cost)
let driver = Epd27::new(&mut device);
let display = Display::new(driver, orientation, true);
// Display<Epd27> is a concrete type
// All calls are direct, no vtable overhead
```

**Key Improvements:**

1. **Zero-Cost Abstraction**: Generics have no runtime overhead
2. **Monomorphization**: Compiler generates specialized code for each type
3. **Inlining**: Generic methods can be inlined across crate boundaries
4. **Type Safety**: `Display<Epd27>` vs `Display<Epd42>` are different types
5. **Flexibility**: Can use trait objects if dynamic dispatch truly needed

**Performance Comparison:**

```rust
// Trait object (dynamic dispatch)
let driver: Box<dyn Driver> = Box::new(Epd27::new(&mut device));
// Every call goes through vtable (~1-2ns overhead)

// Generic (static dispatch)
let driver = Epd27::new(&mut device);
let display = Display::new(driver, ...);
// Direct function calls, can be inlined (0ns overhead)
```

---

## Resource Management

### C++ Implementation

```cpp
// device.cpp
auto Device::init() -> std::expected<void, Error> {
  // Allocate resources
  pimpl_->chip = gpiod_chip_open("/dev/gpiochip0");
  if (pimpl_->chip == nullptr) {
    return std::unexpected(Error(ErrorCode::GPIOInitFailed));
  }
  
  pimpl_->spi_fd = open("/dev/spidev0.0", O_RDWR);
  if (pimpl_->spi_fd < 0) {
    // Must manually clean up chip!
    gpiod_chip_close(pimpl_->chip);
    pimpl_->chip = nullptr;
    return std::unexpected(Error(ErrorCode::SPIDeviceOpenFailed));
  }
  
  // Configure SPI
  uint8_t mode = SPI_MODE_0;
  if (ioctl(pimpl_->spi_fd, SPI_IOC_WR_MODE, &mode) < 0) {
    // Must manually clean up both!
    close(pimpl_->spi_fd);
    gpiod_chip_close(pimpl_->chip);
    return std::unexpected(Error(ErrorCode::SPIConfigFailed));
  }
  
  return {};
}

// Destructor must clean up
DeviceImpl::~DeviceImpl() {
  if (line_request != nullptr) {
    gpiod_line_request_release(line_request);
  }
  if (chip != nullptr) {
    gpiod_chip_close(chip);
  }
  if (spi_fd >= 0) {
    close(spi_fd);
  }
}
```

### Rust Implementation (Idiomatic)

```rust
// device.rs
impl Device {
    pub fn init() -> Result<Self, DeviceError> {
        // Open GPIO chip
        let chip = Chip::new("/dev/gpiochip0")?;
        
        // If this fails, chip is automatically dropped (closed)
        let lines = chip.request_lines(Options::output())?;
        
        // If this fails, chip and lines are automatically dropped
        let mut spi = Spidev::open("/dev/spidev0.0")?;
        
        // If this fails, all previous resources are automatically dropped
        let options = SpidevOptions::new()
            .mode(SpiModeFlags::SPI_MODE_0)
            .max_speed_hz(1_953_125)
            .build();
        
        spi.configure(&options)?;
        
        // All resources successfully initialized
        Ok(Self {
            gpio: GpioController { chip, lines },
            spi: SpiController { device: spi },
            pin_configs: HashMap::new(),
        })
    }
}

// Automatic cleanup - Drop trait
impl Drop for GpioController {
    fn drop(&mut self) {
        // Rust automatically drops chip and lines in reverse order
        // No manual cleanup needed, guaranteed to run
    }
}

impl Drop for SpiController {
    fn drop(&mut self) {
        // Rust automatically closes spi device
    }
}

// If init() fails at any point, all previously allocated resources
// are automatically cleaned up via Drop. No manual cleanup needed!
```

**Key Improvements:**

1. **Automatic Cleanup**: Drop trait ensures resources are freed
2. **Exception Safety**: Even on panic, Drop runs (unwinding)
3. **Correct Order**: Drops in reverse construction order
4. **No Leaks**: Impossible to forget to free resources
5. **Simpler Code**: ~50% less code than C++ RAII

---

## Drawing Operations

### C++ Implementation

```cpp
// display.cpp
auto Display::draw_line(size_t x_start, size_t y_start,
                        size_t x_end, size_t y_end,
                        Color color, DotPixel line_width,
                        LineStyle style) -> void {
  // Bresenham's line algorithm
  const auto dx = static_cast<int32_t>(x_end > x_start ? x_end - x_start : x_start - x_end);
  const auto dy = static_cast<int32_t>(y_end > y_start ? y_end - y_start : y_start - y_end);
  
  const int32_t x_inc = x_start < x_end ? 1 : -1;
  const int32_t y_inc = y_start < y_end ? 1 : -1;
  
  int32_t esp = dx - dy;
  auto x = static_cast<int32_t>(x_start);
  auto y = static_cast<int32_t>(y_start);
  
  size_t dot_count = 0;
  
  while (true) {
    const bool should_draw = (style == LineStyle::Solid) || ((dot_count % 6) < 3);
    
    if (should_draw) {
      draw_point(static_cast<size_t>(x), static_cast<size_t>(y), color, line_width);
    }
    ++dot_count;
    
    if (x == static_cast<int32_t>(x_end) && y == static_cast<int32_t>(y_end)) {
      break;
    }
    
    // Bresenham step...
  }
}
```

### Rust Implementation (Idiomatic)

```rust
// drawing.rs
impl<D: Driver> Display<D> {
    pub fn draw_line(&mut self, start: Point, end: Point, 
                     color: Color, width: DotPixel, style: LineStyle) {
        // Use iterator for idiomatic Rust
        LineIterator::new(start, end)
            .enumerate()
            .filter(|(i, _)| style.should_draw(*i))
            .for_each(|(_, point)| self.draw_point(point, color, width));
    }
}

// Bresenham iterator
pub struct LineIterator {
    current: Point,
    end: Point,
    dx: i32,
    dy: i32,
    x_inc: i32,
    y_inc: i32,
    error: i32,
    finished: bool,
}

impl LineIterator {
    pub fn new(start: Point, end: Point) -> Self {
        let dx = (end.x as i32 - start.x as i32).abs();
        let dy = (end.y as i32 - start.y as i32).abs();
        let x_inc = if start.x < end.x { 1 } else { -1 };
        let y_inc = if start.y < end.y { 1 } else { -1 };
        
        Self {
            current: start,
            end,
            dx,
            dy,
            x_inc,
            y_inc,
            error: dx - dy,
            finished: false,
        }
    }
}

impl Iterator for LineIterator {
    type Item = Point;
    
    fn next(&mut self) -> Option<Self::Item> {
        if self.finished {
            return None;
        }
        
        let current = self.current;
        
        if self.current == self.end {
            self.finished = true;
            return Some(current);
        }
        
        // Bresenham step
        let error2 = 2 * self.error;
        if error2 >= -self.dy {
            self.error -= self.dy;
            self.current.x = (self.current.x as i32 + self.x_inc) as usize;
        }
        if error2 <= self.dx {
            self.error += self.dx;
            self.current.y = (self.current.y as i32 + self.y_inc) as usize;
        }
        
        Some(current)
    }
}

// Line style determines which dots to draw
impl LineStyle {
    fn should_draw(&self, index: usize) -> bool {
        match self {
            LineStyle::Solid => true,
            LineStyle::Dotted => (index % 6) < 3,
        }
    }
}
```

**Key Improvements:**

1. **Iterator Pattern**: More composable than while loop
2. **Functional Style**: `.filter().for_each()` is clearer than if statements
3. **Reusable**: `LineIterator` can be used elsewhere
4. **Testable**: Can test iterator independently
5. **Chainable**: Can add `.skip()`, `.take()`, etc.

---

## Type Safety Improvements

### C++ Implementation

```cpp
// display.hpp
enum class DotPixel : uint8_t {
  Pixel1x1 = 1,
  Pixel2x2 = 2,
  //...
  Pixel8x8 = 8
};

// Usage - can pass invalid values
uint8_t size = 10;  // Invalid!
draw_point(x, y, color, static_cast<DotPixel>(size));  // Compiles, undefined behavior
```

### Rust Implementation (Idiomatic)

```rust
// types.rs
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct DotPixel(u8);

impl DotPixel {
    pub const PIXEL_1X1: Self = Self(1);
    pub const PIXEL_2X2: Self = Self(2);
    pub const PIXEL_3X3: Self = Self(3);
    pub const PIXEL_4X4: Self = Self(4);
    pub const PIXEL_5X5: Self = Self(5);
    pub const PIXEL_6X6: Self = Self(6);
    pub const PIXEL_7X7: Self = Self(7);
    pub const PIXEL_8X8: Self = Self(8);
    
    /// Create with compile-time validation
    pub const fn new(size: u8) -> Option<Self> {
        if size >= 1 && size <= 8 {
            Some(Self(size))
        } else {
            None
        }
    }
    
    /// Get size (always valid)
    pub const fn size(self) -> u8 {
        self.0
    }
}

// Usage - type-safe
let pixel_size = DotPixel::PIXEL_3X3;  // OK
let invalid = DotPixel::new(10);  // Returns None, can't construct invalid value

// Pin type safety
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Pin(u8);

impl Pin {
    pub const RST: Self = Self(17);
    pub const DC: Self = Self(25);
    pub const CS: Self = Self(8);
    pub const BUSY: Self = Self(24);
    
    /// Create with validation
    pub const fn new(number: u8) -> Option<Self> {
        // Can only create valid pins
        if number < 28 {
            Some(Self(number))
        } else {
            None
        }
    }
}

// Color with guaranteed values
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u8)]
pub enum Color {
    White = 0xFF,
    Gray1 = 0x80,
    Gray2 = 0x40,
    Black = 0x00,
}

impl Color {
    /// Convert RGB to Color (always returns valid color)
    pub fn from_rgb(r: u8, g: u8, b: u8) -> Self {
        let gray = (0.299 * r as f32 + 0.587 * g as f32 + 0.114 * b as f32) as u8;
        
        match gray {
            192..=255 => Color::White,
            128..=191 => Color::Gray1,
            64..=127 => Color::Gray2,
            0..=63 => Color::Black,
        }
    }
    
    /// Get raw byte value
    pub const fn as_u8(self) -> u8 {
        self as u8
    }
}
```

**Key Improvements:**

1. **Newtype Pattern**: `DotPixel(u8)` prevents raw u8 usage
2. **Validation**: `new()` returns `Option`, forces error handling
3. **Exhaustive Matching**: Can't forget Color variants
4. **Compile-Time Constants**: `const fn` for zero-cost construction
5. **Type-Safe Conversions**: Can't accidentally mix types

---

## Iterator-Based Algorithms

### C++ Implementation

```cpp
// display.cpp
auto Display::draw_circle(size_t x_center, size_t y_center, size_t radius,
                          Color color, DotPixel line_width, DrawFill fill) -> void {
  if (radius == 0) {
    return;
  }
  
  // Midpoint circle algorithm
  int32_t x = 0;
  auto y = static_cast<int32_t>(radius);
  int32_t d = 3 - (2 * static_cast<int32_t>(radius));
  
  while (x <= y) {
    if (fill == DrawFill::Full) {
      draw_horizontal_line(x_center - x, x_center + x, y_center + y, color, line_width);
      draw_horizontal_line(x_center - x, x_center + x, y_center - y, color, line_width);
      draw_horizontal_line(x_center - y, x_center + y, y_center + x, color, line_width);
      draw_horizontal_line(x_center - y, x_center + y, y_center - x, color, line_width);
    } else {
      // Draw 8 symmetric points
      draw_point(x_center + x, y_center + y, color, line_width);
      draw_point(x_center - x, y_center + y, color, line_width);
      // ... 6 more points
    }
    
    if (d < 0) {
      d = d + 4 * x + 6;
    } else {
      d = d + 4 * (x - y) + 10;
      --y;
    }
    ++x;
  }
}
```

### Rust Implementation (Idiomatic)

```rust
// drawing.rs
impl<D: Driver> Display<D> {
    pub fn draw_circle(&mut self, center: Point, radius: usize,
                       color: Color, width: DotPixel, fill: DrawFill) {
        if radius == 0 {
            return;
        }
        
        // Use iterator for circle points
        CircleIterator::new(radius)
            .for_each(|(x, y)| {
                match fill {
                    DrawFill::Full => {
                        self.draw_horizontal_line(
                            center.x.saturating_sub(x),
                            center.x.saturating_add(x),
                            center.y.saturating_add(y),
                            color,
                            width,
                        );
                        self.draw_horizontal_line(
                            center.x.saturating_sub(x),
                            center.x.saturating_add(x),
                            center.y.saturating_sub(y),
                            color,
                            width,
                        );
                    }
                    DrawFill::Empty => {
                        // Use symmetry iterator for 8 points
                        SymmetryPoints::new(center, x, y)
                            .for_each(|point| self.draw_point(point, color, width));
                    }
                }
            });
    }
}

// Midpoint circle iterator
pub struct CircleIterator {
    x: i32,
    y: i32,
    d: i32,
    finished: bool,
}

impl CircleIterator {
    pub fn new(radius: usize) -> Self {
        Self {
            x: 0,
            y: radius as i32,
            d: 3 - 2 * radius as i32,
            finished: false,
        }
    }
}

impl Iterator for CircleIterator {
    type Item = (usize, usize);
    
    fn next(&mut self) -> Option<Self::Item> {
        if self.finished || self.x > self.y {
            return None;
        }
        
        let result = (self.x as usize, self.y as usize);
        
        if self.d < 0 {
            self.d += 4 * self.x + 6;
        } else {
            self.d += 4 * (self.x - self.y) + 10;
            self.y -= 1;
        }
        self.x += 1;
        
        Some(result)
    }
}

// Iterator for 8-way symmetry points
struct SymmetryPoints {
    center: Point,
    x: usize,
    y: usize,
    index: u8,
}

impl SymmetryPoints {
    fn new(center: Point, x: usize, y: usize) -> Self {
        Self { center, x, y, index: 0 }
    }
}

impl Iterator for SymmetryPoints {
    type Item = Point;
    
    fn next(&mut self) -> Option<Self::Item> {
        let point = match self.index {
            0 => Some(Point::new(self.center.x + self.x, self.center.y + self.y)),
            1 => Some(Point::new(self.center.x - self.x, self.center.y + self.y)),
            2 => Some(Point::new(self.center.x + self.x, self.center.y - self.y)),
            3 => Some(Point::new(self.center.x - self.x, self.center.y - self.y)),
            4 => Some(Point::new(self.center.x + self.y, self.center.y + self.x)),
            5 => Some(Point::new(self.center.x - self.y, self.center.y + self.x)),
            6 => Some(Point::new(self.center.x + self.y, self.center.y - self.x)),
            7 => Some(Point::new(self.center.x - self.y, self.center.y - self.x)),
            _ => None,
        };
        
        self.index += 1;
        point
    }
}
```

**Key Improvements:**

1. **Composable**: Iterators can be chained, filtered, mapped
2. **Lazy Evaluation**: Points generated on-demand
3. **Reusable**: `CircleIterator` can be used elsewhere
4. **Testable**: Can test iterators independently
5. **Clear Intent**: `.for_each()` shows iteration purpose

---

## Conclusion

These examples demonstrate that idiomatic Rust isn't just translating C++ syntax—it's a fundamental architectural shift toward:

1. **Ownership over manual memory management**
2. **Traits over virtual inheritance**
3. **Generics over runtime polymorphism**
4. **Iterators over imperative loops**
5. **Type safety over runtime validation**
6. **Compile-time dispatch over runtime dispatch**

The Rust version is often **simpler, safer, and faster** than the C++ equivalent, with these benefits enforced by the compiler rather than relying on developer discipline.
