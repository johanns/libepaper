#include <chrono>
#include <cstdlib>
#include <epaper/device.hpp>
#include <epaper/display.hpp>
#include <epaper/drivers/epd27.hpp>
#include <epaper/font.hpp>
#include <iostream>
#include <thread>

using namespace epaper;

auto test_blackwhite_mode(Device &device) -> bool {
  std::cout << "\n=== Testing Black & White Mode ===\n";

  auto display = create_display<EPD27>(device, DisplayMode::BlackWhite);
  if (!display) {
    std::cerr << "Failed to create B/W display: " << display.error().what() << "\n";
    return false;
  }

  std::cout << "Display mode: BlackWhite\n";
  std::cout << "Dimensions: " << display->width() << "x" << display->height() << " pixels\n";

  // Clear to white
  display->clear(Color::White);

  // Title
  display->draw_string(10, 5, "BLACK & WHITE MODE", Font::font16(), Color::Black, Color::White);

  // Test pure black
  display->draw_rectangle(10, 30, 80, 80, Color::Black, DotPixel::Pixel1x1, DrawFill::Full);
  display->draw_string(10, 85, "Black", Font::font12(), Color::Black, Color::White);

  // Test pure white
  display->draw_rectangle(90, 30, 160, 80, Color::Black, DotPixel::Pixel1x1, DrawFill::Empty);
  display->draw_rectangle(92, 32, 158, 78, Color::White, DotPixel::Pixel1x1, DrawFill::Full);
  display->draw_string(100, 85, "White", Font::font12(), Color::Black, Color::White);

  // Test patterns
  display->draw_string(10, 105, "Patterns:", Font::font12(), Color::Black, Color::White);

  // Checkerboard pattern
  for (std::size_t y = 0; y < 40; y += 4) {
    for (std::size_t x = 0; x < 80; x += 4) {
      const bool is_black = ((x / 4) + (y / 4)) % 2 == 0;
      display->draw_rectangle(10 + x, 120 + y, 13 + x, 123 + y, is_black ? Color::Black : Color::White,
                              DotPixel::Pixel1x1, DrawFill::Full);
    }
  }
  display->draw_string(10, 165, "Checkerboard", Font::font8(), Color::Black, Color::White);

  // Line patterns
  for (std::size_t i = 0; i < 40; i += 2) {
    display->draw_line(100, 120 + i, 160, 120 + i, Color::Black, DotPixel::Pixel1x1, LineStyle::Solid);
  }
  display->draw_string(100, 165, "Lines", Font::font8(), Color::Black, Color::White);

  // Test text rendering
  display->draw_string(10, 180, "Text Test:", Font::font12(), Color::Black, Color::White);
  display->draw_string(10, 195, "ABCDEFGHIJKLM", Font::font12(), Color::Black, Color::White);
  display->draw_string(10, 210, "0123456789", Font::font12(), Color::Black, Color::White);

  // Refresh display
  std::cout << "Refreshing display...\n";
  if (auto result = display->refresh(); !result) {
    std::cerr << "Refresh failed: " << result.error().what() << "\n";
    return false;
  }

  std::cout << "Black & White mode test complete.\n";
  std::cout << "Visual verification: Check that black is pure black and white is pure white.\n";

  return true;
}

