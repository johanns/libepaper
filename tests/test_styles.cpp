#include "test_config.hpp"
#include <cstdlib>
#include <epaper/core/device.hpp>
#include <epaper/core/display.hpp>
#include <epaper/draw/styles.hpp>
#include <epaper/graphics/font.hpp>
#include <iostream>

using namespace epaper;

auto main() -> int {
  std::cout << "=======================================\n";
  std::cout << "  Style Specs Test\n";
  std::cout << "=======================================\n";

  try {
    Device device;
    if (auto result = device.init(); !result) {
      std::cerr << "Device initialization failed: " << result.error().what() << "\n";
      return EXIT_FAILURE;
    }

    auto display_res = create_display<TestDriver, MonoFramebuffer>(device, DisplayMode::BlackWhite);
    if (!display_res) {
      std::cerr << "Display initialization failed: " << display_res.error().what() << "\n";
      return EXIT_FAILURE;
    }
    auto &display = display_res.value();

    display.clear();

    // 1. LineStyleSpec
    std::cout << "Testing LineStyleSpec...\n";
    LineStyleSpec thick_black{Color::Black, DotPixel::Pixel3x3, LineStyle::Solid};
    display.draw(display.line().from({10, 10}).to({100, 10}).with_style(thick_black).build());

    // 2. ShapeStyleSpec (Rectangle)
    std::cout << "Testing ShapeStyleSpec (Rectangle)...\n";
    ShapeStyleSpec filled_black{Color::Black, DotPixel::Pixel1x1, DrawFill::Full};
    display.draw(display.rectangle().top_left({10, 20}).bottom_right({50, 60}).with_style(filled_black).build());

    // 3. ShapeStyleSpec (Circle)
    std::cout << "Testing ShapeStyleSpec (Circle)...\n";
    ShapeStyleSpec border_black{Color::Black, DotPixel::Pixel2x2, DrawFill::Empty};
    display.draw(display.circle().center({80, 40}).radius(15).with_style(border_black).build());

    // 4. TextStyleSpec
    std::cout << "Testing TextStyleSpec...\n";
    TextStyleSpec text_style{&Font::font20(), Color::Black, Color::White};
    display.draw(display.text("Styled Text").at({10, 80}).with_style(text_style).build());

    if (auto res = display.refresh(); !res) {
      std::cerr << "Refresh failed: " << res.error().what() << "\n";
      return EXIT_FAILURE;
    }

    std::cout << "Test passed!\n";

  } catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
