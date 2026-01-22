#include "test_config.hpp"
#include <cstdlib>
#include <epaper/core/device.hpp>
#include <epaper/core/display.hpp>
#include <epaper/graphics/font.hpp>
#include <iostream>

using namespace epaper;

auto main() -> int {
  std::cout << "=======================================\n";
  std::cout << "  Font Sizes Test\n";
  std::cout << "=======================================\n";
  std::cout << "This test displays all available font sizes.\n\n";

  try {
    // Initialize device
    std::cout << "Initializing device...\n";
    Device device;
    if (auto result = device.init(); !result) {
      std::cerr << "Device initialization failed: " << result.error().what() << "\n";
      return EXIT_FAILURE;
    }

    // Create display in landscape for more space
    auto display =
        create_display<TestDriver, MonoFramebuffer>(device, DisplayMode::BlackWhite, Orientation::Landscape90);
    if (!display) {
      std::cerr << "Display initialization failed: " << display.error().what() << "\n";
      return EXIT_FAILURE;
    }

    std::cout << "Display size: " << display->effective_width() << "x" << display->effective_height() << "\n";

    display->clear(Color::White);

    // Title
    display->draw(display->text("FONT SIZE TEST")
                      .at(5, 2)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());

    std::size_t y_pos = 20;

    // Font8 test
    std::cout << "Testing Font8...\n";
    display->draw(display->text("Font8:")
                      .at(5, y_pos)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text("ABCDEFGHIJKLMNOPQRSTUVWXYZ")
                      .at(50, y_pos)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    y_pos += 10;
    display->draw(display->text("abcdefghijklmnopqrstuvwxyz")
                      .at(50, y_pos)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    y_pos += 10;
    display->draw(display->text("0123456789 !@#$%^&*()")
                      .at(50, y_pos)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    y_pos += 15;

    // Font12 test
    std::cout << "Testing Font12...\n";
    display->draw(display->text("Font12:")
                      .at(5, y_pos)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text("ABCDEFGHIJKLMNOPQRSTUVWXYZ")
                      .at(60, y_pos)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    y_pos += 14;
    display->draw(display->text("abcdefghijklmnopqrstuvwxyz")
                      .at(60, y_pos)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    y_pos += 14;
    display->draw(display->text("0123456789 !@#$%")
                      .at(60, y_pos)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    y_pos += 18;

    // Font16 test
    std::cout << "Testing Font16...\n";
    display->draw(display->text("Font16:")
                      .at(5, y_pos)
                      .font(&Font::font16())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text("ABCDEFGHIJKLMNOPQRST")
                      .at(70, y_pos)
                      .font(&Font::font16())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    y_pos += 18;
    display->draw(display->text("abcdefghijklmnopqrst")
                      .at(70, y_pos)
                      .font(&Font::font16())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    y_pos += 18;
    display->draw(display->text("0123456789 !@#$")
                      .at(70, y_pos)
                      .font(&Font::font16())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    y_pos += 22;

    // Font20 test
    std::cout << "Testing Font20...\n";
    display->draw(display->text("Font20:")
                      .at(5, y_pos)
                      .font(&Font::font20())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text("ABCDEFGHIJKLMNO")
                      .at(85, y_pos)
                      .font(&Font::font20())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    y_pos += 22;
    display->draw(display->text("abcdefghijklmno")
                      .at(85, y_pos)
                      .font(&Font::font20())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    y_pos += 22;
    display->draw(display->text("0123456789")
                      .at(85, y_pos)
                      .font(&Font::font20())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    y_pos += 26;

    // Font24 test
    std::cout << "Testing Font24...\n";
    display->draw(display->text("Font24:")
                      .at(5, y_pos)
                      .font(&Font::font24())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text("ABCDEFGHIJKLM")
                      .at(95, y_pos)
                      .font(&Font::font24())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    y_pos += 26;
    display->draw(display->text("abcdefghijklm")
                      .at(95, y_pos)
                      .font(&Font::font24())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    y_pos += 26;
    display->draw(display->text("0123456789")
                      .at(95, y_pos)
                      .font(&Font::font24())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());

    // Refresh display
    std::cout << "Refreshing display...\n";
    if (auto result = display->refresh(); !result) {
      std::cerr << "Refresh failed: " << result.error().what() << "\n";
      return EXIT_FAILURE;
    }

    std::cout << "\n=======================================\n";
    std::cout << "  Font Sizes Test: PASSED\n";
    std::cout << "=======================================\n";
    std::cout << "\nVisual verification checklist:\n";
    std::cout << "  [ ] Font8: Smallest, all characters readable\n";
    std::cout << "  [ ] Font12: Small, good for dense text\n";
    std::cout << "  [ ] Font16: Medium, standard size\n";
    std::cout << "  [ ] Font20: Large, good for titles\n";
    std::cout << "  [ ] Font24: Largest, very readable\n";
    std::cout << "  [ ] All uppercase letters visible\n";
    std::cout << "  [ ] All lowercase letters visible\n";
    std::cout << "  [ ] All digits (0-9) visible\n";
    std::cout << "  [ ] Special characters visible\n";
    std::cout << "  [ ] Font sizes increase progressively\n";

    return EXIT_SUCCESS;

  } catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
    return EXIT_FAILURE;
  }
}
