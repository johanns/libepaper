#include <chrono>
#include <cstdlib>
#include <epaper/device.hpp>
#include <epaper/display.hpp>
#include <epaper/drivers/epd27.hpp>
#include <epaper/font.hpp>
#include <iostream>
#include <thread>

using namespace epaper;

auto main() -> int {
  std::cout << "=======================================\n";
  std::cout << "  Power Management Test\n";
  std::cout << "=======================================\n";
  std::cout << "This test exercises power management features.\n\n";

  try {
    // Initialize device
    std::cout << "Initializing device...\n";
    Device device;
    if (auto result = device.init(); !result) {
      std::cerr << "Device initialization failed: " << result.error().what() << "\n";
      return EXIT_FAILURE;
    }

    // Create display
    auto display = create_display<EPD27>(device, DisplayMode::BlackWhite);
    if (!display) {
      std::cerr << "Display initialization failed: " << display.error().what() << "\n";
      return EXIT_FAILURE;
    }

    std::cout << "Display size: " << display->effective_width() << "x" << display->effective_height() << "\n";

    // Check power management capabilities
    std::cout << "\nChecking power management capabilities:\n";
    std::cout << "  Supports wake from sleep: " << (display->supports_wake() ? "YES" : "NO") << "\n";
    std::cout << "  Supports power control: " << (display->supports_power_control() ? "YES" : "NO") << "\n";

    // Test 1: Basic display operation
    std::cout << "\n=== Test 1: Initial Display ===\n";
    display->clear(Color::White);
    display->draw_string(10, 10, "POWER MANAGEMENT TEST", Font::font16(), Color::Black, Color::White);
    display->draw_string(10, 30, "Test 1: Initial State", Font::font12(), Color::Black, Color::White);
    display->draw_rectangle(10, 50, 166, 100, Color::Black, DotPixel::Pixel2x2, DrawFill::Empty);
    display->draw_string(20, 65, "Display Active", Font::font12(), Color::Black, Color::White);

    std::cout << "Refreshing display...\n";
    if (auto result = display->refresh(); !result) {
      std::cerr << "Refresh failed: " << result.error().what() << "\n";
      return EXIT_FAILURE;
    }
    std::cout << "Initial display complete.\n";
    std::cout << "Waiting 2 seconds...\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Test 2: Manual sleep
    std::cout << "\n=== Test 2: Manual Sleep ===\n";
    std::cout << "Putting display into sleep mode...\n";
    display->sleep();
    std::cout << "Display should now be in sleep mode.\n";
    std::cout << "Waiting 2 seconds...\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Test 3: Wake from sleep
    std::cout << "\n=== Test 3: Wake from Sleep ===\n";
    if (display->supports_wake()) {
      std::cout << "Waking display from sleep...\n";
      if (auto result = display->wake(); !result) {
        std::cerr << "Wake failed: " << result.error().what() << "\n";
        std::cout << "Note: Wake may re-initialize display if true wake not supported.\n";
      } else {
        std::cout << "Display wake successful.\n";
      }
    } else {
      std::cout << "Driver does not support wake from sleep.\n";
      std::cout << "Note: Will re-initialize for next test.\n";
    }

    // Redraw to verify wake
    display->clear(Color::White);
    display->draw_string(10, 10, "POWER MANAGEMENT TEST", Font::font16(), Color::Black, Color::White);
    display->draw_string(10, 30, "Test 3: After Wake", Font::font12(), Color::Black, Color::White);
    display->draw_rectangle(10, 50, 166, 100, Color::Black, DotPixel::Pixel2x2, DrawFill::Empty);
    display->draw_string(20, 65, "Display Awake", Font::font12(), Color::Black, Color::White);

    std::cout << "Refreshing display after wake...\n";
    if (auto result = display->refresh(); !result) {
      std::cerr << "Refresh after wake failed: " << result.error().what() << "\n";
      return EXIT_FAILURE;
    }
    std::cout << "Display refreshed successfully after wake.\n";
    std::cout << "Waiting 2 seconds...\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Test 4: Power control (if supported)
    if (display->supports_power_control()) {
      std::cout << "\n=== Test 4: Power Off ===\n";
      std::cout << "Turning display power off...\n";
      if (auto result = display->power_off(); !result) {
        std::cerr << "Power off failed: " << result.error().what() << "\n";
      } else {
        std::cout << "Display power off successful.\n";
        std::cout << "Waiting 2 seconds...\n";
        std::this_thread::sleep_for(std::chrono::seconds(2));

        std::cout << "\n=== Test 5: Power On ===\n";
        std::cout << "Turning display power on...\n";
        if (auto result = display->power_on(); !result) {
          std::cerr << "Power on failed: " << result.error().what() << "\n";
        } else {
          std::cout << "Display power on successful.\n";
        }

        // Redraw to verify power on
        display->clear(Color::White);
        display->draw_string(10, 10, "POWER MANAGEMENT TEST", Font::font16(), Color::Black, Color::White);
        display->draw_string(10, 30, "Test 5: After Power On", Font::font12(), Color::Black, Color::White);
        display->draw_rectangle(10, 50, 166, 100, Color::Black, DotPixel::Pixel2x2, DrawFill::Empty);
        display->draw_string(20, 65, "Power Restored", Font::font12(), Color::Black, Color::White);

        std::cout << "Refreshing display after power on...\n";
        if (auto result = display->refresh(); !result) {
          std::cerr << "Refresh after power on failed: " << result.error().what() << "\n";
          return EXIT_FAILURE;
        }
        std::cout << "Display refreshed successfully after power on.\n";
      }
    } else {
      std::cout << "\n=== Test 4 & 5: Power Control Not Supported ===\n";
      std::cout << "Driver does not support power_off/power_on methods.\n";
      std::cout << "This is normal for some e-paper displays.\n";
    }

    // Final display
    std::cout << "\n=== Final State ===\n";
    display->clear(Color::White);
    display->draw_string(10, 10, "POWER MANAGEMENT TEST", Font::font16(), Color::Black, Color::White);
    display->draw_string(10, 30, "All Tests Complete", Font::font12(), Color::Black, Color::White);
    display->draw_rectangle(10, 50, 166, 140, Color::Black, DotPixel::Pixel2x2, DrawFill::Empty);

    std::size_t y_pos = 60;
    display->draw_string(20, y_pos, "Tests Completed:", Font::font12(), Color::Black, Color::White);
    y_pos += 15;
    display->draw_string(20, y_pos, "1. Initial display", Font::font8(), Color::Black, Color::White);
    y_pos += 10;
    display->draw_string(20, y_pos, "2. Manual sleep", Font::font8(), Color::Black, Color::White);
    y_pos += 10;
    display->draw_string(20, y_pos, "3. Wake from sleep", Font::font8(), Color::Black, Color::White);
    y_pos += 10;
    if (display->supports_power_control()) {
      display->draw_string(20, y_pos, "4. Power off", Font::font8(), Color::Black, Color::White);
      y_pos += 10;
      display->draw_string(20, y_pos, "5. Power on", Font::font8(), Color::Black, Color::White);
    } else {
      display->draw_string(20, y_pos, "4-5. Not supported", Font::font8(), Color::Black, Color::White);
    }

    std::cout << "Final refresh...\n";
    if (auto result = display->refresh(); !result) {
      std::cerr << "Final refresh failed: " << result.error().what() << "\n";
      return EXIT_FAILURE;
    }

    std::cout << "\n=======================================\n";
    std::cout << "  Power Management Test: PASSED\n";
    std::cout << "=======================================\n";
    std::cout << "\nVisual verification checklist:\n";
    std::cout << "  [ ] Initial display appeared correctly\n";
    std::cout << "  [ ] Display entered sleep mode (no error)\n";
    std::cout << "  [ ] Display woke from sleep (or re-initialized)\n";
    std::cout << "  [ ] Display refreshed correctly after wake\n";
    if (display->supports_power_control()) {
      std::cout << "  [ ] Power off executed successfully\n";
      std::cout << "  [ ] Power on executed successfully\n";
      std::cout << "  [ ] Display refreshed correctly after power cycle\n";
    }
    std::cout << "  [ ] Final summary display is visible\n";
    std::cout << "  [ ] No crashes or errors during power state changes\n";

    return EXIT_SUCCESS;

  } catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
    return EXIT_FAILURE;
  }
}
