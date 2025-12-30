# Driver Development Guide

A comprehensive guide to writing custom display drivers for libepaper.

## Table of Contents

- [Overview](#overview)
- [Driver Interface](#driver-interface)
- [Step-by-Step: Writing a Driver](#step-by-step-writing-a-driver)
- [EPD27 as Reference](#epd27-as-reference)
- [Hardware Integration](#hardware-integration)
- [Display Modes](#display-modes)
- [State Management](#state-management)
- [Error Handling in Drivers](#error-handling-in-drivers)
- [Testing Your Driver](#testing-your-driver)
- [Optimization Tips](#optimization-tips)
- [Implementation Checklist](#implementation-checklist)
- [Common Issues & Solutions](#common-issues--solutions)
- [Contributing Your Driver](#contributing-your-driver)

## Overview

### What is a Driver?

A driver is a concrete implementation of the abstract `Driver` interface that:
- Communicates with specific e-paper display hardware
- Translates generic operations into hardware-specific commands
- Manages display initialization, sleep/wake cycles, and power control
- Handles timing, busy waiting, and hardware state

### When to Write a New Driver

Write a new driver when you want to support:
- A different e-paper display size (1.54", 4.2", 7.5", etc.)
- A different manufacturer's displays
- Displays with different capabilities (partial refresh, color, faster refresh)

### Architecture

```
Application Code
       ↓
Display API (display.hpp)
       ↓
Driver Interface (driver.hpp) ← Abstract contract
       ↓
Your Driver (your_epd.cpp) ← Implement this!
       ↓
Hardware Device (device.hpp)
       ↓
BCM2835/SPI/GPIO
```

## Driver Interface

### Required Methods

All drivers must implement the pure virtual methods of the `Driver` interface:

```cpp
// include/epaper/drivers/driver.hpp
class Driver {
public:
  virtual ~Driver() = default;

  // Lifecycle
  virtual auto init(DisplayMode mode) -> std::expected<void, Error> = 0;
  virtual auto clear() -> std::expected<void, Error> = 0;
  virtual auto display(std::span<const std::byte> buffer) -> std::expected<void, Error> = 0;

  // Power management
  virtual auto sleep() -> std::expected<void, Error> = 0;
  virtual auto wake() -> std::expected<void, Error> = 0;
  virtual auto power_off() -> std::expected<void, Error> = 0;
  virtual auto power_on() -> std::expected<void, Error> = 0;

  // Capabilities
  [[nodiscard]] virtual auto width() const noexcept -> std::size_t = 0;
  [[nodiscard]] virtual auto height() const noexcept -> std::size_t = 0;
  [[nodiscard]] virtual auto mode() const noexcept -> DisplayMode = 0;
  [[nodiscard]] virtual auto buffer_size() const noexcept -> std::size_t = 0;
  [[nodiscard]] virtual auto supports_partial_refresh() const noexcept -> bool = 0;
  [[nodiscard]] virtual auto supports_power_control() const noexcept -> bool = 0;
};
```

### Method Responsibilities

**`init(DisplayMode mode)`**
- Initialize display hardware
- Load appropriate LUT (Look-Up Table) for mode
- Set up display parameters
- Must be called before other operations
- Returns error if hardware communication fails

**`clear()`**
- Clear display to white (or default background)
- Send clear command to hardware
- Usually calls `wait_busy()` internally

**`display(std::span<const std::byte> buffer)`**
- Transfer framebuffer to display RAM
- Trigger display update
- **Important:** For transparent sleep/wake, check if asleep and call `wake()` if needed
- Block until update completes

**`sleep()`**
- Put display into low-power sleep mode
- Set `is_asleep_` flag to true
- Should be idempotent (safe to call when already asleep)

**`wake()`**
- Wake display from sleep
- For EPD27: requires full re-initialization (~2 seconds)
- Set `is_asleep_` flag to false
- Should be idempotent (safe to call when already awake)

**`power_off()` / `power_on()`**
- Complete power control (if supported)
- Return error if not supported by hardware

**Capability Queries:**
- `width()`, `height()`: Physical display dimensions
- `mode()`: Current display mode
- `buffer_size()`: Required framebuffer size in bytes
- `supports_partial_refresh()`: Can do partial screen updates?
- `supports_power_control()`: Has power control pins?

## Step-by-Step: Writing a Driver

### Step 1: Hardware Analysis

Before writing code, study the display datasheet:

**Key Information to Extract:**
1. **Dimensions**: Width and height in pixels
2. **Command Set**: List of commands and their hex codes
3. **Initialization Sequence**: Commands to initialize display
4. **LUT Requirements**: Look-up tables for grayscale/refresh modes
5. **Timing**: Delays needed between commands
6. **Power Control**: Sleep/wake procedures
7. **Busy Pin Behavior**: How to detect ready state

**Example: EPD27 Datasheet Analysis**
- Dimensions: 176×264 pixels
- Commands: `0x10` (sleep), `0x12` (reset), `0x20` (activate), etc.
- Initialization: 12-step sequence with specific delays
- LUT: 30-byte table for grayscale mode
- Busy pin: HIGH = busy, LOW = ready

### Step 2: Header File

Create a header in `include/epaper/drivers/`:

```cpp
// include/epaper/drivers/epd42.hpp (example for 4.2" display)
#pragma once

#include <epaper/drivers/driver.hpp>
#include <epaper/device.hpp>
#include <epaper/display.hpp>
#include <epaper/errors.hpp>
#include <cstddef>
#include <expected>

namespace epaper {

class EPD42 : public Driver {
public:
  // Display dimensions (from datasheet)
  static constexpr std::size_t WIDTH = 400;
  static constexpr std::size_t HEIGHT = 300;

  explicit EPD42(Device& device);
  ~EPD42() override = default;

  // Lifecycle
  auto init(DisplayMode mode) -> std::expected<void, Error> override;
  auto clear() -> std::expected<void, Error> override;
  auto display(std::span<const std::byte> buffer) -> std::expected<void, Error> override;

  // Power management
  auto sleep() -> std::expected<void, Error> override;
  auto wake() -> std::expected<void, Error> override;
  auto power_off() -> std::expected<void, Error> override;
  auto power_on() -> std::expected<void, Error> override;

  // Capabilities
  [[nodiscard]] auto width() const noexcept -> std::size_t override { return WIDTH; }
  [[nodiscard]] auto height() const noexcept -> std::size_t override { return HEIGHT; }
  [[nodiscard]] auto mode() const noexcept -> DisplayMode override { return current_mode_; }
  [[nodiscard]] auto buffer_size() const noexcept -> std::size_t override;
  [[nodiscard]] auto supports_partial_refresh() const noexcept -> bool override { return false; }
  [[nodiscard]] auto supports_power_control() const noexcept -> bool override { return false; }

private:
  // Hardware communication
  auto reset() -> void;
  auto send_command(std::uint8_t command) -> void;
  auto send_data(std::uint8_t data) -> void;
  auto send_data(std::span<const std::uint8_t> data) -> void;
  auto wait_busy() -> void;

  // Display-specific
  auto set_lut_bw() -> void;
  auto set_lut_gray() -> void;
  auto set_memory_area(std::uint16_t x_start, std::uint16_t y_start,
                       std::uint16_t x_end, std::uint16_t y_end) -> void;
  auto set_memory_pointer(std::uint16_t x, std::uint16_t y) -> void;

  // State
  Device& device_;
  DisplayMode current_mode_;
  bool initialized_ = false;
  bool is_asleep_ = false;  // Track sleep state for transparent wake
};

}  // namespace epaper
```

### Step 3: Initialization Sequence

Implement the `init()` method with the hardware initialization sequence:

```cpp
// src/drivers/epd42.cpp
#include <epaper/drivers/epd42.hpp>
#include <thread>
#include <chrono>

namespace epaper {

namespace {
  // Command definitions (from datasheet)
  constexpr std::uint8_t SW_RESET = 0x12;
  constexpr std::uint8_t DRIVER_OUTPUT_CONTROL = 0x01;
  constexpr std::uint8_t DATA_ENTRY_MODE = 0x11;
  constexpr std::uint8_t DISPLAY_UPDATE_CONTROL = 0x21;
  constexpr std::uint8_t MASTER_ACTIVATION = 0x20;
  constexpr std::uint8_t WRITE_LUT = 0x32;
  // ... more commands ...
}

EPD42::EPD42(Device& device) : device_(device) {}

auto EPD42::init(DisplayMode mode) -> std::expected<void, Error> {
  current_mode_ = mode;

  // Hardware reset
  reset();

  // Software reset
  send_command(SW_RESET);
  wait_busy();

  // Driver output control (from datasheet)
  send_command(DRIVER_OUTPUT_CONTROL);
  send_data((HEIGHT - 1) & 0xFF);        // Y pixels low byte
  send_data(((HEIGHT - 1) >> 8) & 0xFF); // Y pixels high byte
  send_data(0x00);                       // GD = 0, SM = 0, TB = 0

  // Data entry mode (X increment, Y increment)
  send_command(DATA_ENTRY_MODE);
  send_data(0x03);  // Increment X and Y

  // Set RAM area (full screen)
  set_memory_area(0, 0, WIDTH - 1, HEIGHT - 1);

  // Set RAM pointer to origin
  set_memory_pointer(0, 0);

  // Load LUT based on mode
  if (mode == DisplayMode::Grayscale4) {
    set_lut_gray();
  } else {
    set_lut_bw();
  }

  // Display update control
  send_command(DISPLAY_UPDATE_CONTROL);
  send_data(0xC7);  // Enable clock, enable analog, display mode 1, disable OSC

  // Master activation (required before first display update)
  send_command(MASTER_ACTIVATION);
  wait_busy();

  initialized_ = true;
  is_asleep_ = false;  // Awake after init

  return {};
}

}  // namespace epaper
```

### Step 4: Display Buffer Transfer

Implement the `display()` method with **transparent wake support**:

```cpp
auto EPD42::display(std::span<const std::byte> buffer) -> std::expected<void, Error> {
  if (!initialized_) {
    return std::unexpected(Error{ErrorCode::DriverNotInitialized,
                                 "EPD42 not initialized"});
  }

  // IMPORTANT: Transparent wake management
  if (is_asleep_) {
    auto wake_result = wake();  // Re-initialize if asleep
    if (!wake_result) {
      return wake_result;  // Propagate error
    }
  }

  const std::size_t expected_size = buffer_size();
  if (buffer.size() != expected_size) {
    return std::unexpected(Error{ErrorCode::InvalidDimensions,
                                 "Buffer size mismatch"});
  }

  // Set memory pointer to origin
  set_memory_pointer(0, 0);

  // Write buffer to display RAM
  constexpr std::uint8_t WRITE_RAM = 0x24;
  send_command(WRITE_RAM);

  // Transfer buffer in chunks (SPI limitation)
  constexpr std::size_t CHUNK_SIZE = 4096;
  for (std::size_t i = 0; i < buffer.size(); i += CHUNK_SIZE) {
    std::size_t chunk_size = std::min(CHUNK_SIZE, buffer.size() - i);
    auto chunk = buffer.subspan(i, chunk_size);

    // Convert std::byte to uint8_t for send_data
    std::vector<std::uint8_t> data(chunk_size);
    std::transform(chunk.begin(), chunk.end(), data.begin(),
                  [](std::byte b) { return static_cast<std::uint8_t>(b); });
    send_data(data);
  }

  // Trigger display update
  send_command(MASTER_ACTIVATION);
  wait_busy();  // Block until complete (~2 seconds)

  return {};
}
```

### Step 5: Power Management

Implement sleep/wake with proper state tracking:

```cpp
auto EPD42::sleep() -> std::expected<void, Error> {
  // Idempotent: safe to call when already asleep
  if (is_asleep_) {
    return {};
  }

  constexpr std::uint8_t DEEP_SLEEP_MODE = 0x10;
  send_command(DEEP_SLEEP_MODE);
  send_data(0x01);  // Enter deep sleep mode

  is_asleep_ = true;
  return {};
}

auto EPD42::wake() -> std::expected<void, Error> {
  // Idempotent: safe to call when already awake
  if (!is_asleep_) {
    return {};
  }

  // EPD42 (like EPD27) requires full re-initialization after deep sleep
  auto result = init(current_mode_);
  if (!result) {
    return result;
  }

  is_asleep_ = false;
  return {};
}

auto EPD42::power_off() -> std::expected<void, Error> {
  // Not supported by this hardware
  return std::unexpected(Error{ErrorCode::UnsupportedOperation,
                               "EPD42 does not support power_off"});
}

auto EPD42::power_on() -> std::expected<void, Error> {
  // Not supported by this hardware
  return std::unexpected(Error{ErrorCode::UnsupportedOperation,
                               "EPD42 does not support power_on"});
}
```

### Step 6: Testing

Write tests to validate your driver (see [Testing Your Driver](#testing-your-driver)).

## EPD27 as Reference

The `EPD27` driver is the canonical reference implementation. Study it for patterns:

### Key Patterns from EPD27

**1. Hardware Reset:**
```cpp
auto EPD27::reset() -> void {
  device_.write_pin(pins::RST, true);
  device_.delay_ms(200);
  device_.write_pin(pins::RST, false);
  device_.delay_ms(2);
  device_.write_pin(pins::RST, true);
  device_.delay_ms(200);
}
```

**2. Command/Data Sending:**
```cpp
auto EPD27::send_command(std::uint8_t command) -> void {
  device_.write_pin(pins::DC, false);  // DC LOW = command
  device_.spi_transfer(std::span{reinterpret_cast<const std::byte*>(&command), 1});
}

auto EPD27::send_data(std::uint8_t data) -> void {
  device_.write_pin(pins::DC, true);   // DC HIGH = data
  device_.spi_transfer(std::span{reinterpret_cast<const std::byte*>(&data), 1});
}
```

**3. Busy Waiting:**
```cpp
auto EPD27::wait_busy() -> void {
  // EPD27 BUSY pin: HIGH = busy, LOW = ready
  while (device_.read_pin(pins::BUSY)) {
    device_.delay_ms(10);  // Poll every 10ms
  }
}
```

**4. LUT Loading:**
```cpp
auto EPD27::set_lut() -> void {
  // 30-byte Look-Up Table for grayscale waveforms
  constexpr std::array<std::uint8_t, 30> lut_full = {
    0x02, 0x02, 0x01, 0x11, 0x12, 0x12, 0x22, 0x22,
    0x66, 0x69, 0x69, 0x59, 0x58, 0x99, 0x99, 0x88,
    // ... rest of LUT ...
  };

  send_command(WRITE_LUT_REGISTER);
  for (auto byte : lut_full) {
    send_data(byte);
  }
}
```

### EPD27 State Management

Study how EPD27 implements transparent wake:

```cpp
// In display() method:
if (is_asleep_) {
  auto wake_result = wake();
  if (!wake_result) {
    return wake_result;
  }
}
// ... proceed with display update ...
```

## Hardware Integration

### SPI Communication

Use the `Device` class for all hardware access:

```cpp
// GPIO pin control
device_.write_pin(pins::RST, true);   // Set RST HIGH
device_.write_pin(pins::DC, false);   // Set DC LOW
bool busy = device_.read_pin(pins::BUSY);  // Read BUSY pin

// SPI transfer
std::array<std::byte, 4> data = {std::byte{0x01}, std::byte{0x02}, ...};
device_.spi_transfer(data);

// Delays
device_.delay_ms(100);  // Wait 100ms
```

### Pin Usage

Standard e-paper pins:

| Pin | Direction | Purpose |
|-----|-----------|---------|
| RST | Output | Hardware reset (LOW to reset) |
| DC  | Output | Data/Command select (LOW = command, HIGH = data) |
| CS  | Output | Chip select (handled by SPI library) |
| BUSY | Input | Status (HIGH = busy, LOW = ready) |
| MOSI | Output | SPI data out (handled by SPI library) |
| SCLK | Output | SPI clock (handled by SPI library) |

### Command/Data Protocol

```cpp
// Send command
void send_command(uint8_t cmd) {
  device_.write_pin(pins::DC, false);  // Command mode
  device_.spi_transfer(span of cmd);
}

// Send data
void send_data(uint8_t data) {
  device_.write_pin(pins::DC, true);   // Data mode
  device_.spi_transfer(span of data);
}

// Typical command sequence
send_command(SOME_COMMAND);
send_data(param1);
send_data(param2);
wait_busy();
```

## Display Modes

### Implementing Black/White Mode

**Buffer Format:** 1 bit per pixel, 8 pixels per byte

```cpp
auto EPD42::buffer_size() const noexcept -> std::size_t {
  if (current_mode_ == DisplayMode::BlackWhite) {
    // 1 bit per pixel: (width * height) / 8
    return (WIDTH * HEIGHT) / 8;
  } else {
    // Grayscale4: 2 bits per pixel: (width * height) / 4
    return (WIDTH * HEIGHT) / 4;
  }
}
```

**Pixel Encoding:**
- Bit = 0: White pixel
- Bit = 1: Black pixel

**Byte Packing (MSB first):**
```
Byte: [P7 P6 P5 P4 P3 P2 P1 P0]
P0 = leftmost pixel
P7 = rightmost pixel
```

### Implementing Grayscale4 Mode

**Buffer Format:** 2 bits per pixel, 4 pixels per byte

**Pixel Encoding:**
- `00`: White (0)
- `01`: Light gray (1)
- `10`: Dark gray (2)
- `11`: Black (3)

**LUT Requirements:**
Grayscale mode requires a Look-Up Table (LUT) that defines voltage waveforms for each gray level. Consult your display's datasheet for the correct LUT values.

```cpp
auto EPD42::set_lut_gray() -> void {
  constexpr std::array<std::uint8_t, 30> lut_gray = {
    // Voltage waveforms for 4-level grayscale (from datasheet)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // ... (values specific to your display)
  };

  send_command(WRITE_LUT);
  for (auto byte : lut_gray) {
    send_data(byte);
  }
}
```

## State Management

### Required State Variables

```cpp
class YourDriver : public Driver {
private:
  Device& device_;           // Hardware reference
  DisplayMode current_mode_; // Current display mode
  bool initialized_ = false; // Initialization state
  bool is_asleep_ = false;   // Sleep state (for transparent wake)
};
```

### State Transitions

```cpp
// Initialization
init(mode) → initialized_ = true, is_asleep_ = false

// Sleep
sleep() → is_asleep_ = true (if not already)

// Wake
wake() → is_asleep_ = false, re-init hardware

// Display (with transparent wake)
display(buffer) → check is_asleep_, call wake() if needed, then transfer
```

### Idempotent Operations

Make sleep/wake idempotent (safe to call multiple times):

```cpp
auto YourDriver::sleep() -> std::expected<void, Error> {
  if (is_asleep_) {
    return {};  // Already asleep, nothing to do
  }

  // Send sleep command
  send_command(DEEP_SLEEP);
  is_asleep_ = true;
  return {};
}

auto YourDriver::wake() -> std::expected<void, Error> {
  if (!is_asleep_) {
    return {};  // Already awake, nothing to do
  }

  // Re-initialize (or send wake command if supported)
  auto result = init(current_mode_);
  if (!result) {
    return result;
  }

  is_asleep_ = false;
  return {};
}
```

## Error Handling in Drivers

### When to Return Errors

Return `std::unexpected(Error{...})` for:
- Hardware communication failures
- Timeouts (e.g., BUSY pin stuck HIGH)
- Invalid state (e.g., `display()` called before `init()`)
- Unsupported operations

### Error Codes to Use

```cpp
// Initialization errors
return std::unexpected(Error{ErrorCode::DriverInitFailed,
                             "Hardware not responding"});

// State errors
if (!initialized_) {
  return std::unexpected(Error{ErrorCode::DriverNotInitialized,
                               "Call init() first"});
}

// Hardware errors
return std::unexpected(Error{ErrorCode::HardwareTimeout,
                             "BUSY pin timeout after 5 seconds"});

// Unsupported features
return std::unexpected(Error{ErrorCode::UnsupportedOperation,
                             "Partial refresh not supported"});
```

### Error Propagation

Use `std::expected` chaining:

```cpp
auto YourDriver::some_operation() -> std::expected<void, Error> {
  auto result = sub_operation();
  if (!result) {
    return result;  // Propagate error
  }

  // Continue with success path
  return {};
}
```

## Testing Your Driver

### Test Categories

**1. Basic Initialization:**
```cpp
TEST(YourDriverTest, InitializeSuccessfully) {
  MockDevice device;
  YourDriver driver(device);

  auto result = driver.init(DisplayMode::BlackWhite);
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(driver.width(), EXPECTED_WIDTH);
  EXPECT_EQ(driver.height(), EXPECTED_HEIGHT);
}
```

**2. Display Modes:**
```cpp
TEST(YourDriverTest, SupportsBothModes) {
  MockDevice device;
  YourDriver driver(device);

  EXPECT_TRUE(driver.init(DisplayMode::BlackWhite).has_value());
  EXPECT_TRUE(driver.init(DisplayMode::Grayscale4).has_value());
}
```

**3. Sleep/Wake Cycles:**
```cpp
TEST(YourDriverTest, TransparentWakeWorks) {
  MockDevice device;
  YourDriver driver(device);
  driver.init(DisplayMode::BlackWhite);

  std::vector<std::byte> buffer(driver.buffer_size(), std::byte{0});

  // First display
  EXPECT_TRUE(driver.display(buffer).has_value());

  // Sleep
  EXPECT_TRUE(driver.sleep().has_value());

  // Second display (should auto-wake)
  EXPECT_TRUE(driver.display(buffer).has_value());
}
```

**4. Error Conditions:**
```cpp
TEST(YourDriverTest, RejectsOperationsBeforeInit) {
  MockDevice device;
  YourDriver driver(device);

  std::vector<std::byte> buffer(1000);
  auto result = driver.display(buffer);

  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error().code, ErrorCode::DriverNotInitialized);
}
```

### Hardware Testing Checklist

Test with actual hardware:
- [ ] Initialization completes without errors
- [ ] Display clears to white
- [ ] Simple pattern (checkerboard) displays correctly
- [ ] Text renders legibly
- [ ] All four orientations work
- [ ] Sleep/wake cycle works
- [ ] Auto-sleep + transparent wake works
- [ ] Multiple refresh cycles work
- [ ] Both display modes work (B/W and grayscale)
- [ ] No memory leaks over many cycles

## Optimization Tips

### SPI Transfer Optimization

**Batch Transfers:**
```cpp
// Good: Single large transfer
send_data(std::span<const uint8_t>{buffer.data(), buffer.size()});

// Bad: Many small transfers (slow!)
for (auto byte : buffer) {
  send_data(byte);  // Inefficient!
}
```

**Chunk Large Buffers:**
```cpp
// For very large buffers, transfer in chunks
constexpr size_t CHUNK_SIZE = 4096;
for (size_t i = 0; i < buffer.size(); i += CHUNK_SIZE) {
  size_t chunk_size = std::min(CHUNK_SIZE, buffer.size() - i);
  send_data(buffer.subspan(i, chunk_size));
}
```

### Memory Efficiency

**LUT Storage:**
```cpp
// Good: Compile-time constant
constexpr std::array<uint8_t, 30> LUT = { ... };

// Bad: Heap allocation
std::vector<uint8_t> lut = { ... };  // Unnecessary allocation
```

**Buffer Reuse:**
```cpp
// Good: Reuse internal buffers
class YourDriver {
  std::vector<uint8_t> transfer_buffer_;  // Reused for conversions
};

// Bad: Allocate on every call
auto display(buffer) {
  std::vector<uint8_t> temp(buffer.size());  // Allocates every time!
}
```

### Performance Profiling

Profile critical sections:

```cpp
#include <chrono>

auto start = std::chrono::steady_clock::now();
// ... operation ...
auto end = std::chrono::steady_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
std::cout << "Operation took " << duration.count() << "ms\n";
```

## Implementation Checklist

Before submitting your driver:

### Code Completeness
- [ ] All pure virtual methods implemented
- [ ] Header guards and proper includes
- [ ] Namespace `epaper`
- [ ] Constants defined for all commands
- [ ] Width/height constants match datasheet
- [ ] Buffer size calculation correct

### State Management
- [ ] `initialized_` flag used correctly
- [ ] `is_asleep_` flag maintained
- [ ] `current_mode_` tracked
- [ ] Idempotent sleep/wake

### Transparent Sleep/Wake
- [ ] `display()` checks `is_asleep_` and calls `wake()` if needed
- [ ] `sleep()` sets `is_asleep_ = true`
- [ ] `wake()` clears `is_asleep_ = false`
- [ ] `init()` clears `is_asleep_ = false`

### Error Handling
- [ ] All operations return `std::expected<void, Error>`
- [ ] Appropriate error codes used
- [ ] Error messages are descriptive
- [ ] Edge cases handled (null buffers, wrong size, etc.)

### Testing
- [ ] Unit tests pass
- [ ] Hardware tests pass
- [ ] Sleep/wake cycle tested
- [ ] Both display modes tested
- [ ] Multiple orientations tested

### Documentation
- [ ] Header comments explain purpose
- [ ] Public methods documented
- [ ] Example usage provided
- [ ] Added to README driver list

### Code Quality
- [ ] Follows project code style (clang-format)
- [ ] No compiler warnings
- [ ] Const correctness
- [ ] RAII for resources
- [ ] No raw pointers

## Common Issues & Solutions

### Display Not Responding

**Problem:** Display doesn't update or shows garbage.

**Solutions:**
1. Verify initialization sequence matches datasheet exactly
2. Check BUSY pin logic (HIGH vs LOW varies by display)
3. Ensure reset sequence has correct delays
4. Verify SPI transfer byte order (MSB vs LSB first)
5. Check DC pin toggling (command vs data)

### Slow Refresh

**Problem:** Display update takes too long.

**Solutions:**
1. Batch SPI transfers (don't send byte-by-byte)
2. Reduce LUT complexity if possible
3. Check for unnecessary delays in code
4. Profile to find bottlenecks
5. Remember: e-paper is inherently slow (~2s is normal)

### Sleep Not Working

**Problem:** Display doesn't sleep or wake.

**Solutions:**
1. Verify sleep command is correct for your hardware
2. Check if `is_asleep_` flag is being set/cleared correctly
3. Ensure `display()` checks `is_asleep_` and calls `wake()`
4. Some displays need special wake sequence (not just re-init)
5. Consult datasheet for power-down requirements

### Grayscale Not Working

**Problem:** Only black and white appear, no grays.

**Solutions:**
1. Verify LUT is loaded correctly
2. Check LUT values match datasheet for grayscale mode
3. Ensure buffer encoding is correct (2 bits per pixel)
4. Some displays don't support grayscale - check capabilities
5. Test with simple grayscale pattern first

## Contributing Your Driver

### Pull Request Requirements

When contributing a new driver:

1. **Code:**
   - Header: `include/epaper/drivers/your_epd.hpp`
   - Implementation: `src/drivers/your_epd.cpp`
   - Follow existing naming conventions

2. **Tests:**
   - Unit tests: `tests/test_your_epd.cpp`
   - Hardware validation log (if possible)

3. **Documentation:**
   - Update `README.md` with driver in "Supported Hardware" section
   - Add example usage in `examples/`
   - Datasheet link or reference

4. **CMake Integration:**
   - Add to `src/CMakeLists.txt`
   - Add test to `tests/CMakeLists.txt`

### Pull Request Template

```markdown
## New Driver: EPD42 (4.2" Display)

### Hardware Specifications
- Display: Waveshare 4.2" e-Paper
- Resolution: 400×300 pixels
- Modes: Black/White, 4-level Grayscale
- Partial Refresh: No
- Power Control: No

### Implementation Details
- Initialization: 15-step sequence based on datasheet v2.1
- Sleep: Deep sleep mode (command 0x10)
- Wake: Full re-initialization required
- Transparent wake: Implemented via `is_asleep_` flag

### Testing
- [x] Unit tests pass
- [x] Hardware tested on Raspberry Pi 4
- [x] All orientations verified
- [x] Both display modes verified
- [x] Sleep/wake cycles verified
- [x] Example program included

### Datasheet
https://www.waveshare.com/wiki/4.2inch_e-Paper_Module

### Checklist
- [x] Code follows project style
- [x] All tests pass
- [x] Documentation updated
- [x] Example included
```

### Maintenance Responsibilities

As a driver contributor, you're expected to:
- Respond to issues related to your driver
- Test driver with library updates
- Update driver if API changes
- Provide hardware support to users (within reason)

---

**For architecture details, see [ARCHITECTURE.md](ARCHITECTURE.md).**
**For API usage, see [API.md](API.md).**
**For examples, see [examples/README.md](../examples/README.md).**
