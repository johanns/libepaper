#include <epaper/device.hpp>
#include <epaper/display.hpp>
#include <epaper/drivers/epd27.hpp>
#include <epaper/font.hpp>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

using namespace epaper;

auto test_orientation(Device &device, Orientation orientation, std::string_view name) -> bool {
  std::cout << "\n=== Testing " << name << " ===\n";

  auto display = create_display<EPD27>(device, DisplayMode::BlackWhite, orientation);
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
  display->draw_string(5, 5, name, Font::font16(), Color::Black, Color::White);

  // Draw border
  display->draw_rectangle(0, 0, width - 1, height - 1, Color::Black, DotPixel::Pixel1x1, DrawFill::Empty);

  // Draw corner markers
  display->draw_string(5, 20, "TL", Font::font12(), Color::Black, Color::White);
  display->draw_string(width - 25, 20, "TR", Font::font12(), Color::Black, Color::White);
  display->draw_string(5, height - 20, "BL", Font::font12(), Color::Black, Color::White);
  display->draw_string(width - 25, height - 20, "BR", Font::font12(), Color::Black, Color::White);

  // Draw center cross
  const auto center_x = width / 2;
  const auto center_y = height / 2;
  display->draw_line(center_x - 10, center_y, center_x + 10, center_y, Color::Black, DotPixel::Pixel2x2,
                     LineStyle::Solid);
  display->draw_line(center_x, center_y - 10, center_x, center_y + 10, Color::Black, DotPixel::Pixel2x2,
                     LineStyle::Solid);
  display->draw_string(center_x - 20, center_y + 15, "CENTER", Font::font8(), Color::Black, Color::White);

  // Draw arrow pointing right (indicating orientation direction)
  const auto arrow_y = center_y + 40;
  display->draw_line(20, arrow_y, 60, arrow_y, Color::Black, DotPixel::Pixel2x2, LineStyle::Solid);
  display->draw_line(60, arrow_y, 50, arrow_y - 5, Color::Black, DotPixel::Pixel2x2, LineStyle::Solid);
  display->draw_line(60, arrow_y, 50, arrow_y + 5, Color::Black, DotPixel::Pixel2x2, LineStyle::Solid);

  // Test circle at specific position
  display->draw_circle(width - 30, 50, 20, Color::Black, DotPixel::Pixel1x1, DrawFill::Empty);
  display->draw_circle(width - 30, 50, 10, Color::Black, DotPixel::Pixel1x1, DrawFill::Full);

  // Test rectangle
  display->draw_rectangle(20, height - 50, 60, height - 30, Color::Black, DotPixel::Pixel1x1, DrawFill::Full);

  // Display dimensions
  std::string dim_str = std::to_string(width) + "x" + std::to_string(height);
  display->draw_string(center_x - 20, center_y - 30, dim_str, Font::font12(), Color::Black, Color::White);

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
