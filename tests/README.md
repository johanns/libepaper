# Manual Test Suite for libepaper2

This directory contains a comprehensive manual test suite for the libepaper2 library. Since e-paper displays require physical verification of correct operation, these tests are designed to be run manually with visual inspection.

## Overview

The test suite consists of 11 standalone test programs that exercise both features and edge cases of the library. Each test displays visual patterns on the e-paper display that can be verified by inspection.

## Running Tests

### Run All Tests

To run all tests in sequence with automatic pauses between tests:

```bash
sudo ./run_all_tests.sh
```

**Note:** Root privileges are required for GPIO/SPI access.

### Run Individual Tests

Each test can be run independently:

```bash
sudo ./test_display_modes
sudo ./test_drawing_primitives
sudo ./test_orientations
# ... etc
```

### Script Options

The `run_all_tests.sh` script supports several options:

```bash
# Run all tests
sudo ./run_all_tests.sh

# Skip a specific test
sudo ./run_all_tests.sh --skip stress

# Run only a specific test
sudo ./run_all_tests.sh test_fonts

# Run without pauses (auto mode)
sudo ./run_all_tests.sh --auto
```

## Test Programs

### 1. test_display_modes

**Purpose:** Verify both BlackWhite and Grayscale4 display modes work correctly.

**What to verify:**
- BlackWhite mode shows crisp black and white patterns
- Grayscale4 mode shows 4 distinct gray levels (Black, Gray2, Gray1, White)
- Color differences are clearly visible in grayscale mode

### 2. test_drawing_primitives

**Purpose:** Test all drawing operations (points, lines, rectangles, circles, text, numbers).

**What to verify:**
- All shapes render correctly
- Different DotPixel sizes are visible
- Solid and dotted lines are distinguishable
- Text is readable in all fonts
- Numbers and decimals render correctly

### 3. test_orientations

**Purpose:** Verify all 4 display orientations (Portrait0, Landscape90, Portrait180, Landscape270).

**What to verify:**
- Same pattern appears correctly in each orientation
- Text is readable in each orientation
- Coordinate transformations are accurate
- No distortion or artifacts

### 4. test_fonts

**Purpose:** Test all font sizes (Font8, Font12, Font16, Font20, Font24).

**What to verify:**
- All fonts are readable
- Font sizes increase appropriately
- Characters render correctly
- Numbers and text align properly

### 5. test_bitmaps

**Purpose:** Test bitmap operations (from memory, from files, scaling).

**What to verify:**
- Bitmaps from memory display correctly
- PNG, JPEG, and BMP files load correctly
- Scaling works (both up and down)
- Clipping works at boundaries

### 6. test_power_management

**Purpose:** Test power management features (sleep, wake, power_off, power_on).

**What to verify:**
- Display enters sleep mode correctly
- Display wakes from sleep (if supported)
- Power off/on cycles work correctly
- Display state is preserved/restored appropriately

### 7. test_auto_sleep

**Purpose:** Test auto-sleep functionality.

**What to verify:**
- Auto-sleep is enabled by default
- Display automatically sleeps after refresh
- Auto-sleep can be disabled
- Multiple refreshes work with both settings

### 8. test_edge_cases

**Purpose:** Test edge cases and boundary conditions.

**What to verify:**
- Out-of-bounds coordinates don't crash (silently clipped)
- Empty strings don't crash
- Very large numbers render (or are handled gracefully)
- Zero-radius/zero-size shapes don't crash
- Operations at display boundaries work correctly

### 9. test_coordinate_transforms

**Purpose:** Test coordinate transformations at boundaries.

**What to verify:**
- Drawing at corners works in all orientations
- Drawing at edges works in all orientations
- Coordinate transformations are accurate
- No artifacts at boundaries

### 10. test_error_handling

**Purpose:** Test error conditions are handled gracefully.

**What to verify:**
- File not found errors are reported clearly
- Invalid formats are handled gracefully
- Error messages are informative
- No crashes on error conditions

### 11. test_stress

**Purpose:** Test rapid operations and stress conditions.

**What to verify:**
- Multiple rapid refreshes work correctly
- Rapid drawing operations don't corrupt display
- Memory doesn't leak during long operations
- Resource cleanup works correctly

## Expected Results

All tests should:
- **Compile without warnings**
- **Execute without crashes**
- **Display clear visual patterns**
- **Handle errors gracefully**
- **Complete successfully**

## Troubleshooting

### Permission Denied Errors

Make sure to run tests with `sudo`:

```bash
sudo ./test_display_modes
```

### SPI Not Enabled

Enable SPI on Raspberry Pi:

```bash
sudo raspi-config
# Interface Options -> SPI -> Enable
```

### Display Not Responding

1. Check physical connections
2. Verify BCM2835 library is installed
3. Ensure display is powered
4. Try running with `sudo`

### Compilation Errors

Ensure you have:
- GCC 14+ or Clang 18+
- CMake 3.25+
- BCM2835 library installed

### Test Failures

If a test fails:
1. Check console output for error messages
2. Verify display connections
3. Try running test individually
4. Check display mode compatibility

## Test Development

### Adding New Tests

To add a new test:

1. Create `test_<name>.cpp` in this directory
2. Add test name to `TEST_PROGRAMS` list in `CMakeLists.txt`
3. Follow the structure of existing tests
4. Document test in this README

### Test Structure

Each test should:
1. Initialize device and display
2. Print clear test description
3. Perform test operations
4. Display visual patterns
5. Clean up resources
6. Exit with appropriate status code

### Code Standards

Follow project standards:
- C++23 features
- clang-format formatting
- Trailing return types
- Const correctness
- RAII patterns

## Hardware Setup

Default GPIO pin assignments:

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

## Contributing

If you find issues or want to add more tests:
1. Follow existing test structure
2. Ensure tests are manually verifiable
3. Document expected visual output
4. Update this README

## License

This test suite is part of the libepaper2 project and follows the same license.
