#include "test_config.hpp"
#include <cstdlib>
#include <epaper/core/device.hpp>
#include <epaper/core/display.hpp>
#include <epaper/graphics/font.hpp>
#include <iostream>
#include <vector>

using namespace epaper;

auto main() -> int {
  std::cout << "=======================================\n";
  std::cout << "  Bitmap Operations Test\n";
  std::cout << "=======================================\n";
  std::cout << "This test exercises bitmap drawing operations.\n\n";

  try {
    // Initialize device
    std::cout << "Initializing device...\n";
    Device device;
    if (auto result = device.init(); !result) {
      std::cerr << "Device initialization failed: " << result.error().what() << "\n";
      return EXIT_FAILURE;
    }

    // Create display
    auto display = create_display<TestDriver, MonoFramebuffer>(device, DisplayMode::BlackWhite);
    if (!display) {
      std::cerr << "Display initialization failed: " << display.error().what() << "\n";
      return EXIT_FAILURE;
    }

    std::cout << "Display size: " << display->effective_width() << "x" << display->effective_height() << "\n";

    display->clear(Color::White);

    // Title
    display->draw(display->text("BITMAP TEST")
                      .at(5, 5)
                      .font(&Font::font16())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());

    // Test 1: Simple checkerboard pattern from memory
    std::cout << "Creating checkerboard bitmap from memory...\n";
    display->draw(display->text("Checkerboard (32x32):")
                      .at(5, 25)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());

    std::vector<Color> checkerboard;
    const std::size_t checker_size = 32;
    for (std::size_t y = 0; y < checker_size; ++y) {
      for (std::size_t x = 0; x < checker_size; ++x) {
        const bool is_white = ((x / 4) + (y / 4)) % 2 == 0;
        checkerboard.push_back(is_white ? Color::White : Color::Black);
      }
    }
    display->draw_bitmap(5, 40, checkerboard, checker_size, checker_size);

    // Test 2: Gradient pattern
    std::cout << "Creating gradient bitmap from memory...\n";
    display->draw(display->text("Gradient (32x32):")
                      .at(45, 25)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());

    std::vector<Color> gradient;
    for (std::size_t y = 0; y < checker_size; ++y) {
      for (std::size_t x = 0; x < checker_size; ++x) {
        // Create simple vertical gradient (black to white)
        const bool is_white = y < 16;
        gradient.push_back(is_white ? Color::White : Color::Black);
      }
    }
    display->draw_bitmap(45, 40, gradient, checker_size, checker_size);

    // Test 3: Solid patterns
    std::cout << "Creating solid pattern bitmaps...\n";
    display->draw(display->text("Solid (16x16):")
                      .at(85, 25)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());

    // All black
    std::vector<Color> solid_black(16 * 16, Color::Black);
    display->draw_bitmap(85, 40, solid_black, 16, 16);

    // All white (with border)
    display->draw(display->rectangle()
                      .top_left(105, 40)
                      .bottom_right(121, 56)
                      .color(Color::Black)
                      .border_width(DotPixel::Pixel1x1)
                      .fill(DrawFill::Empty)
                      .build());
    std::vector<Color> solid_white(16 * 16, Color::White);
    display->draw_bitmap(106, 41, solid_white, 14, 14);

    // Test 4: Stripe pattern
    std::cout << "Creating stripe pattern bitmap...\n";
    display->draw(display->text("Stripes (64x32):")
                      .at(5, 80)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());

    std::vector<Color> stripes;
    const std::size_t stripe_width = 64;
    const std::size_t stripe_height = 32;
    for (std::size_t y = 0; y < stripe_height; ++y) {
      for (std::size_t x = 0; x < stripe_width; ++x) {
        const bool is_white = (x / 4) % 2 == 0;
        stripes.push_back(is_white ? Color::White : Color::Black);
      }
    }
    display->draw_bitmap(5, 95, stripes, stripe_width, stripe_height);

    // Test 5: Scaling test
    std::cout << "Testing bitmap scaling...\n";
    display->draw(display->text("Scaling:")
                      .at(5, 135)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());

    // Small checkerboard
    std::vector<Color> small_checker;
    const std::size_t small_size = 8;
    for (std::size_t y = 0; y < small_size; ++y) {
      for (std::size_t x = 0; x < small_size; ++x) {
        const bool is_white = ((x / 2) + (y / 2)) % 2 == 0;
        small_checker.push_back(is_white ? Color::White : Color::Black);
      }
    }

    // Draw original
    display->draw(
        display->text("8x8").at(5, 150).font(&Font::font8()).foreground(Color::Black).background(Color::White).build());
    display->draw_bitmap(5, 160, small_checker, small_size, small_size);

    // Draw scaled 2x
    display->draw(display->text("16x16")
                      .at(20, 150)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw_bitmap(20, 160, small_checker, small_size, small_size, 16, 16);

    // Draw scaled 4x
    display->draw(display->text("32x32")
                      .at(45, 150)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw_bitmap(45, 160, small_checker, small_size, small_size, 32, 32);

    // Test 6: Attempt to load image from file (if available)
    std::cout << "Testing bitmap from file...\n";
    display->draw(display->text("File Loading:")
                      .at(85, 80)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());

    // Try to load a test image (this may fail if no image exists)
    const char *test_image_path = "../examples/images/test.png";
    auto result = display->draw_bitmap_from_file(85, 95, test_image_path, 50, 50);
    if (result) {
      display->draw(display->text("PNG: OK")
                        .at(85, 150)
                        .font(&Font::font8())
                        .foreground(Color::Black)
                        .background(Color::White)
                        .build());
      std::cout << "Successfully loaded test image.\n";
    } else {
      display->draw(display->text("No test image")
                        .at(85, 95)
                        .font(&Font::font8())
                        .foreground(Color::Black)
                        .background(Color::White)
                        .build());
      display->draw(display->text("available.")
                        .at(85, 105)
                        .font(&Font::font8())
                        .foreground(Color::Black)
                        .background(Color::White)
                        .build());
      display->draw(display->text("(Expected)")
                        .at(85, 115)
                        .font(&Font::font8())
                        .foreground(Color::Black)
                        .background(Color::White)
                        .build());
      std::cout << "Note: Test image not found (this is OK for this test).\n";
    }

    // Test 7: Boundary clipping
    std::cout << "Testing boundary clipping...\n";
    display->draw(display->text("Clipping test:")
                      .at(5, 200)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());

    // Draw bitmap that extends beyond display bounds
    std::vector<Color> large_pattern;
    const std::size_t large_size = 40;
    for (std::size_t y = 0; y < large_size; ++y) {
      for (std::size_t x = 0; x < large_size; ++x) {
        const bool is_white = ((x / 8) + (y / 8)) % 2 == 0;
        large_pattern.push_back(is_white ? Color::White : Color::Black);
      }
    }

    // Draw partially off-screen (should clip gracefully)
    const auto display_width = display->effective_width();
    const auto display_height = display->effective_height();
    display->draw_bitmap(display_width - 20, display_height - 20, large_pattern, large_size, large_size);

    display->draw(display->text("(Bottom-right corner clipped)")
                      .at(5, 215)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());

    // Refresh display
    std::cout << "Refreshing display...\n";
    if (auto refresh_result = display->refresh(); !refresh_result) {
      std::cerr << "Refresh failed: " << refresh_result.error().what() << "\n";
      return EXIT_FAILURE;
    }

    std::cout << "\n=======================================\n";
    std::cout << "  Bitmap Operations Test: PASSED\n";
    std::cout << "=======================================\n";
    std::cout << "\nVisual verification checklist:\n";
    std::cout << "  [ ] Checkerboard pattern displays correctly\n";
    std::cout << "  [ ] Gradient pattern displays correctly\n";
    std::cout << "  [ ] Solid patterns (black and white) display\n";
    std::cout << "  [ ] Stripe pattern displays correctly\n";
    std::cout << "  [ ] Scaling test shows 3 sizes (8x8, 16x16, 32x32)\n";
    std::cout << "  [ ] File loading status displayed\n";
    std::cout << "  [ ] Boundary clipping works (no crash)\n";
    std::cout << "  [ ] All bitmaps render without artifacts\n";

    return EXIT_SUCCESS;

  } catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
    return EXIT_FAILURE;
  }
}
