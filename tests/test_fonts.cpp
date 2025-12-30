#include <cstdlib>
#include <epaper/device.hpp>
#include <epaper/display.hpp>
#include <epaper/drivers/epd27.hpp>
#include <epaper/font.hpp>
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
    auto display = create_display<EPD27>(device, DisplayMode::BlackWhite, Orientation::Landscape90);
    if (!display) {
      std::cerr << "Display initialization failed: " << display.error().what() << "\n";
      return EXIT_FAILURE;
    }

    std::cout << "Display size: " << display->effective_width() << "x" << display->effective_height() << "\n";

    display->clear(Color::White);

    // Title
    display->draw_string(5, 2, "FONT SIZE TEST", Font::font12(), Color::Black, Color::White);

    std::size_t y_pos = 20;

    // Font8 test
    std::cout << "Testing Font8...\n";
    display->draw_string(5, y_pos, "Font8:", Font::font8(), Color::Black, Color::White);
    display->draw_string(50, y_pos, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", Font::font8(), Color::Black, Color::White);
    y_pos += 10;
    display->draw_string(50, y_pos, "abcdefghijklmnopqrstuvwxyz", Font::font8(), Color::Black, Color::White);
    y_pos += 10;
    display->draw_string(50, y_pos, "0123456789 !@#$%^&*()", Font::font8(), Color::Black, Color::White);
    y_pos += 15;

    // Font12 test
    std::cout << "Testing Font12...\n";
    display->draw_string(5, y_pos, "Font12:", Font::font12(), Color::Black, Color::White);
    display->draw_string(60, y_pos, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", Font::font12(), Color::Black, Color::White);
    y_pos += 14;
    display->draw_string(60, y_pos, "abcdefghijklmnopqrstuvwxyz", Font::font12(), Color::Black, Color::White);
    y_pos += 14;
    display->draw_string(60, y_pos, "0123456789 !@#$%", Font::font12(), Color::Black, Color::White);
    y_pos += 18;

    // Font16 test
    std::cout << "Testing Font16...\n";
    display->draw_string(5, y_pos, "Font16:", Font::font16(), Color::Black, Color::White);
    display->draw_string(70, y_pos, "ABCDEFGHIJKLMNOPQRST", Font::font16(), Color::Black, Color::White);
    y_pos += 18;
    display->draw_string(70, y_pos, "abcdefghijklmnopqrst", Font::font16(), Color::Black, Color::White);
    y_pos += 18;
    display->draw_string(70, y_pos, "0123456789 !@#$", Font::font16(), Color::Black, Color::White);
    y_pos += 22;

    // Font20 test
    std::cout << "Testing Font20...\n";
    display->draw_string(5, y_pos, "Font20:", Font::font20(), Color::Black, Color::White);
    display->draw_string(85, y_pos, "ABCDEFGHIJKLMNO", Font::font20(), Color::Black, Color::White);
    y_pos += 22;
    display->draw_string(85, y_pos, "abcdefghijklmno", Font::font20(), Color::Black, Color::White);
    y_pos += 22;
    display->draw_string(85, y_pos, "0123456789", Font::font20(), Color::Black, Color::White);
    y_pos += 26;

    // Font24 test
    std::cout << "Testing Font24...\n";
    display->draw_string(5, y_pos, "Font24:", Font::font24(), Color::Black, Color::White);
    display->draw_string(95, y_pos, "ABCDEFGHIJKLM", Font::font24(), Color::Black, Color::White);
    y_pos += 26;
    display->draw_string(95, y_pos, "abcdefghijklm", Font::font24(), Color::Black, Color::White);
    y_pos += 26;
    display->draw_string(95, y_pos, "0123456789", Font::font24(), Color::Black, Color::White);

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
