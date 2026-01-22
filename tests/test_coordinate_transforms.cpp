#include "test_config.hpp"
#include <cmath>
#include <cstdlib>
#include <epaper/core/device.hpp>
#include <epaper/core/display.hpp>
#include <epaper/graphics/font.hpp>
#include <iostream>
#include <thread>

using namespace epaper;

template <typename DisplayType>
auto test_corner_markers(DisplayType &display, std::string_view orientation_name) -> void {
  const auto width = display.effective_width();
  const auto height = display.effective_height();

  display.clear(Color::White);

  // Title
  display.draw(display.text(orientation_name)
                   .at(5, 5)
                   .font(&Font::font12())
                   .foreground(Color::Black)
                   .background(Color::White)
                   .build());
  display.draw(display.text("Corner Transform Test")
                   .at(5, 20)
                   .font(&Font::font8())
                   .foreground(Color::Black)
                   .background(Color::White)
                   .build());

  // Draw corners with distinctive patterns
  // Top-left (0, 0)
  display.draw(display.rectangle()
                   .top_left(0, 0)
                   .bottom_right(20, 20)
                   .color(Color::Black)
                   .border_width(DotPixel::Pixel2x2)
                   .fill(DrawFill::Empty)
                   .build());
  display.draw(
      display.text("TL").at(3, 6).font(&Font::font8()).foreground(Color::Black).background(Color::White).build());
  display.draw(display.line()
                   .from(0, 0)
                   .to(10, 10)
                   .color(Color::Black)
                   .width(DotPixel::Pixel1x1)
                   .style(LineStyle::Solid)
                   .build());

  // Top-right (width-1, 0)
  display.draw(display.rectangle()
                   .top_left(width - 21, 0)
                   .bottom_right(width - 1, 20)
                   .color(Color::Black)
                   .border_width(DotPixel::Pixel2x2)
                   .fill(DrawFill::Empty)
                   .build());
  display.draw(display.text("TR")
                   .at(width - 18, 6)
                   .font(&Font::font8())
                   .foreground(Color::Black)
                   .background(Color::White)
                   .build());
  display.draw(display.line()
                   .from(width - 1, 0)
                   .to(width - 11, 10)
                   .color(Color::Black)
                   .width(DotPixel::Pixel1x1)
                   .style(LineStyle::Solid)
                   .build());

  // Bottom-left (0, height-1)
  display.draw(display.rectangle()
                   .top_left(0, height - 21)
                   .bottom_right(20, height - 1)
                   .color(Color::Black)
                   .border_width(DotPixel::Pixel2x2)
                   .fill(DrawFill::Empty)
                   .build());
  display.draw(display.text("BL")
                   .at(3, height - 15)
                   .font(&Font::font8())
                   .foreground(Color::Black)
                   .background(Color::White)
                   .build());
  display.draw(display.line()
                   .from(0, height - 1)
                   .to(10, height - 11)
                   .color(Color::Black)
                   .width(DotPixel::Pixel1x1)
                   .style(LineStyle::Solid)
                   .build());

  // Bottom-right (width-1, height-1)
  display.draw(display.rectangle()
                   .top_left(width - 21, height - 21)
                   .bottom_right(width - 1, height - 1)
                   .color(Color::Black)
                   .border_width(DotPixel::Pixel2x2)
                   .fill(DrawFill::Empty)
                   .build());
  display.draw(display.rectangle()
                   .top_left(width - 21, height - 21)
                   .bottom_right(width - 1, height - 1)
                   .color(Color::Black)
                   .border_width(DotPixel::Pixel2x2)
                   .fill(DrawFill::Empty)
                   .build());
  display.draw(display.text("BR")
                   .at(width - 18, height - 15)
                   .font(&Font::font8())
                   .foreground(Color::Black)
                   .background(Color::White)
                   .build());
  display.draw(display.line()
                   .from(width - 1, height - 1)
                   .to(width - 11, height - 11)
                   .color(Color::Black)
                   .width(DotPixel::Pixel1x1)
                   .style(LineStyle::Solid)
                   .build());

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
  display.draw(display.line()
                   .from(center_x - 15, center_y)
                   .to(center_x + 15, center_y)
                   .color(Color::Black)
                   .width(DotPixel::Pixel2x2)
                   .style(LineStyle::Solid)
                   .build());
  display.draw(display.line()
                   .from(center_x - 15, center_y)
                   .to(center_x + 15, center_y)
                   .color(Color::Black)
                   .width(DotPixel::Pixel2x2)
                   .style(LineStyle::Solid)
                   .build());
  display.draw(display.line()
                   .from(center_x, center_y - 15)
                   .to(center_x, center_y + 15)
                   .color(Color::Black)
                   .width(DotPixel::Pixel2x2)
                   .style(LineStyle::Solid)
                   .build());
  display.draw(display.line()
                   .from(center_x, center_y - 15)
                   .to(center_x, center_y + 15)
                   .color(Color::Black)
                   .width(DotPixel::Pixel2x2)
                   .style(LineStyle::Solid)
                   .build());
  display.draw(display.circle()
                   .center(center_x, center_y)
                   .radius(10)
                   .color(Color::Black)
                   .border_width(DotPixel::Pixel1x1)
                   .fill(DrawFill::Empty)
                   .build());

  // Draw dimension information
  std::string dim_str = std::to_string(width) + "x" + std::to_string(height);
  display.draw(display.text(dim_str)
                   .at(center_x - 25, center_y + 20)
                   .font(&Font::font8())
                   .foreground(Color::Black)
                   .background(Color::White)
                   .build());
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
      auto display =
          create_display<TestDriver, MonoFramebuffer>(device, DisplayMode::BlackWhite, Orientation::Portrait0);
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
      auto display =
          create_display<TestDriver, MonoFramebuffer>(device, DisplayMode::BlackWhite, Orientation::Landscape90);
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
      auto display =
          create_display<TestDriver, MonoFramebuffer>(device, DisplayMode::BlackWhite, Orientation::Portrait180);
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
      auto display =
          create_display<TestDriver, MonoFramebuffer>(device, DisplayMode::BlackWhite, Orientation::Landscape270);
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
      auto display =
          create_display<TestDriver, MonoFramebuffer>(device, DisplayMode::BlackWhite, Orientation::Portrait0);
      if (!display) {
        std::cerr << "Display creation failed: " << display.error().what() << "\n";
        return EXIT_FAILURE;
      }

      const auto width = display->effective_width();
      const auto height = display->effective_height();

      std::cout << "Testing intensive boundary operations...\n";
      display->clear(Color::White);
      display->draw(display->text("BOUNDARY STRESS")
                        .at(5, 5)
                        .font(&Font::font12())
                        .foreground(Color::Black)
                        .background(Color::White)
                        .build());

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
      display->draw(display->line()
                        .from(0, 0)
                        .to(width - 1, height - 1)
                        .color(Color::Black)
                        .width(DotPixel::Pixel1x1)
                        .style(LineStyle::Solid)
                        .build());
      display->draw(display->line()
                        .from(width - 1, 0)
                        .to(0, height - 1)
                        .color(Color::Black)
                        .width(DotPixel::Pixel1x1)
                        .style(LineStyle::Dotted)
                        .build());

      // Draw circles at corners
      std::cout << "  Drawing circles at corners...\n";
      display->draw(display->circle()
                        .center(15, 15)
                        .radius(10)
                        .color(Color::Black)
                        .border_width(DotPixel::Pixel1x1)
                        .fill(DrawFill::Empty)
                        .build());
      display->draw(display->circle()
                        .center(width - 16, 15)
                        .radius(10)
                        .color(Color::Black)
                        .border_width(DotPixel::Pixel1x1)
                        .fill(DrawFill::Empty)
                        .build());
      display->draw(display->circle()
                        .center(15, height - 16)
                        .radius(10)
                        .color(Color::Black)
                        .border_width(DotPixel::Pixel1x1)
                        .fill(DrawFill::Empty)
                        .build());
      display->draw(display->circle()
                        .center(width - 16, height - 16)
                        .radius(10)
                        .color(Color::Black)
                        .border_width(DotPixel::Pixel1x1)
                        .fill(DrawFill::Empty)
                        .build());

      // Test exact boundary coordinates
      std::cout << "  Testing exact boundary pixels...\n";
      display->draw(display->point().at(0, 0).color(Color::Black).size(DotPixel::Pixel3x3).build());
      display->draw(display->point().at(width - 1, 0).color(Color::Black).size(DotPixel::Pixel3x3).build());
      display->draw(display->point().at(0, height - 1).color(Color::Black).size(DotPixel::Pixel3x3).build());
      display->draw(display->point().at(width - 1, height - 1).color(Color::Black).size(DotPixel::Pixel3x3).build());

      display->draw(display->text("All boundaries tested")
                        .at(5, 25)
                        .font(&Font::font8())
                        .foreground(Color::Black)
                        .background(Color::White)
                        .build());
      display->draw(display->text("No artifacts expected")
                        .at(5, 35)
                        .font(&Font::font8())
                        .foreground(Color::Black)
                        .background(Color::White)
                        .build());

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
