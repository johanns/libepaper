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
  display->draw(display->text("BLACK & WHITE MODE")
                    .at(10, 5)
                    .font(&Font::font16())
                    .foreground(Color::Black)
                    .background(Color::White)
                    .build());

  // Test pure black
  display->draw(display->rectangle()
                    .top_left(10, 30)
                    .bottom_right(80, 80)
                    .color(Color::Black)
                    .border_width(DotPixel::Pixel1x1)
                    .fill(DrawFill::Full)
                    .build());
  display->draw(display->text("Black")
                    .at(10, 85)
                    .font(&Font::font12())
                    .foreground(Color::Black)
                    .background(Color::White)
                    .build());

  // Test pure white
  display->draw(display->rectangle()
                    .top_left(90, 30)
                    .bottom_right(160, 80)
                    .color(Color::Black)
                    .border_width(DotPixel::Pixel1x1)
                    .fill(DrawFill::Empty)
                    .build());
  display->draw(display->rectangle()
                    .top_left(92, 32)
                    .bottom_right(158, 78)
                    .color(Color::White)
                    .border_width(DotPixel::Pixel1x1)
                    .fill(DrawFill::Full)
                    .build());
  display->draw(display->text("White")
                    .at(100, 85)
                    .font(&Font::font12())
                    .foreground(Color::Black)
                    .background(Color::White)
                    .build());

  // Test patterns
  display->draw(display->text("Patterns:")
                    .at(10, 105)
                    .font(&Font::font12())
                    .foreground(Color::Black)
                    .background(Color::White)
                    .build());

  // Checkerboard pattern
  for (std::size_t y = 0; y < 40; y += 4) {
    for (std::size_t x = 0; x < 80; x += 4) {
      const bool is_black = ((x / 4) + (y / 4)) % 2 == 0;
      display->draw(display->rectangle()
                        .top_left(10 + x, 120 + y)
                        .bottom_right(13 + x, 123 + y)
                        .color(is_black ? Color::Black : Color::White)
                        .border_width(DotPixel::Pixel1x1)
                        .fill(DrawFill::Full)
                        .build());
    }
  }
  display->draw(display->text("Checkerboard")
                    .at(10, 165)
                    .font(&Font::font8())
                    .foreground(Color::Black)
                    .background(Color::White)
                    .build());

  // Line patterns
  for (std::size_t i = 0; i < 40; i += 2) {
    display->draw(display->line()
                      .from(100, 120 + i)
                      .to(160, 120 + i)
                      .color(Color::Black)
                      .width(DotPixel::Pixel1x1)
                      .style(LineStyle::Solid)
                      .build());
  }
  display->draw(display->text("Lines")
                    .at(100, 165)
                    .font(&Font::font8())
                    .foreground(Color::Black)
                    .background(Color::White)
                    .build());

  // Test text rendering
  display->draw(display->text("Text Test:")
                    .at(10, 180)
                    .font(&Font::font12())
                    .foreground(Color::Black)
                    .background(Color::White)
                    .build());
  display->draw(display->text("ABCDEFGHIJKLM")
                    .at(10, 195)
                    .font(&Font::font12())
                    .foreground(Color::Black)
                    .background(Color::White)
                    .build());
  display->draw(display->text("0123456789")
                    .at(10, 210)
                    .font(&Font::font12())
                    .foreground(Color::Black)
                    .background(Color::White)
                    .build());

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
  display->draw(display->text("GRAYSCALE MODE")
                    .at(10, 5)
                    .font(&Font::font16())
                    .foreground(Color::Black)
                    .background(Color::White)
                    .build());

  // Display all 4 gray levels
  const std::size_t box_width = 35;
  const std::size_t box_height = 50;
  const std::size_t start_y = 30;

  // White
  display->draw(display->rectangle()
                    .top_left(10, start_y)
                    .bottom_right(10 + box_width, start_y + box_height)
                    .color(Color::Black)
                    .border_width(DotPixel::Pixel1x1)
                    .fill(DrawFill::Empty)
                    .build());
  display->draw(display->rectangle()
                    .top_left(12, start_y + 2)
                    .bottom_right(10 + box_width - 2, start_y + box_height - 2)
                    .color(Color::White)
                    .border_width(DotPixel::Pixel1x1)
                    .fill(DrawFill::Full)
                    .build());
  display->draw(display->text("White")
                    .at(13, start_y + box_height + 5)
                    .font(&Font::font8())
                    .foreground(Color::Black)
                    .background(Color::White)
                    .build());

  // Gray1 (lighter)
  display->draw(display->rectangle()
                    .top_left(50, start_y)
                    .bottom_right(50 + box_width, start_y + box_height)
                    .color(Color::Gray1)
                    .border_width(DotPixel::Pixel1x1)
                    .fill(DrawFill::Full)
                    .build());
  display->draw(display->text("Gray1")
                    .at(53, start_y + box_height + 5)
                    .font(&Font::font8())
                    .foreground(Color::Black)
                    .background(Color::White)
                    .build());

  // Gray2 (darker)
  display->draw(display->rectangle()
                    .top_left(90, start_y)
                    .bottom_right(90 + box_width, start_y + box_height)
                    .color(Color::Gray2)
                    .border_width(DotPixel::Pixel1x1)
                    .fill(DrawFill::Full)
                    .build());
  display->draw(display->text("Gray2")
                    .at(93, start_y + box_height + 5)
                    .font(&Font::font8())
                    .foreground(Color::Black)
                    .background(Color::White)
                    .build());

  // Black
  display->draw(display->rectangle()
                    .top_left(130, start_y)
                    .bottom_right(130 + box_width, start_y + box_height)
                    .color(Color::Black)
                    .border_width(DotPixel::Pixel1x1)
                    .fill(DrawFill::Full)
                    .build());
  display->draw(display->text("Black")
                    .at(133, start_y + box_height + 5)
                    .font(&Font::font8())
                    .foreground(Color::Black)
                    .background(Color::White)
                    .build());

  // Gradient demonstration
  display->draw(display->text("Gradient:")
                    .at(10, 100)
                    .font(&Font::font12())
                    .foreground(Color::Black)
                    .background(Color::White)
                    .build());

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

    display->draw(display->rectangle()
                      .top_left(10 + i * segment_width, 115)
                      .bottom_right(10 + (i + 1) * segment_width, 115 + gradient_height)
                      .color(color)
                      .border_width(DotPixel::Pixel1x1)
                      .fill(DrawFill::Full)
                      .build());
  }

  // Test text in different colors
  display->draw(display->text("Text Test:")
                    .at(10, 155)
                    .font(&Font::font12())
                    .foreground(Color::Black)
                    .background(Color::White)
                    .build());
  display->draw(display->text("Black on White")
                    .at(10, 170)
                    .font(&Font::font12())
                    .foreground(Color::Black)
                    .background(Color::White)
                    .build());
  display->draw(display->text("Gray1 on White")
                    .at(10, 185)
                    .font(&Font::font12())
                    .foreground(Color::Gray1)
                    .background(Color::White)
                    .build());
  display->draw(display->text("Gray2 on White")
                    .at(10, 200)
                    .font(&Font::font12())
                    .foreground(Color::Gray2)
                    .background(Color::White)
                    .build());
  display->draw(display->text("White on Black")
                    .at(10, 215)
                    .font(&Font::font12())
                    .foreground(Color::White)
                    .background(Color::Black)
                    .build());

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
