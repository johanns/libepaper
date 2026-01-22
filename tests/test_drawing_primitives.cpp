#include "test_config.hpp"
#include <cstdlib>
#include <epaper/core/device.hpp>
#include <epaper/core/display.hpp>
#include <epaper/graphics/font.hpp>
#include <iostream>
#include <numbers>

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
    auto display =
        create_display<TestDriver, MonoFramebuffer>(device, DisplayMode::BlackWhite, Orientation::Landscape90);
    if (!display) {
      std::cerr << "Display initialization failed: " << display.error().what() << "\n";
      return EXIT_FAILURE;
    }

    std::cout << "Display size: " << display->effective_width() << "x" << display->effective_height() << "\n";

    display->clear(Color::White);

    // Title
    display->draw(display->text("DRAWING PRIMITIVES")
                      .at(5, 2)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());

    // Test Points - different DotPixel sizes
    std::cout << "Drawing points...\n";
    display->draw(display->text("Points:")
                      .at(5, 18)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->point().at(5, 28).color(Color::Black).size(DotPixel::Pixel1x1).build());
    display->draw(display->point().at(12, 28).color(Color::Black).size(DotPixel::Pixel2x2).build());
    display->draw(display->point().at(21, 28).color(Color::Black).size(DotPixel::Pixel3x3).build());
    display->draw(display->point().at(32, 28).color(Color::Black).size(DotPixel::Pixel4x4).build());
    display->draw(display->point().at(45, 28).color(Color::Black).size(DotPixel::Pixel5x5).build());
    display->draw(display->point().at(60, 28).color(Color::Black).size(DotPixel::Pixel6x6).build());

    // Test Lines - horizontal, vertical, diagonal
    std::cout << "Drawing lines...\n";
    display->draw(display->text("Lines:")
                      .at(5, 45)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    // Horizontal solid
    display->draw(display->line()
                      .from(5, 55)
                      .to(50, 55)
                      .color(Color::Black)
                      .width(DotPixel::Pixel1x1)
                      .style(LineStyle::Solid)
                      .build());
    // Vertical solid
    display->draw(display->line()
                      .from(55, 55)
                      .to(55, 75)
                      .color(Color::Black)
                      .width(DotPixel::Pixel1x1)
                      .style(LineStyle::Solid)
                      .build());
    // Diagonal
    display->draw(display->line()
                      .from(60, 55)
                      .to(80, 75)
                      .color(Color::Black)
                      .width(DotPixel::Pixel1x1)
                      .style(LineStyle::Solid)
                      .build());
    // Dotted
    display->draw(display->line()
                      .from(85, 55)
                      .to(110, 55)
                      .color(Color::Black)
                      .width(DotPixel::Pixel1x1)
                      .style(LineStyle::Dotted)
                      .build());
    // Thick line
    display->draw(display->line()
                      .from(115, 55)
                      .to(140, 55)
                      .color(Color::Black)
                      .width(DotPixel::Pixel3x3)
                      .style(LineStyle::Solid)
                      .build());

    // Test Rectangles - empty and filled
    std::cout << "Drawing rectangles...\n";
    display->draw(display->text("Rectangles:")
                      .at(5, 85)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    // Empty rectangle
    display->draw(display->rectangle()
                      .top_left(5, 95)
                      .bottom_right(30, 115)
                      .color(Color::Black)
                      .border_width(DotPixel::Pixel1x1)
                      .fill(DrawFill::Empty)
                      .build());
    // Filled rectangle
    display->draw(display->rectangle()
                      .top_left(35, 95)
                      .bottom_right(60, 115)
                      .color(Color::Black)
                      .border_width(DotPixel::Pixel1x1)
                      .fill(DrawFill::Full)
                      .build());
    // Thick border empty
    display->draw(display->rectangle()
                      .top_left(65, 95)
                      .bottom_right(90, 115)
                      .color(Color::Black)
                      .border_width(DotPixel::Pixel2x2)
                      .fill(DrawFill::Empty)
                      .build());
    // Small filled
    display->draw(display->rectangle()
                      .top_left(95, 95)
                      .bottom_right(110, 115)
                      .color(Color::Black)
                      .border_width(DotPixel::Pixel1x1)
                      .fill(DrawFill::Full)
                      .build());

    // Test Circles - empty and filled
    std::cout << "Drawing circles...\n";
    display->draw(display->text("Circles:")
                      .at(150, 18)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    // Empty circle
    display->draw(display->circle()
                      .center(165, 40)
                      .radius(15)
                      .color(Color::Black)
                      .border_width(DotPixel::Pixel1x1)
                      .fill(DrawFill::Empty)
                      .build());
    // Filled circle
    display->draw(display->circle()
                      .center(200, 40)
                      .radius(15)
                      .color(Color::Black)
                      .border_width(DotPixel::Pixel1x1)
                      .fill(DrawFill::Full)
                      .build());
    // Small circle
    display->draw(display->circle()
                      .center(235, 40)
                      .radius(8)
                      .color(Color::Black)
                      .border_width(DotPixel::Pixel1x1)
                      .fill(DrawFill::Empty)
                      .build());
    // Thick border circle
    display->draw(display->circle()
                      .center(165, 80)
                      .radius(12)
                      .color(Color::Black)
                      .border_width(DotPixel::Pixel2x2)
                      .fill(DrawFill::Empty)
                      .build());

    // Test Text - different fonts
    std::cout << "Drawing text...\n";
    display->draw(display->text("Text Fonts:")
                      .at(5, 125)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text("Font8")
                      .at(5, 135)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text("Font12")
                      .at(45, 135)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text("Font16")
                      .at(95, 135)
                      .font(&Font::font16())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());

    // Test Numbers - positive, negative, zero
    std::cout << "Drawing numbers...\n";
    display->draw(display->text("Numbers:")
                      .at(5, 155)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text()
                      .number(0)
                      .at(5, 165)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text()
                      .number(42)
                      .at(25, 165)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text()
                      .number(-123)
                      .at(55, 165)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text()
                      .number(999999)
                      .at(100, 165)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());

    // Test Decimals - various precisions
    std::cout << "Drawing decimals...\n";
    display->draw(display->text("Decimals:")
                      .at(150, 85)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text()
                      .decimal(std::numbers::pi, 2)
                      .at(150, 95)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text()
                      .decimal(-2.5, 1)
                      .at(150, 110)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text()
                      .decimal(0.001, 3)
                      .at(150, 125)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text()
                      .decimal(99.99, 2)
                      .at(150, 138)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());

    // Complex shape combination
    std::cout << "Drawing complex pattern...\n";
    display->draw(display->text("Pattern:")
                      .at(150, 155)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->circle()
                      .center(185, 168)
                      .radius(8)
                      .color(Color::Black)
                      .border_width(DotPixel::Pixel1x1)
                      .fill(DrawFill::Empty)
                      .build());
    display->draw(display->rectangle()
                      .top_left(175, 165)
                      .bottom_right(180, 170)
                      .color(Color::Black)
                      .border_width(DotPixel::Pixel1x1)
                      .fill(DrawFill::Full)
                      .build());
    display->draw(display->rectangle()
                      .top_left(190, 165)
                      .bottom_right(195, 170)
                      .color(Color::Black)
                      .border_width(DotPixel::Pixel1x1)
                      .fill(DrawFill::Full)
                      .build());
    display->draw(display->line()
                      .from(180, 173)
                      .to(190, 173)
                      .color(Color::Black)
                      .width(DotPixel::Pixel2x2)
                      .style(LineStyle::Solid)
                      .build());

    // Refresh display
    std::cout << "Refreshing display...\n";
    if (auto result = display->refresh(); !result) {
      std::cerr << "Refresh failed: " << result.error().what() << "\n";
      return EXIT_FAILURE;
    }

    std::cout << "\n=======================================\n";
    std::cout << "  Drawing Primitives Test: PASSED\n";
    std::cout << "  =======================================\n";
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
