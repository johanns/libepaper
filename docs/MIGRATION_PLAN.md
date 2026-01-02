# Migration Plan: BCM2835 → libgpiod + SPIdev

**Status**: Planning → Implementation  
**Goal**: Migrate from BCM2835 (requires sudo) to libgpiod (GPIO) + Linux SPIdev (SPI)

## Benefits

- ✅ **No sudo required** - User-space GPIO access via `/dev/gpiomem`
- ✅ **Developer-friendly** - Existing GPIO API (`set_pin_input`, `read_pin`) already supports button handling
- ✅ **Modern standard** - libgpiod is the Linux GPIO standard
- ✅ **Better security** - No root access needed

---

## Phase 1: Research & Setup

### 1.1 Dependencies
- [ ] Verify libgpiod installation: `sudo apt install libgpiod-dev`
- [ ] Verify SPIdev kernel module is loaded (`lsmod | grep spi`)
- [ ] Check `/dev/spidev0.0` exists and permissions
- [ ] Document user group setup (add user to `gpio` group)
- [ ] Research pin mapping: BCM pin numbers → libgpiod chip/offset

### 1.2 Current State Documentation
- [x] Document current BCM2835 usage in `src/device.cpp`
- [x] Identify GPIO operations (RST, DC, CS, BUSY pins)
- [x] Identify SPI operations (data transfer)
- [ ] Document SPI configuration (mode, speed, bit order)

---

## Phase 2: GPIO Migration (libgpiod)

### 2.1 CMake Updates
- [ ] Remove BCM2835 dependency from `CMakeLists.txt`
- [ ] Add libgpiod dependency using `pkg_check_modules`
- [ ] Update link libraries
- [ ] Update setup script (`bin/setup`) to install libgpiod

### 2.2 Header Updates (`include/epaper/device.hpp`)
- [ ] Remove `#include <bcm2835.h>`
- [ ] Update Device class documentation (remove BCM2835 references)
- [ ] Keep existing GPIO API unchanged (no API changes needed)

### 2.3 Implementation Updates (`src/device.cpp`)

#### GPIO Initialization
- [ ] Replace `bcm2835_init()` with `gpiod_chip_open_by_name("gpiochip0")`
- [ ] Store chip handle in PImpl structure
- [ ] Handle chip open failures

#### Pin Operations
- [ ] Replace `bcm2835_gpio_fsel()` with `gpiod_chip_get_line()` + `gpiod_line_request_output()`/`gpiod_line_request_input()`
- [ ] Implement `set_pin_output()` using libgpiod
- [ ] Implement `set_pin_input()` using libgpiod
- [ ] Store line handles for reuse (avoid repeated lookups)

#### Read/Write Operations
- [ ] Replace `bcm2835_gpio_write()` with `gpiod_line_set_value()`
- [ ] Replace `bcm2835_gpio_lev()` with `gpiod_line_get_value()`
- [ ] Implement `write_pin()` using libgpiod
- [ ] Implement `read_pin()` using libgpiod

#### Cleanup
- [ ] Update destructor to release gpiod lines and chip
- [ ] Update move constructor/assignment for libgpiod handles
- [ ] Handle resource cleanup on errors

### 2.4 Error Handling
- [ ] Add `GPIOInitFailed` error code (if needed)
- [ ] Handle `gpiod_chip_open()` failures
- [ ] Handle `gpiod_line_request_*()` failures
- [ ] Add descriptive error messages

---

## Phase 3: SPI Migration (Linux SPIdev)

### 3.1 Header Updates
- [ ] Add Linux SPI headers: `<linux/spi/spidev.h>`
- [ ] Add system headers: `<fcntl.h>`, `<unistd.h>`, `<sys/ioctl.h>`
- [ ] Update Device class documentation

### 3.2 Implementation Updates

#### SPI Initialization
- [ ] Replace `bcm2835_spi_begin()` with `open("/dev/spidev0.0", O_RDWR)`
- [ ] Store file descriptor in PImpl structure
- [ ] Configure SPI mode (SPI_MODE_0) via `ioctl(SPI_IOC_WR_MODE)`
- [ ] Calculate and set SPI speed (from CLOCK_DIVIDER_128: 250MHz/128 ≈ 1.95MHz)
- [ ] Configure bit order (MSB first) via `ioctl(SPI_IOC_WR_BITS_PER_WORD)` if needed
- [ ] Handle device open failures

#### SPI Transfer Operations
- [ ] Replace `bcm2835_spi_transfer()` with `ioctl(SPI_IOC_MESSAGE)`
- [ ] Implement `spi_transfer()` using SPIdev
- [ ] Implement `spi_write()` using SPIdev (batch transfers)
- [ ] Handle CS pin (may need manual GPIO control vs hardware CS)

#### Cleanup
- [ ] Update destructor to close SPI device file descriptor
- [ ] Update move semantics for file descriptor

### 3.3 Error Handling
- [ ] Add `SPIDeviceOpenFailed` error code (if needed)
- [ ] Add `SPIConfigFailed` error code (if needed)
- [ ] Handle `open()` failures
- [ ] Handle `ioctl()` failures
- [ ] Add descriptive error messages

---

## Phase 4: Delay Functions

