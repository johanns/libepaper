#include <epaper/device.hpp>
#include <epaper/display.hpp>
#include <epaper/drivers/epd27.hpp>
#include <epaper/font.hpp>
#include <cstdlib>
#include <iostream>

using namespace epaper;

auto main() -> int {
  std::cout << "=======================================\n";
  std::cout << "  Drawing Primitives Test\n";
  std::cout << "=======================================\n";
  std::cout << "This test exercises all drawing operations.\n\n";

  try {
    // Initialize device
    std::cout << "Initializing device...\n";
    Device device;
    if (auto result = device.init(); !result) {
      std::cerr << "Device initialization failed: " << result.error().what() << "\n";
      return EXIT_FAILURE;
    }

    // Create display in landscape mode for more space
    auto display = create_display<EPD27>(device, DisplayMode::BlackWhite, Orientation::Landscape90);
    if (!display) {
      std::cerr << "Display initialization failed: " << display.error().what() << "\n";
      return EXIT_FAILURE;
    }

    std::cout << "Display size: " << display->effective_width() << "x" << display->effective_height() << "\n";

    display->clear(Color::White);

    // Title
    display->draw_string(5, 2, "DRAWING PRIMITIVES", Font::font12(), Color::Black, Color::White);

    // Test Points - different DotPixel sizes
    std::cout << "Drawing points...\n";
    display->draw_string(5, 18, "Points:", Font::font8(), Color::Black, Color::White);
    display->draw_point(5, 28, Color::Black, DotPixel::Pixel1x1);
    display->draw_point(12, 28, Color::Black, DotPixel::Pixel2x2);
    display->draw_point(21, 28, Color::Black, DotPixel::Pixel3x3);
    display->draw_point(32, 28, Color::Black, DotPixel::Pixel4x4);
    display->draw_point(45, 28, Color::Black, DotPixel::Pixel5x5);
    display->draw_point(60, 28, Color::Black, DotPixel::Pixel6x6);

    // Test Lines - horizontal, vertical, diagonal
    std::cout << "Drawing lines...\n";
    display->draw_string(5, 45, "Lines:", Font::font8(), Color::Black, Color::White);
    // Horizontal solid
    display->draw_line(5, 55, 50, 55, Color::Black, DotPixel::Pixel1x1, LineStyle::Solid);
    // Vertical solid
    display->draw_line(55, 55, 55, 75, Color::Black, DotPixel::Pixel1x1, LineStyle::Solid);
    // Diagonal
    display->draw_line(60, 55, 80, 75, Color::Black, DotPixel::Pixel1x1, LineStyle::Solid);
    // Dotted
    display->draw_line(85, 55, 110, 55, Color::Black, DotPixel::Pixel1x1, LineStyle::Dotted);
    // Thick line
    display->draw_line(115, 55, 140, 55, Color::Black, DotPixel::Pixel3x3, LineStyle::Solid);

    // Test Rectangles - empty and filled
    std::cout << "Drawing rectangles...\n";
    display->draw_string(5, 85, "Rectangles:", Font::font8(), Color::Black, Color::White);
    // Empty rectangle
    display->draw_rectangle(5, 95, 30, 115, Color::Black, DotPixel::Pixel1x1, DrawFill::Empty);
    // Filled rectangle
    display->draw_rectangle(35, 95, 60, 115, Color::Black, DotPixel::Pixel1x1, DrawFill::Full);
    // Thick border empty
    display->draw_rectangle(65, 95, 90, 115, Color::Black, DotPixel::Pixel2x2, DrawFill::Empty);
    // Small filled
    display->draw_rectangle(95, 95, 110, 115, Color::Black, DotPixel::Pixel1x1, DrawFill::Full);

    // Test Circles - empty and filled
    std::cout << "Drawing circles...\n";
    display->draw_string(150, 18, "Circles:", Font::font8(), Color::Black, Color::White);
    // Empty circle
    display->draw_circle(165, 40, 15, Color::Black, DotPixel::Pixel1x1, DrawFill::Empty);
    // Filled circle
    display->draw_circle(200, 40, 15, Color::Black, DotPixel::Pixel1x1, DrawFill::Full);
    // Small circle
    display->draw_circle(235, 40, 8, Color::Black, DotPixel::Pixel1x1, DrawFill::Empty);
    // Thick border circle
    display->draw_circle(165, 80, 12, Color::Black, DotPixel::Pixel2x2, DrawFill::Empty);

    // Test Text - different fonts
    std::cout << "Drawing text...\n";
    display->draw_string(5, 125, "Text Fonts:", Font::font8(), Color::Black, Color::White);
    display->draw_string(5, 135, "Font8", Font::font8(), Color::Black, Color::White);
    display->draw_string(45, 135, "Font12", Font::font12(), Color::Black, Color::White);
    display->draw_string(95, 135, "Font16", Font::font16(), Color::Black, Color::White);

    // Test Numbers - positive, negative, zero
    std::cout << "Drawing numbers...\n";
    display->draw_string(5, 155, "Numbers:", Font::font8(), Color::Black, Color::White);
    display->draw_number(5, 165, 0, Font::font12(), Color::Black, Color::White);
    display->draw_number(25, 165, 42, Font::font12(), Color::Black, Color::White);
    display->draw_number(55, 165, -123, Font::font12(), Color::Black, Color::White);
    display->draw_number(100, 165, 999999, Font::font8(), Color::Black, Color::White);

    // Test Decimals - various precisions
    std::cout << "Drawing decimals...\n";
    display->draw_string(150, 85, "Decimals:", Font::font8(), Color::Black, Color::White);
    display->draw_decimal(150, 95, 3.14159, 2, Font::font12(), Color::Black, Color::White);
    display->draw_decimal(150, 110, -2.5, 1, Font::font12(), Color::Black, Color::White);
    display->draw_decimal(150, 125, 0.001, 3, Font::font8(), Color::Black, Color::White);
    display->draw_decimal(150, 138, 99.99, 2, Font::font12(), Color::Black, Color::White);

    // Complex shape combination
    std::cout << "Drawing complex pattern...\n";
    display->draw_string(150, 155, "Pattern:", Font::font8(), Color::Black, Color::White);
    display->draw_circle(185, 168, 8, Color::Black, DotPixel::Pixel1x1, DrawFill::Empty);
    display->draw_rectangle(175, 165, 180, 170, Color::Black, DotPixel::Pixel1x1, DrawFill::Full);
    display->draw_rectangle(190, 165, 195, 170, Color::Black, DotPixel::Pixel1x1, DrawFill::Full);
    display->draw_line(180, 173, 190, 173, Color::Black, DotPixel::Pixel2x2, LineStyle::Solid);

    // Refresh display
    std::cout << "Refreshing display...\n";
    if (auto result = display->refresh(); !result) {
      std::cerr << "Refresh failed: " << result.error().what() << "\n";
      return EXIT_FAILURE;
    }

    std::cout << "\n=======================================\n";
    std::cout << "  Drawing Primitives Test: PASSED\n";
    std::cout << "=======================================\n";
    std::cout << "\nVisual verification checklist:\n";
    std::cout << "  [ ] Points increase in size from 1x1 to 6x6\n";
    std::cout << "  [ ] Lines show horizontal, vertical, diagonal, dotted, and thick\n";
    std::cout << "  [ ] Rectangles show empty and filled variants\n";
    std::cout << "  [ ] Circles show empty and filled variants\n";
    std::cout << "  [ ] Text renders in Font8, Font12, and Font16\n";
    std::cout << "  [ ] Numbers display correctly (0, 42, -123, 999999)\n";
    std::cout << "  [ ] Decimals display with correct precision\n";
    std::cout << "  [ ] Complex pattern (smiley face) is recognizable\n";

    return EXIT_SUCCESS;

  } catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
    return EXIT_FAILURE;
  }
}