auto test_grayscale_mode(Device &device) -> bool {
  std::cout << "\n=== Testing 4-Level Grayscale Mode ===\n";

  auto display = create_display<EPD27>(device, DisplayMode::Grayscale4);
  if (!display) {
    std::cerr << "Failed to create grayscale display: " << display.error().what() << "\n";
    return false;
  }

  std::cout << "Display mode: Grayscale4\n";
  std::cout << "Dimensions: " << display->width() << "x" << display->height() << " pixels\n";

  // Clear to white
  display->clear(Color::White);

  // Title
  display->draw_string(10, 5, "GRAYSCALE MODE", Font::font16(), Color::Black, Color::White);

  // Display all 4 gray levels
  const std::size_t box_width = 35;
  const std::size_t box_height = 50;
  const std::size_t start_y = 30;

  // White
  display->draw_rectangle(10, start_y, 10 + box_width, start_y + box_height, Color::Black, DotPixel::Pixel1x1,
                          DrawFill::Empty);
  display->draw_rectangle(12, start_y + 2, 10 + box_width - 2, start_y + box_height - 2, Color::White,
                          DotPixel::Pixel1x1, DrawFill::Full);
  display->draw_string(13, start_y + box_height + 5, "White", Font::font8(), Color::Black, Color::White);

  // Gray1 (lighter)
  display->draw_rectangle(50, start_y, 50 + box_width, start_y + box_height, Color::Gray1, DotPixel::Pixel1x1,
                          DrawFill::Full);
  display->draw_string(53, start_y + box_height + 5, "Gray1", Font::font8(), Color::Black, Color::White);

  // Gray2 (darker)
  display->draw_rectangle(90, start_y, 90 + box_width, start_y + box_height, Color::Gray2, DotPixel::Pixel1x1,
                          DrawFill::Full);
  display->draw_string(93, start_y + box_height + 5, "Gray2", Font::font8(), Color::Black, Color::White);

  // Black
  display->draw_rectangle(130, start_y, 130 + box_width, start_y + box_height, Color::Black, DotPixel::Pixel1x1,
                          DrawFill::Full);
  display->draw_string(133, start_y + box_height + 5, "Black", Font::font8(), Color::Black, Color::White);

  // Gradient demonstration
  display->draw_string(10, 100, "Gradient:", Font::font12(), Color::Black, Color::White);

  const std::size_t gradient_width = 160;
  const std::size_t gradient_height = 30;
  const std::size_t segment_width = gradient_width / 4;

  for (std::size_t i = 0; i < 4; ++i) {
    Color color;
    switch (i) {
    case 0:
      color = Color::White;
      break;
    case 1:
      color = Color::Gray1;
      break;
    case 2:
      color = Color::Gray2;
      break;
    case 3:
      color = Color::Black;
      break;
    }

    display->draw_rectangle(10 + i * segment_width, 115, 10 + (i + 1) * segment_width, 115 + gradient_height, color,
                            DotPixel::Pixel1x1, DrawFill::Full);
  }

  // Test text in different colors
  display->draw_string(10, 155, "Text Test:", Font::font12(), Color::Black, Color::White);
  display->draw_string(10, 170, "Black on White", Font::font12(), Color::Black, Color::White);
  display->draw_string(10, 185, "Gray1 on White", Font::font12(), Color::Gray1, Color::White);
  display->draw_string(10, 200, "Gray2 on White", Font::font12(), Color::Gray2, Color::White);
  display->draw_string(10, 215, "White on Black", Font::font12(), Color::White, Color::Black);

  // Refresh display
  std::cout << "Refreshing display...\n";
  if (auto result = display->refresh(); !result) {
    std::cerr << "Refresh failed: " << result.error().what() << "\n";
    return false;
  }

  std::cout << "Grayscale mode test complete.\n";
  std::cout << "Visual verification: Check that 4 distinct gray levels are visible.\n";
  std::cout << "  - White (lightest)\n";
  std::cout << "  - Gray1 (light gray)\n";
  std::cout << "  - Gray2 (dark gray)\n";
  std::cout << "  - Black (darkest)\n";

  return true;
}

auto main() -> int {
  std::cout << "=======================================\n";
  std::cout << "  Display Modes Test\n";
  std::cout << "=======================================\n";
  std::cout << "This test verifies both BlackWhite and Grayscale4 display modes.\n\n";

  try {
    // Initialize device
    std::cout << "Initializing device...\n";
    Device device;
    if (auto result = device.init(); !result) {
      std::cerr << "Device initialization failed: " << result.error().what() << "\n";
      return EXIT_FAILURE;
    }
    std::cout << "Device initialized successfully.\n";

    // Test Black & White mode
    if (!test_blackwhite_mode(device)) {
      return EXIT_FAILURE;
    }

    // Wait between tests
    std::cout << "\nWaiting 3 seconds before next test...\n";
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // Test Grayscale mode
    if (!test_grayscale_mode(device)) {
      return EXIT_FAILURE;
    }

    std::cout << "\n=======================================\n";
    std::cout << "  Display Modes Test: PASSED\n";
    std::cout << "=======================================\n";

    return EXIT_SUCCESS;

  } catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
    return EXIT_FAILURE;
  }
}
