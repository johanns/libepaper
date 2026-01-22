#include "test_config.hpp"
#include <chrono>
#include <cstdlib>
#include <epaper/core/device.hpp>
#include <epaper/core/display.hpp>
#include <epaper/graphics/font.hpp>
#include <iostream>
#include <thread>

using namespace epaper;

auto main() -> int {
  std::cout << "=======================================\n";
  std::cout << "  Final Test: Clear & Power Off\n";
  std::cout << "=======================================\n";
  std::cout << "Driver: " << TEST_DRIVER_NAME << "\n";
  std::cout << "This test clears the screen and powers off the display.\n\n";

  try {
    // Initialize device (skip for MockDriver since it doesn't use hardware)
    std::cout << "Initializing device...\n";
    Device device;

#if TEST_DRIVER == DRIVER_EPD27
    if (auto result = device.init(); !result) {
      std::cerr << "Device initialization failed: " << result.error().what() << "\n";
      return EXIT_FAILURE;
    }
#else
    std::cout << "Skipping device init for MockDriver (no hardware needed)\n";
#endif

    // Create display
    std::cout << "Creating display...\n";
    auto display = create_display<TestDriver, MonoFramebuffer>(device, DisplayMode::BlackWhite);
    if (!display) {
      std::cerr << "Display initialization failed: " << display.error().what() << "\n";
      return EXIT_FAILURE;
    }

    std::cout << "Display size: " << display->effective_width() << "x" << display->effective_height() << "\n";

    // Clear the screen
    std::cout << "\n=== Clearing Screen ===\n";
    display->clear(Color::White);
    std::cout << "Screen cleared to white.\n";

    // Optionally show a message before clearing (for visual confirmation)
    display->draw(display->text("FINAL TEST")
                      .at(10, 10)
                      .font(&Font::font16())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text("Clearing screen...")
                      .at(10, 30)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text("Powering off...")
                      .at(10, 50)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());

    std::cout << "Refreshing display with final message...\n";
    if (auto result = display->refresh(); !result) {
      std::cerr << "Refresh failed: " << result.error().what() << "\n";
      return EXIT_FAILURE;
    }
    std::cout << "Display refreshed.\n";
    std::cout << "Waiting 2 seconds before clearing and powering off...\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Clear the screen again and refresh to show white screen
    std::cout << "\n=== Final Clear ===\n";
    // Disable auto-sleep so we can manually control power off
    display->set_auto_sleep(false);
    display->clear(Color::White);
    std::cout << "Screen cleared to white.\n";
    std::cout << "Refreshing display to show cleared screen...\n";
    if (auto result = display->refresh(); !result) {
      std::cerr << "Refresh after clear failed: " << result.error().what() << "\n";
      return EXIT_FAILURE;
    }
    std::cout << "Display refreshed - screen should now be white.\n";
    std::cout << "Waiting 1 second before powering off...\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Power off the display
    std::cout << "\n=== Powering Off Display ===\n";
    if (display->supports_power_control()) {
      std::cout << "Power control is supported. Powering off...\n";
      if (auto result = display->power_off(); !result) {
        std::cerr << "Power off failed: " << result.error().what() << "\n";
        return EXIT_FAILURE;
      }
      std::cout << "Display powered off successfully.\n";
    } else {
      std::cout << "Power control not supported by driver.\n";
      std::cout << "Using sleep() instead for power management...\n";
      display->sleep();
      std::cout << "Display put into sleep mode.\n";
    }

    std::cout << "\n=======================================\n";
    std::cout << "  Final Test: PASSED\n";
    std::cout << "=======================================\n";
    std::cout << "\nTest completed successfully:\n";
    std::cout << "  [✓] Screen cleared\n";
    if (display->supports_power_control()) {
      std::cout << "  [✓] Display powered off\n";
    } else {
      std::cout << "  [✓] Display put into sleep mode (power control not supported)\n";
    }
    std::cout << "\nThe display should now be cleared and powered off.\n";

    return EXIT_SUCCESS;

  } catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
    return EXIT_FAILURE;
  }
}
