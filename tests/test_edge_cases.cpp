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
    display->draw(display->text("EDGE CASES TEST")
                      .at(5, 5)
                      .font(&Font::font16())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());

    // Test 1: Out-of-bounds coordinates
    std::cout << "=== Test 1: Out-of-Bounds Coordinates ===\n";
    display->draw(display->text("1. Out-of-bounds:")
                      .at(5, 25)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());

    // Far out of bounds (should be silently clipped)
    std::cout << "  Drawing far out of bounds...\n";
    display->set_pixel(width + 1000, height + 1000, Color::Black);
    display->draw(display->point().at(width + 100, height + 100).color(Color::Black).size(DotPixel::Pixel5x5).build());
    display->draw(
        display->line().from(width + 50, height + 50).to(width + 100, height + 100).color(Color::Black).build());

    // Negative-like (wrapped around due to std::size_t)
    std::cout << "  Drawing with wrapped coordinates...\n";
    constexpr std::size_t very_large = std::numeric_limits<std::size_t>::max() - 10;
    display->set_pixel(very_large, very_large, Color::Black);
    display->draw(display->point().at(very_large, very_large).color(Color::Black).size(DotPixel::Pixel3x3).build());

    display->draw(display->text("No crash (OK)")
                      .at(5, 40)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    std::cout << "✓ Out-of-bounds coordinates handled gracefully.\n\n";

    // Test 2: Boundary coordinates
    std::cout << "=== Test 2: Boundary Coordinates ===\n";
    display->draw(display->text("2. Boundaries:")
                      .at(5, 55)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());

    std::cout << "  Drawing at exact boundaries...\n";
    // Draw at all four corners
    display->draw(display->point().at(0, 0).color(Color::Black).size(DotPixel::Pixel2x2).build());
    display->draw(display->point().at(width - 1, 0).color(Color::Black).size(DotPixel::Pixel2x2).build());
    display->draw(display->point().at(0, height - 1).color(Color::Black).size(DotPixel::Pixel2x2).build());
    display->draw(display->point().at(width - 1, height - 1).color(Color::Black).size(DotPixel::Pixel2x2).build());

    // Draw lines along edges
    display->draw(display->line().from(0, 0).to(width - 1, 0).color(Color::Black).build());
    display->draw(display->line().from(0, 0).to(0, height - 1).color(Color::Black).build());

    display->draw(display->text("Corners marked")
                      .at(5, 70)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    std::cout << "✓ Boundary coordinates handled correctly.\n\n";

    // Test 3: Empty strings
    std::cout << "=== Test 3: Empty Strings ===\n";
    display->draw(display->text("3. Empty strings:")
                      .at(5, 85)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());

    std::cout << "  Drawing empty string...\n";
    display->draw(
        display->text("").at(5, 100).font(&Font::font12()).foreground(Color::Black).background(Color::White).build());

    display->draw(display->text("No crash (OK)")
                      .at(5, 100)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    std::cout << "✓ Empty string handled gracefully.\n\n";

    // Test 4: Very large numbers
    std::cout << "=== Test 4: Large Numbers ===\n";
    display->draw(display->text("4. Large numbers:")
                      .at(5, 115)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());

    std::cout << "  Drawing very large numbers...\n";
    display->draw(display->text()
                      .number(std::numeric_limits<std::int32_t>::max())
                      .at(5, 130)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text()
                      .number(std::numeric_limits<std::int32_t>::min())
                      .at(5, 140)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text()
                      .number(0)
                      .at(5, 150)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());

    std::cout << "✓ Large numbers handled correctly.\n\n";

    // Test 5: Zero-size shapes
    std::cout << "=== Test 5: Zero-Size Shapes ===\n";
    display->draw(display->text("5. Zero sizes:")
                      .at(95, 25)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());

    std::cout << "  Drawing zero-radius circle...\n";
    display->draw(display->circle()
                      .center(120, 45)
                      .radius(0)
                      .color(Color::Black)
                      .border_width(DotPixel::Pixel1x1)
                      .fill(DrawFill::Full)
                      .build());

    std::cout << "  Drawing zero-width rectangle...\n";
    display->draw(display->rectangle()
                      .top_left(130, 40)
                      .bottom_right(130, 50)
                      .color(Color::Black)
                      .border_width(DotPixel::Pixel1x1)
                      .fill(DrawFill::Full)
                      .build());

    std::cout << "  Drawing zero-length line...\n";
    display->draw(display->line()
                      .from(140, 45)
                      .to(140, 45)
                      .color(Color::Black)
                      .width(DotPixel::Pixel1x1)
                      .style(LineStyle::Solid)
                      .build());

    display->draw(display->text("No crash (OK)")
                      .at(95, 60)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    std::cout << "✓ Zero-size shapes handled gracefully.\n\n";

    // Test 6: Empty bitmap
    std::cout << "=== Test 6: Empty Bitmap ===\n";
    display->draw(display->text("6. Empty bitmap:")
                      .at(95, 75)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());

    std::cout << "  Drawing empty bitmap...\n";
    std::vector<Color> empty_bitmap;
    display->draw_bitmap(95, 90, empty_bitmap, 0, 0);

    display->draw(display->text("No crash (OK)")
                      .at(95, 90)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    std::cout << "✓ Empty bitmap handled gracefully.\n\n";

    // Test 7: Overlapping operations
    std::cout << "=== Test 7: Overlapping Operations ===\n";
    display->draw(display->text("7. Overlaps:")
                      .at(95, 105)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());

    std::cout << "  Drawing overlapping shapes...\n";
    display->draw(display->rectangle()
                      .top_left(95, 120)
                      .bottom_right(130, 145)
                      .color(Color::Black)
                      .border_width(DotPixel::Pixel1x1)
                      .fill(DrawFill::Full)
                      .build());
    display->draw(display->circle()
                      .center(110, 132)
                      .radius(10)
                      .color(Color::White)
                      .border_width(DotPixel::Pixel1x1)
                      .fill(DrawFill::Full)
                      .build());
    display->draw(display->line()
                      .from(95, 120)
                      .to(130, 145)
                      .color(Color::Black)
                      .width(DotPixel::Pixel2x2)
                      .style(LineStyle::Solid)
                      .build());

    display->draw(display->text("Overlap OK")
                      .at(95, 150)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    std::cout << "✓ Overlapping operations work correctly.\n\n";

    // Test 8: Inverted rectangles
    std::cout << "=== Test 8: Inverted Coordinates ===\n";
    display->draw(display->text("8. Inverted coords:")
                      .at(5, 165)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());

    std::cout << "  Drawing with start > end...\n";
    // Rectangle with inverted coordinates (end before start)
    display->draw(display->rectangle()
                      .top_left(60, 200)
                      .bottom_right(20, 180)
                      .color(Color::Black)
                      .border_width(DotPixel::Pixel1x1)
                      .fill(DrawFill::Empty)
                      .build());

    // Line with inverted coordinates
    display->draw(display->line()
                      .from(100, 200)
                      .to(70, 180)
                      .color(Color::Black)
                      .width(DotPixel::Pixel1x1)
                      .style(LineStyle::Solid)
                      .build());

    display->draw(display->text("Handled OK")
                      .at(5, 210)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    std::cout << "✓ Inverted coordinates handled correctly.\n\n";

    // Test 9: Special characters
    std::cout << "=== Test 9: Special Characters ===\n";
    display->draw(display->text("9. Special chars:")
                      .at(95, 165)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());

    std::cout << "  Drawing special characters...\n";
    display->draw(display->text("!@#$%^&*()")
                      .at(95, 180)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text("[]{}|\\<>?/")
                      .at(95, 190)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text("~`-_=+:;\"\047,.")
                      .at(95, 200)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());

    display->draw(display->text("Chars OK")
                      .at(95, 210)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    std::cout << "✓ Special characters handled correctly.\n\n";

    // Summary
    std::cout << "=== All Edge Cases Tested ===\n";
    display->draw(display->rectangle()
                      .top_left(3, 223)
                      .bottom_right(173, 260)
                      .color(Color::Black)
                      .border_width(DotPixel::Pixel1x1)
                      .fill(DrawFill::Empty)
                      .build());
    display->draw(display->text("All edge cases handled:")
                      .at(10, 230)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text("No crashes or exceptions!")
                      .at(10, 240)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text("System is robust.")
                      .at(10, 250)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());

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
