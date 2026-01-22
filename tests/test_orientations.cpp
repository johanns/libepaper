#include "test_config.hpp"
#include <chrono>
#include <cstdlib>
#include <epaper/core/device.hpp>
#include <epaper/core/display.hpp>
#include <epaper/graphics/font.hpp>
#include <iostream>
#include <thread>

using namespace epaper;

auto test_orientation(Device &device, Orientation orientation, std::string_view name) -> bool {
  std::cout << "\n=== Testing " << name << " ===\n";

  auto display = create_display<TestDriver, MonoFramebuffer>(device, DisplayMode::BlackWhite, orientation);
  if (!display) {
    std::cerr << "Failed to create display: " << display.error().what() << "\n";
    return false;
  }

  std::cout << "Orientation: " << name << "\n";
  std::cout << "Effective dimensions: " << display->effective_width() << "x" << display->effective_height()
            << " pixels\n";

  display->clear(Color::White);

  // Draw orientation indicator
  const auto width = display->effective_width();
  const auto height = display->effective_height();

  // Title at top
  display->draw(
      display->text(name).at(5, 5).font(&Font::font16()).foreground(Color::Black).background(Color::White).build());

  // Draw border
  display->draw(display->rectangle()
                    .top_left(0, 0)
                    .bottom_right(width - 1, height - 1)
                    .color(Color::Black)
                    .border_width(DotPixel::Pixel1x1)
                    .fill(DrawFill::Empty)
                    .build());

  // Draw corner markers
  display->draw(
      display->text("TL").at(5, 20).font(&Font::font12()).foreground(Color::Black).background(Color::White).build());
  display->draw(display->text("TR")
                    .at(width - 25, 20)
                    .font(&Font::font12())
                    .foreground(Color::Black)
                    .background(Color::White)
                    .build());
  display->draw(display->text("BL")
                    .at(5, height - 20)
                    .font(&Font::font12())
                    .foreground(Color::Black)
                    .background(Color::White)
                    .build());
  display->draw(display->text("BR")
                    .at(width - 25, height - 20)
                    .font(&Font::font12())
                    .foreground(Color::Black)
                    .background(Color::White)
                    .build());

  // Draw center cross
  const auto center_x = width / 2;
  const auto center_y = height / 2;
  display->draw(display->line()
                    .from(center_x - 10, center_y)
                    .to(center_x + 10, center_y)
                    .color(Color::Black)
                    .width(DotPixel::Pixel2x2)
                    .style(LineStyle::Solid)
                    .build());
  display->draw(display->line()
                    .from(center_x, center_y - 10)
                    .to(center_x, center_y + 10)
                    .color(Color::Black)
                    .width(DotPixel::Pixel2x2)
                    .style(LineStyle::Solid)
                    .build());
  display->draw(display->text("CENTER")
                    .at(center_x - 20, center_y + 15)
                    .font(&Font::font8())
                    .foreground(Color::Black)
                    .background(Color::White)
                    .build());

  // Draw arrow pointing right (indicating orientation direction)
  const auto arrow_y = center_y + 40;
  display->draw(display->line()
                    .from(20, arrow_y)
                    .to(60, arrow_y)
                    .color(Color::Black)
                    .width(DotPixel::Pixel2x2)
                    .style(LineStyle::Solid)
                    .build());
  display->draw(display->line()
                    .from(60, arrow_y)
                    .to(50, arrow_y - 5)
                    .color(Color::Black)
                    .width(DotPixel::Pixel2x2)
                    .style(LineStyle::Solid)
                    .build());
  display->draw(display->line()
                    .from(60, arrow_y)
                    .to(50, arrow_y + 5)
                    .color(Color::Black)
                    .width(DotPixel::Pixel2x2)
                    .style(LineStyle::Solid)
                    .build());

  // Test circle at specific position
  display->draw(display->circle()
                    .center(width - 30, 50)
                    .radius(20)
                    .color(Color::Black)
                    .border_width(DotPixel::Pixel1x1)
                    .fill(DrawFill::Empty)
                    .build());
  display->draw(display->circle()
                    .center(width - 30, 50)
                    .radius(10)
                    .color(Color::Black)
                    .border_width(DotPixel::Pixel1x1)
                    .fill(DrawFill::Full)
                    .build());

  // Test rectangle
  display->draw(display->rectangle()
                    .top_left(20, height - 50)
                    .bottom_right(60, height - 30)
                    .color(Color::Black)
                    .border_width(DotPixel::Pixel1x1)
                    .fill(DrawFill::Full)
                    .build());

  // Display dimensions
  std::string dim_str = std::to_string(width) + "x" + std::to_string(height);
  display->draw(display->text(dim_str)
                    .at(center_x - 20, center_y - 30)
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

  std::cout << name << " test complete.\n";
  std::cout << "Visual verification:\n";
  std::cout << "  - Title appears at top\n";
  std::cout << "  - Corner markers (TL, TR, BL, BR) are in correct corners\n";
  std::cout << "  - Center cross is at display center\n";
  std::cout << "  - All shapes are correctly oriented\n";

  return true;
}

auto main() -> int {
  std::cout << "=======================================\n";
  std::cout << "  Display Orientations Test\n";
  std::cout << "=======================================\n";
  std::cout << "This test verifies all 4 display orientations.\n\n";

  try {
    // Initialize device
    std::cout << "Initializing device...\n";
    Device device;
    if (auto result = device.init(); !result) {
      std::cerr << "Device initialization failed: " << result.error().what() << "\n";
      return EXIT_FAILURE;
    }
    std::cout << "Device initialized successfully.\n";

    // Test Portrait 0°
    if (!test_orientation(device, Orientation::Portrait0, "Portrait 0°")) {
      return EXIT_FAILURE;
    }
    std::cout << "\nPress Enter to continue to next orientation...\n";
    std::cin.get();

    // Test Landscape 90°
    if (!test_orientation(device, Orientation::Landscape90, "Landscape 90°")) {
      return EXIT_FAILURE;
    }
    std::cout << "\nPress Enter to continue to next orientation...\n";
    std::cin.get();

    // Test Portrait 180°
    if (!test_orientation(device, Orientation::Portrait180, "Portrait 180°")) {
      return EXIT_FAILURE;
    }
    std::cout << "\nPress Enter to continue to next orientation...\n";
    std::cin.get();

    // Test Landscape 270°
    if (!test_orientation(device, Orientation::Landscape270, "Landscape 270°")) {
      return EXIT_FAILURE;
    }

    std::cout << "\n=======================================\n";
    std::cout << "  Display Orientations Test: PASSED\n";
    std::cout << "=======================================\n";
    std::cout << "\nAll 4 orientations tested successfully.\n";
    std::cout << "Verify that each orientation displayed correctly with:\n";
    std::cout << "  [ ] Title readable at top\n";
    std::cout << "  [ ] Corner markers in correct positions\n";
    std::cout << "  [ ] Center cross properly centered\n";
    std::cout << "  [ ] Dimensions match orientation\n";

    return EXIT_SUCCESS;

  } catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
    return EXIT_FAILURE;
  }
}
