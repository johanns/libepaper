#include <epaper/device.hpp>
#include <epaper/display.hpp>
#include <epaper/drivers/epd27.hpp>
#include <epaper/font.hpp>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

using namespace epaper;

auto test_corner_markers(Display &display, std::string_view orientation_name) -> void {
  const auto width = display.effective_width();
  const auto height = display.effective_height();

  display.clear(Color::White);

  // Title
  display.draw_string(5, 5, orientation_name, Font::font12(), Color::Black, Color::White);
  display.draw_string(5, 20, "Corner Transform Test", Font::font8(), Color::Black, Color::White);

  // Draw corners with distinctive patterns
  // Top-left (0, 0)
  display.draw_rectangle(0, 0, 20, 20, Color::Black, DotPixel::Pixel2x2, DrawFill::Empty);
  display.draw_string(3, 6, "TL", Font::font8(), Color::Black, Color::White);
  display.draw_line(0, 0, 10, 10, Color::Black, DotPixel::Pixel1x1, LineStyle::Solid);

  // Top-right (width-1, 0)
  display.draw_rectangle(width - 21, 0, width - 1, 20, Color::Black, DotPixel::Pixel2x2, DrawFill::Empty);
  display.draw_string(width - 18, 6, "TR", Font::font8(), Color::Black, Color::White);
  display.draw_line(width - 1, 0, width - 11, 10, Color::Black, DotPixel::Pixel1x1, LineStyle::Solid);

  // Bottom-left (0, height-1)
  display.draw_rectangle(0, height - 21, 20, height - 1, Color::Black, DotPixel::Pixel2x2, DrawFill::Empty);
  display.draw_string(3, height - 15, "BL", Font::font8(), Color::Black, Color::White);
  display.draw_line(0, height - 1, 10, height - 11, Color::Black, DotPixel::Pixel1x1, LineStyle::Solid);

  // Bottom-right (width-1, height-1)
  display.draw_rectangle(width - 21, height - 21, width - 1, height - 1, Color::Black, DotPixel::Pixel2x2,
                         DrawFill::Empty);
  display.draw_string(width - 18, height - 15, "BR", Font::font8(), Color::Black, Color::White);
  display.draw_line(width - 1, height - 1, width - 11, height - 11, Color::Black, DotPixel::Pixel1x1, LineStyle::Solid);

  // Test edge coordinates
  // Draw pixels along all four edges
  for (std::size_t i = 25; i < width - 25; i += 5) {
    display.set_pixel(i, 0, Color::Black);
    display.set_pixel(i, height - 1, Color::Black);
  }
  for (std::size_t i = 25; i < height - 25; i += 5) {
    display.set_pixel(0, i, Color::Black);
    display.set_pixel(width - 1, i, Color::Black);
  }

  // Draw center cross
  const auto center_x = width / 2;
  const auto center_y = height / 2;
  display.draw_line(center_x - 15, center_y, center_x + 15, center_y, Color::Black, DotPixel::Pixel2x2,
                    LineStyle::Solid);
  display.draw_line(center_x, center_y - 15, center_x, center_y + 15, Color::Black, DotPixel::Pixel2x2,
                    LineStyle::Solid);
  display.draw_circle(center_x, center_y, 10, Color::Black, DotPixel::Pixel1x1, DrawFill::Empty);

  // Draw dimension information
  std::string dim_str = std::to_string(width) + "x" + std::to_string(height);
  display.draw_string(center_x - 25, center_y + 20, dim_str, Font::font8(), Color::Black, Color::White);
}