### 4.1 Implementation
- [ ] Replace `bcm2835_delay()` with `std::this_thread::sleep_for()`
- [ ] Replace `bcm2835_delayMicroseconds()` with `std::this_thread::sleep_for()`
- [ ] Keep static methods for API compatibility
- [ ] Add `<thread>` and `<chrono>` includes

---

## Phase 5: Internal Structure (PImpl)

### 5.1 Design
- [ ] Create PImpl structure for libgpiod handles:
  - `gpiod_chip* chip`
  - `std::unordered_map<Pin, gpiod_line*> lines` (for pin reuse)
- [ ] Add SPI file descriptor: `int spi_fd`
- [ ] Update private members to use PImpl

### 5.2 Implementation
- [ ] Move implementation details to PImpl
- [ ] Update all methods to use PImpl
- [ ] Ensure proper resource cleanup

---

## Phase 6: Testing

### 6.1 Unit Tests
- [ ] Test GPIO pin operations (read/write)
- [ ] Test SPI transfers
- [ ] Test initialization
- [ ] Test error conditions
- [ ] Test move semantics

### 6.2 Integration Tests
- [ ] Test with actual hardware
- [ ] Verify display initialization works
- [ ] Verify display refresh works
- [ ] Test all driver operations (EPD27)
- [ ] Compare timing with BCM2835 version
- [ ] Verify no sudo required

### 6.3 Hardware Verification
- [ ] GPIO pins work correctly (RST, DC, CS, BUSY)
- [ ] SPI transfers complete successfully
- [ ] Display initializes correctly
- [ ] Display refresh works
- [ ] All display modes work (BlackWhite, Grayscale4)
- [ ] All orientations work

---

## Phase 7: Documentation

### 7.1 Code Documentation
- [ ] Update Device class Doxygen comments
- [ ] Document libgpiod usage in comments
- [ ] Document SPIdev usage in comments
- [ ] Update inline comments

### 7.2 User Documentation
- [ ] Update README.md (no sudo required, new setup steps)
- [ ] Update setup instructions (libgpiod installation, user group)
- [ ] Update API.md (note GPIO API can be used for buttons)
- [ ] Update ARCHITECTURE.md (replace BCM2835 references)
- [ ] Update examples if needed

### 7.3 Setup Instructions
- [ ] Document libgpiod installation: `sudo apt install libgpiod-dev`
- [ ] Document user group setup: `sudo adduser $USER gpio`
- [ ] Document SPIdev verification: `ls -l /dev/spidev0.0`
- [ ] Update `bin/setup` script

---

## Phase 8: Cleanup

### 8.1 Code Cleanup
- [ ] Remove all BCM2835 code
- [ ] Remove unused includes
- [ ] Code formatting (clang-format)
- [ ] Remove debug code

### 8.2 Build System
- [ ] Remove BCM2835 from dependencies
- [ ] Clean build artifacts
- [ ] Verify clean build

---

## Implementation Notes

### GPIO Pin Mapping
- BCM2835 uses BCM pin numbers directly (e.g., GPIO 17)
- libgpiod uses chip name and offset
- Mapping: BCM pin → chip "gpiochip0", offset = pin number
- Example: GPIO 17 → `gpiod_chip_get_line(chip, 17)`

### SPI Configuration
- **Mode**: SPI_MODE_0 (CPOL=0, CPHA=0)
- **Speed**: Calculate from CLOCK_DIVIDER_128
  - BCM2835: 250MHz / 128 = ~1.95MHz
  - Set via `ioctl(SPI_IOC_WR_MAX_SPEED_HZ)`
- **Bit order**: MSB first (default)
- **CS handling**: May need manual GPIO control (CS pin)

### Delay Functions
- Replace blocking delays with `std::this_thread::sleep_for()`
- Keep API the same (static methods)
- Consider using `std::chrono` for better precision

### Error Codes
- Add to `include/epaper/errors.hpp`:
  - `GPIOInitFailed` - libgpiod initialization failed
  - `GPIORequestFailed` - Line request failed
  - `SPIDeviceOpenFailed` - SPIdev open failed
  - `SPIConfigFailed` - SPI configuration failed

### Button Support (Developer Usage)
The existing GPIO API already supports button handling:
```cpp
// Developer can use existing API:
device.set_pin_input(button_pin);  // Configure button pin
bool pressed = device.read_pin(button_pin);  // Read button state
```
No library changes needed - developers can implement their own button handling.

---

## Timeline Estimate

- Phase 1: Research & Setup - 1-2 hours
- Phase 2: GPIO Migration - 4-6 hours
- Phase 3: SPI Migration - 3-4 hours
- Phase 4: Delay Functions - 1 hour
- Phase 5: PImpl Structure - 2 hours
- Phase 6: Testing - 4-6 hours
- Phase 7: Documentation - 2-3 hours
- Phase 8: Cleanup - 1 hour

**Total Estimate**: 18-25 hours

---

## Risk Assessment

### High Risk
- SPI timing differences may affect display operation
- CS pin handling differences between BCM2835 and SPIdev
- Pin mapping accuracy (BCM → chip offset)

### Medium Risk
- User permissions setup
- Performance differences
- Resource cleanup on errors

### Low Risk
- Documentation updates
- Code cleanup
- Example updates

---

## Next Steps

1. ✅ Complete Phase 1: Verify dependencies and setup
2. Begin Phase 2: Start GPIO migration implementation
3. Test incrementally after each phase
