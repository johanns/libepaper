#include <cstdlib>
#include <epaper/device.hpp>
#include <epaper/display.hpp>
#include <epaper/drivers/epd27.hpp>
#include <epaper/font.hpp>
#include <iostream>
#include <limits>
#include <vector>

using namespace epaper;

auto main() -> int {
  std::cout << "=======================================\n";
  std::cout << "  Edge Cases Test\n";
  std::cout << "=======================================\n";
  std::cout << "This test exercises edge cases and boundary conditions.\n";
  std::cout << "All operations should handle edge cases gracefully.\n\n";

  try {
    // Initialize device
    std::cout << "Initializing device...\n";
    Device device;
    if (auto result = device.init(); !result) {
      std::cerr << "Device initialization failed: " << result.error().what() << "\n";
      return EXIT_FAILURE;
    }

    // Create display
    auto display = create_display<EPD27>(device, DisplayMode::BlackWhite);
    if (!display) {
      std::cerr << "Display initialization failed: " << display.error().what() << "\n";
      return EXIT_FAILURE;
    }

    const auto width = display->effective_width();
    const auto height = display->effective_height();
    std::cout << "Display size: " << width << "x" << height << "\n\n";

    display->clear(Color::White);
    display->draw_string(5, 5, "EDGE CASES TEST", Font::font16(), Color::Black, Color::White);

    // Test 1: Out-of-bounds coordinates
    std::cout << "=== Test 1: Out-of-Bounds Coordinates ===\n";
    display->draw_string(5, 25, "1. Out-of-bounds:", Font::font12(), Color::Black, Color::White);

    // Far out of bounds (should be silently clipped)
    std::cout << "  Drawing far out of bounds...\n";
    display->set_pixel(width + 1000, height + 1000, Color::Black);
    display->draw_point(width + 100, height + 100, Color::Black, DotPixel::Pixel5x5);
    display->draw_line(width + 50, height + 50, width + 100, height + 100, Color::Black);

    // Negative-like (wrapped around due to std::size_t)
    std::cout << "  Drawing with wrapped coordinates...\n";
    constexpr std::size_t very_large = std::numeric_limits<std::size_t>::max() - 10;
    display->set_pixel(very_large, very_large, Color::Black);
    display->draw_point(very_large, very_large, Color::Black, DotPixel::Pixel3x3);

    display->draw_string(5, 40, "No crash (OK)", Font::font8(), Color::Black, Color::White);
    std::cout << "✓ Out-of-bounds coordinates handled gracefully.\n\n";

    // Test 2: Boundary coordinates
    std::cout << "=== Test 2: Boundary Coordinates ===\n";
    display->draw_string(5, 55, "2. Boundaries:", Font::font12(), Color::Black, Color::White);

    std::cout << "  Drawing at exact boundaries...\n";
    // Draw at all four corners
    display->draw_point(0, 0, Color::Black, DotPixel::Pixel2x2);
    display->draw_point(width - 1, 0, Color::Black, DotPixel::Pixel2x2);
    display->draw_point(0, height - 1, Color::Black, DotPixel::Pixel2x2);
    display->draw_point(width - 1, height - 1, Color::Black, DotPixel::Pixel2x2);

    // Draw lines along edges
    display->draw_line(0, 0, width - 1, 0, Color::Black);
    display->draw_line(0, 0, 0, height - 1, Color::Black);

    display->draw_string(5, 70, "Corners marked", Font::font8(), Color::Black, Color::White);
    std::cout << "✓ Boundary coordinates handled correctly.\n\n";

    // Test 3: Empty strings
    std::cout << "=== Test 3: Empty Strings ===\n";
    display->draw_string(5, 85, "3. Empty strings:", Font::font12(), Color::Black, Color::White);

    std::cout << "  Drawing empty string...\n";
    display->draw_string(5, 100, "", Font::font12(), Color::Black, Color::White);

    display->draw_string(5, 100, "No crash (OK)", Font::font8(), Color::Black, Color::White);
    std::cout << "✓ Empty string handled gracefully.\n\n";

    // Test 4: Very large numbers
    std::cout << "=== Test 4: Large Numbers ===\n";
    display->draw_string(5, 115, "4. Large numbers:", Font::font12(), Color::Black, Color::White);

    std::cout << "  Drawing very large numbers...\n";
    display->draw_number(5, 130, std::numeric_limits<std::int32_t>::max(), Font::font8(), Color::Black, Color::White);
    display->draw_number(5, 140, std::numeric_limits<std::int32_t>::min(), Font::font8(), Color::Black, Color::White);
    display->draw_number(5, 150, 0, Font::font8(), Color::Black, Color::White);

    std::cout << "✓ Large numbers handled correctly.\n\n";

    // Test 5: Zero-size shapes
    std::cout << "=== Test 5: Zero-Size Shapes ===\n";
    display->draw_string(95, 25, "5. Zero sizes:", Font::font12(), Color::Black, Color::White);

    std::cout << "  Drawing zero-radius circle...\n";
    display->draw_circle(120, 45, 0, Color::Black, DotPixel::Pixel1x1, DrawFill::Full);

    std::cout << "  Drawing zero-width rectangle...\n";
    display->draw_rectangle(130, 40, 130, 50, Color::Black, DotPixel::Pixel1x1, DrawFill::Full);

    std::cout << "  Drawing zero-length line...\n";
    display->draw_line(140, 45, 140, 45, Color::Black, DotPixel::Pixel1x1, LineStyle::Solid);

    display->draw_string(95, 60, "No crash (OK)", Font::font8(), Color::Black, Color::White);
    std::cout << "✓ Zero-size shapes handled gracefully.\n\n";

    // Test 6: Empty bitmap
    std::cout << "=== Test 6: Empty Bitmap ===\n";
    display->draw_string(95, 75, "6. Empty bitmap:", Font::font12(), Color::Black, Color::White);

    std::cout << "  Drawing empty bitmap...\n";
    std::vector<Color> empty_bitmap;
    display->draw_bitmap(95, 90, empty_bitmap, 0, 0);

    display->draw_string(95, 90, "No crash (OK)", Font::font8(), Color::Black, Color::White);
    std::cout << "✓ Empty bitmap handled gracefully.\n\n";

    // Test 7: Overlapping operations
    std::cout << "=== Test 7: Overlapping Operations ===\n";
    display->draw_string(95, 105, "7. Overlaps:", Font::font12(), Color::Black, Color::White);

    std::cout << "  Drawing overlapping shapes...\n";
    display->draw_rectangle(95, 120, 130, 145, Color::Black, DotPixel::Pixel1x1, DrawFill::Full);
    display->draw_circle(110, 132, 10, Color::White, DotPixel::Pixel1x1, DrawFill::Full);
    display->draw_line(95, 120, 130, 145, Color::Black, DotPixel::Pixel2x2, LineStyle::Solid);

    display->draw_string(95, 150, "Overlap OK", Font::font8(), Color::Black, Color::White);
    std::cout << "✓ Overlapping operations work correctly.\n\n";

    // Test 8: Inverted rectangles
    std::cout << "=== Test 8: Inverted Coordinates ===\n";
    display->draw_string(5, 165, "8. Inverted coords:", Font::font12(), Color::Black, Color::White);

    std::cout << "  Drawing with start > end...\n";
    // Rectangle with inverted coordinates (end before start)
    display->draw_rectangle(60, 200, 20, 180, Color::Black, DotPixel::Pixel1x1, DrawFill::Empty);

    // Line with inverted coordinates
    display->draw_line(100, 200, 70, 180, Color::Black, DotPixel::Pixel1x1, LineStyle::Solid);

    display->draw_string(5, 210, "Handled OK", Font::font8(), Color::Black, Color::White);
    std::cout << "✓ Inverted coordinates handled correctly.\n\n";

    // Test 9: Special characters
    std::cout << "=== Test 9: Special Characters ===\n";
    display->draw_string(95, 165, "9. Special chars:", Font::font12(), Color::Black, Color::White);

    std::cout << "  Drawing special characters...\n";
    display->draw_string(95, 180, "!@#$%^&*()", Font::font8(), Color::Black, Color::White);
    display->draw_string(95, 190, "[]{}|\\<>?/", Font::font8(), Color::Black, Color::White);
    display->draw_string(95, 200, "~`-_=+:;\"',.", Font::font8(), Color::Black, Color::White);

    display->draw_string(95, 210, "Chars OK", Font::font8(), Color::Black, Color::White);
    std::cout << "✓ Special characters handled correctly.\n\n";

    // Summary
    std::cout << "=== All Edge Cases Tested ===\n";
    display->draw_rectangle(3, 223, 173, 260, Color::Black, DotPixel::Pixel1x1, DrawFill::Empty);
    display->draw_string(10, 230, "All edge cases handled:", Font::font8(), Color::Black, Color::White);
    display->draw_string(10, 240, "No crashes or exceptions!", Font::font8(), Color::Black, Color::White);
    display->draw_string(10, 250, "System is robust.", Font::font8(), Color::Black, Color::White);

    // Refresh display
    std::cout << "Refreshing display...\n";
    if (auto result = display->refresh(); !result) {
      std::cerr << "Refresh failed: " << result.error().what() << "\n";
      return EXIT_FAILURE;
    }

    std::cout << "\n=======================================\n";
    std::cout << "  Edge Cases Test: PASSED\n";
    std::cout << "=======================================\n";
    std::cout << "\nVisual verification checklist:\n";
    std::cout << "  [ ] No crashes occurred\n";
    std::cout << "  [ ] Out-of-bounds operations were silently clipped\n";
    std::cout << "  [ ] Boundary coordinates drawn correctly (4 corners marked)\n";
    std::cout << "  [ ] Empty strings handled gracefully\n";
    std::cout << "  [ ] Large numbers displayed (may be clipped)\n";
    std::cout << "  [ ] Zero-size shapes didn't crash\n";
    std::cout << "  [ ] Empty bitmap didn't crash\n";
    std::cout << "  [ ] Overlapping shapes displayed correctly\n";
    std::cout << "  [ ] Inverted coordinates handled\n";
    std::cout << "  [ ] Special characters displayed\n";
    std::cout << "  [ ] All test labels visible\n";
    std::cout << "  [ ] Summary box displayed at bottom\n";

    return EXIT_SUCCESS;

  } catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
    return EXIT_FAILURE;
  }
}