auto main() -> int {
  std::cout << "=======================================\n";
  std::cout << "  Coordinate Transforms Test\n";
  std::cout << "=======================================\n";
  std::cout << "This test verifies coordinate transformations at boundaries.\n\n";

  try {
    // Initialize device
    std::cout << "Initializing device...\n";
    Device device;
    if (auto result = device.init(); !result) {
      std::cerr << "Device initialization failed: " << result.error().what() << "\n";
      return EXIT_FAILURE;
    }

    // Test Portrait 0°
    std::cout << "\n=== Test 1: Portrait 0° ===\n";
    {
      auto display = create_display<EPD27>(device, DisplayMode::BlackWhite, Orientation::Portrait0);
      if (!display) {
        std::cerr << "Display creation failed: " << display.error().what() << "\n";
        return EXIT_FAILURE;
      }

      std::cout << "Orientation: Portrait 0°\n";
      std::cout << "Effective size: " << display->effective_width() << "x" << display->effective_height() << "\n";
      test_corner_markers(*display, "Portrait 0 deg");

      std::cout << "Refreshing display...\n";
      if (auto result = display->refresh(); !result) {
        std::cerr << "Refresh failed: " << result.error().what() << "\n";
        return EXIT_FAILURE;
      }
      std::cout << "✓ Portrait 0° test complete.\n";
      std::cout << "Press Enter to continue...\n";
      std::cin.get();
    }

    // Test Landscape 90°
    std::cout << "\n=== Test 2: Landscape 90° ===\n";
    {
      auto display = create_display<EPD27>(device, DisplayMode::BlackWhite, Orientation::Landscape90);
      if (!display) {
        std::cerr << "Display creation failed: " << display.error().what() << "\n";
        return EXIT_FAILURE;
      }

      std::cout << "Orientation: Landscape 90°\n";
      std::cout << "Effective size: " << display->effective_width() << "x" << display->effective_height() << "\n";
      test_corner_markers(*display, "Landscape 90 deg");

      std::cout << "Refreshing display...\n";
      if (auto result = display->refresh(); !result) {
        std::cerr << "Refresh failed: " << result.error().what() << "\n";
        return EXIT_FAILURE;
      }
      std::cout << "✓ Landscape 90° test complete.\n";
      std::cout << "Press Enter to continue...\n";
      std::cin.get();
    }

    // Test Portrait 180°
    std::cout << "\n=== Test 3: Portrait 180° ===\n";
    {
      auto display = create_display<EPD27>(device, DisplayMode::BlackWhite, Orientation::Portrait180);
      if (!display) {
        std::cerr << "Display creation failed: " << display.error().what() << "\n";
        return EXIT_FAILURE;
      }

      std::cout << "Orientation: Portrait 180°\n";
      std::cout << "Effective size: " << display->effective_width() << "x" << display->effective_height() << "\n";
      test_corner_markers(*display, "Portrait 180 deg");

      std::cout << "Refreshing display...\n";
      if (auto result = display->refresh(); !result) {
        std::cerr << "Refresh failed: " << result.error().what() << "\n";
        return EXIT_FAILURE;
      }
      std::cout << "✓ Portrait 180° test complete.\n";
      std::cout << "Press Enter to continue...\n";
      std::cin.get();
    }

    // Test Landscape 270°
    std::cout << "\n=== Test 4: Landscape 270° ===\n";
    {
      auto display = create_display<EPD27>(device, DisplayMode::BlackWhite, Orientation::Landscape270);
      if (!display) {
        std::cerr << "Display creation failed: " << display.error().what() << "\n";
        return EXIT_FAILURE;
      }

      std::cout << "Orientation: Landscape 270°\n";
      std::cout << "Effective size: " << display->effective_width() << "x" << display->effective_height() << "\n";
      test_corner_markers(*display, "Landscape 270 deg");

      std::cout << "Refreshing display...\n";
      if (auto result = display->refresh(); !result) {
        std::cerr << "Refresh failed: " << result.error().what() << "\n";
        return EXIT_FAILURE;
      }
      std::cout << "✓ Landscape 270° test complete.\n";
      std::cout << "Press Enter to continue...\n";
      std::cin.get();
    }

    // Boundary stress test
    std::cout << "\n=== Test 5: Boundary Stress Test ===\n";
    {
      auto display = create_display<EPD27>(device, DisplayMode::BlackWhite, Orientation::Portrait0);
      if (!display) {
        std::cerr << "Display creation failed: " << display.error().what() << "\n";
        return EXIT_FAILURE;
      }

      const auto width = display->effective_width();
      const auto height = display->effective_height();

      std::cout << "Testing intensive boundary operations...\n";
      display->clear(Color::White);
      display->draw_string(5, 5, "BOUNDARY STRESS", Font::font12(), Color::Black, Color::White);

      // Draw many points along all edges
      std::cout << "  Drawing points along edges...\n";
      for (std::size_t i = 0; i < width; i += 2) {
        display->set_pixel(i, 0, Color::Black);
        display->set_pixel(i, height - 1, Color::Black);
      }
      for (std::size_t i = 0; i < height; i += 2) {
        display->set_pixel(0, i, Color::Black);
        display->set_pixel(width - 1, i, Color::Black);
      }

      // Draw diagonal lines from corners
      std::cout << "  Drawing diagonal lines from corners...\n";
      display->draw_line(0, 0, width - 1, height - 1, Color::Black, DotPixel::Pixel1x1, LineStyle::Solid);
      display->draw_line(width - 1, 0, 0, height - 1, Color::Black, DotPixel::Pixel1x1, LineStyle::Dotted);

      // Draw circles at corners
      std::cout << "  Drawing circles at corners...\n";
      display->draw_circle(15, 15, 10, Color::Black, DotPixel::Pixel1x1, DrawFill::Empty);
      display->draw_circle(width - 16, 15, 10, Color::Black, DotPixel::Pixel1x1, DrawFill::Empty);
      display->draw_circle(15, height - 16, 10, Color::Black, DotPixel::Pixel1x1, DrawFill::Empty);
      display->draw_circle(width - 16, height - 16, 10, Color::Black, DotPixel::Pixel1x1, DrawFill::Empty);

      // Test exact boundary coordinates
      std::cout << "  Testing exact boundary pixels...\n";
      display->draw_point(0, 0, Color::Black, DotPixel::Pixel3x3);
      display->draw_point(width - 1, 0, Color::Black, DotPixel::Pixel3x3);
      display->draw_point(0, height - 1, Color::Black, DotPixel::Pixel3x3);
      display->draw_point(width - 1, height - 1, Color::Black, DotPixel::Pixel3x3);

      display->draw_string(5, 25, "All boundaries tested", Font::font8(), Color::Black, Color::White);
      display->draw_string(5, 35, "No artifacts expected", Font::font8(), Color::Black, Color::White);

      std::cout << "Refreshing display...\n";
      if (auto result = display->refresh(); !result) {
        std::cerr << "Refresh failed: " << result.error().what() << "\n";
        return EXIT_FAILURE;
      }
      std::cout << "✓ Boundary stress test complete.\n";
    }

    std::cout << "\n=======================================\n";
    std::cout << "  Coordinate Transforms Test: PASSED\n";
    std::cout << "=======================================\n";
    std::cout << "\nVisual verification checklist:\n";
    std::cout << "  [ ] All 4 orientations tested\n";
    std::cout << "  [ ] Corner markers (TL, TR, BL, BR) in correct positions\n";
    std::cout << "  [ ] Edge pixels drawn correctly\n";
    std::cout << "  [ ] Center cross properly positioned\n";
    std::cout << "  [ ] Dimensions correct for each orientation\n";
    std::cout << "  [ ] Boundary stress test completed without artifacts\n";
    std::cout << "  [ ] No coordinate transformation errors\n";
    std::cout << "  [ ] Diagonal lines from corners displayed correctly\n";
    std::cout << "  [ ] Corner circles displayed correctly\n";

    return EXIT_SUCCESS;

  } catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
    return EXIT_FAILURE;
  }
}
