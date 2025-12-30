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
  std::cout << "  Auto-Sleep Test\n";
  std::cout << "=======================================\n";
  std::cout << "This test verifies auto-sleep functionality.\n\n";

  try {
    // Initialize device
    std::cout << "Initializing device...\n";
    Device device;
    if (auto result = device.init(); !result) {
      std::cerr << "Device initialization failed: " << result.error().what() << "\n";
      return EXIT_FAILURE;
    }

    // Test 1: Verify auto-sleep is enabled by default
    std::cout << "\n=== Test 1: Auto-Sleep Enabled by Default ===\n";

    auto display = create_display<EPD27>(device, DisplayMode::BlackWhite);
    if (!display) {
      std::cerr << "Display initialization failed: " << display.error().what() << "\n";
      return EXIT_FAILURE;
    }

    std::cout << "Auto-sleep enabled: " << (display->auto_sleep_enabled() ? "YES" : "NO") << "\n";
    if (!display->auto_sleep_enabled()) {
      std::cerr << "ERROR: Auto-sleep should be enabled by default!\n";
      return EXIT_FAILURE;
    }
    std::cout << "✓ Auto-sleep is enabled by default.\n";

    // Display test message
    display->clear(Color::White);
    display->draw_string(10, 10, "AUTO-SLEEP TEST", Font::font16(), Color::Black, Color::White);
    display->draw_string(10, 30, "Test 1: Enabled", Font::font12(), Color::Black, Color::White);
    display->draw_string(10, 50, "Auto-sleep: ON", Font::font12(), Color::Black, Color::White);
    display->draw_rectangle(10, 70, 166, 110, Color::Black, DotPixel::Pixel1x1, DrawFill::Empty);
    display->draw_string(15, 80, "Display will sleep", Font::font8(), Color::Black, Color::White);
    display->draw_string(15, 92, "automatically after", Font::font8(), Color::Black, Color::White);
    display->draw_string(15, 104, "this refresh.", Font::font8(), Color::Black, Color::White);

    std::cout << "Refreshing (should auto-sleep after)...\n";
    if (auto result = display->refresh(); !result) {
      std::cerr << "Refresh failed: " << result.error().what() << "\n";
      return EXIT_FAILURE;
    }
    std::cout << "✓ Refresh complete. Display should have automatically entered sleep mode.\n";
    std::cout << "Waiting 2 seconds...\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Test 2: Disable auto-sleep
    std::cout << "\n=== Test 2: Disable Auto-Sleep ===\n";
    display->set_auto_sleep(false);
    std::cout << "Auto-sleep enabled: " << (display->auto_sleep_enabled() ? "YES" : "NO") << "\n";
    if (display->auto_sleep_enabled()) {
      std::cerr << "ERROR: Auto-sleep should be disabled!\n";
      return EXIT_FAILURE;
    }
    std::cout << "✓ Auto-sleep disabled successfully.\n";

    // Display test message
    display->clear(Color::White);
    display->draw_string(10, 10, "AUTO-SLEEP TEST", Font::font16(), Color::Black, Color::White);
    display->draw_string(10, 30, "Test 2: Disabled", Font::font12(), Color::Black, Color::White);
    display->draw_string(10, 50, "Auto-sleep: OFF", Font::font12(), Color::Black, Color::White);
    display->draw_rectangle(10, 70, 166, 110, Color::Black, DotPixel::Pixel1x1, DrawFill::Empty);
    display->draw_string(15, 80, "Display will NOT", Font::font8(), Color::Black, Color::White);
    display->draw_string(15, 92, "sleep automatically", Font::font8(), Color::Black, Color::White);
    display->draw_string(15, 104, "after this refresh.", Font::font8(), Color::Black, Color::White);

    std::cout << "Refreshing (should NOT auto-sleep)...\n";
    if (auto result = display->refresh(); !result) {
      std::cerr << "Refresh failed: " << result.error().what() << "\n";
      return EXIT_FAILURE;
    }
    std::cout << "✓ Refresh complete. Display should still be active (not sleeping).\n";
    std::cout << "Waiting 2 seconds...\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Test 3: Multiple refreshes with auto-sleep disabled
    std::cout << "\n=== Test 3: Multiple Refreshes (Auto-Sleep OFF) ===\n";
    std::cout << "Performing 3 rapid refreshes with auto-sleep disabled...\n";

    for (int i = 1; i <= 3; ++i) {
      display->clear(Color::White);
      display->draw_string(10, 10, "AUTO-SLEEP TEST", Font::font16(), Color::Black, Color::White);
      display->draw_string(10, 30, "Test 3: Multiple", Font::font12(), Color::Black, Color::White);
      display->draw_string(10, 50, "Auto-sleep: OFF", Font::font12(), Color::Black, Color::White);

      std::string refresh_msg = "Refresh " + std::to_string(i) + " of 3";
      display->draw_rectangle(10, 70, 166, 120, Color::Black, DotPixel::Pixel2x2, DrawFill::Empty);
      display->draw_string(20, 85, refresh_msg, Font::font12(), Color::Black, Color::White);

      std::cout << "  Refresh " << i << " of 3...\n";
      if (auto result = display->refresh(); !result) {
        std::cerr << "Refresh " << i << " failed: " << result.error().what() << "\n";
        return EXIT_FAILURE;
      }
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "✓ Multiple refreshes completed successfully without auto-sleep.\n";

    // Test 4: Re-enable auto-sleep
    std::cout << "\n=== Test 4: Re-enable Auto-Sleep ===\n";
    display->set_auto_sleep(true);
    std::cout << "Auto-sleep enabled: " << (display->auto_sleep_enabled() ? "YES" : "NO") << "\n";
    if (!display->auto_sleep_enabled()) {
      std::cerr << "ERROR: Auto-sleep should be enabled!\n";
      return EXIT_FAILURE;
    }
    std::cout << "✓ Auto-sleep re-enabled successfully.\n";

    // Display test message
    display->clear(Color::White);
    display->draw_string(10, 10, "AUTO-SLEEP TEST", Font::font16(), Color::Black, Color::White);
    display->draw_string(10, 30, "Test 4: Re-enabled", Font::font12(), Color::Black, Color::White);
    display->draw_string(10, 50, "Auto-sleep: ON", Font::font12(), Color::Black, Color::White);
    display->draw_rectangle(10, 70, 166, 110, Color::Black, DotPixel::Pixel1x1, DrawFill::Empty);
    display->draw_string(15, 80, "Display will sleep", Font::font8(), Color::Black, Color::White);
    display->draw_string(15, 92, "automatically after", Font::font8(), Color::Black, Color::White);
    display->draw_string(15, 104, "this refresh.", Font::font8(), Color::Black, Color::White);

    std::cout << "Refreshing (should auto-sleep after)...\n";
    if (auto result = display->refresh(); !result) {
      std::cerr << "Refresh failed: " << result.error().what() << "\n";
      return EXIT_FAILURE;
    }
    std::cout << "✓ Refresh complete. Display should have automatically entered sleep mode.\n";
    std::cout << "Waiting 2 seconds...\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Final summary
    std::cout << "\n=== Test 5: Summary ===\n";

    // May need to wake or re-init display for final refresh
    if (display->supports_wake()) {
      if (auto result = display->wake(); !result) {
        std::cout << "Note: Wake returned error (may be re-initializing): " << result.error().what() << "\n";
      }
    }

    display->clear(Color::White);
    display->draw_string(10, 5, "AUTO-SLEEP TEST", Font::font16(), Color::Black, Color::White);
    display->draw_string(10, 25, "Complete!", Font::font12(), Color::Black, Color::White);

    display->draw_rectangle(5, 45, 171, 230, Color::Black, DotPixel::Pixel1x1, DrawFill::Empty);

    std::size_t y_pos = 55;
    display->draw_string(10, y_pos, "Tests Completed:", Font::font12(), Color::Black, Color::White);
    y_pos += 15;

    display->draw_string(10, y_pos, "1. Default: ENABLED", Font::font8(), Color::Black, Color::White);
    y_pos += 12;
    display->draw_string(15, y_pos, "Auto-sleep on by default", Font::font8(), Color::Black, Color::White);
    y_pos += 15;

    display->draw_string(10, y_pos, "2. Disabled: OFF", Font::font8(), Color::Black, Color::White);
    y_pos += 12;
    display->draw_string(15, y_pos, "Can be turned off", Font::font8(), Color::Black, Color::White);
    y_pos += 15;

    display->draw_string(10, y_pos, "3. Multiple refreshes", Font::font8(), Color::Black, Color::White);
    y_pos += 12;
    display->draw_string(15, y_pos, "Works without auto-sleep", Font::font8(), Color::Black, Color::White);
    y_pos += 15;

    display->draw_string(10, y_pos, "4. Re-enabled: ON", Font::font8(), Color::Black, Color::White);
    y_pos += 12;
    display->draw_string(15, y_pos, "Can be turned back on", Font::font8(), Color::Black, Color::White);
    y_pos += 20;

    display->draw_string(10, y_pos, "Recommendation:", Font::font12(), Color::Black, Color::White);
    y_pos += 15;
    display->draw_string(10, y_pos, "Keep auto-sleep ENABLED", Font::font8(), Color::Black, Color::White);
    y_pos += 10;
    display->draw_string(10, y_pos, "to prevent burn-in.", Font::font8(), Color::Black, Color::White);

    std::cout << "Final refresh...\n";
    if (auto result = display->refresh(); !result) {
      std::cerr << "Final refresh failed: " << result.error().what() << "\n";
      return EXIT_FAILURE;
    }

    std::cout << "\n=======================================\n";
    std::cout << "  Auto-Sleep Test: PASSED\n";
    std::cout << "=======================================\n";
    std::cout << "\nVisual verification checklist:\n";
    std::cout << "  [ ] Test 1 confirmed auto-sleep enabled by default\n";
    std::cout << "  [ ] Test 2 confirmed auto-sleep can be disabled\n";
    std::cout << "  [ ] Test 3 performed multiple refreshes without auto-sleep\n";
    std::cout << "  [ ] Test 4 confirmed auto-sleep can be re-enabled\n";
    std::cout << "  [ ] No crashes or errors during state changes\n";
    std::cout << "  [ ] Final summary display is visible\n";
    std::cout << "\nNOTE: Auto-sleep should be kept ENABLED in production\n";
    std::cout << "      to prevent screen burn-in as recommended by manufacturer.\n";

    // Test 6: Transparent wake (multiple renders with auto-sleep enabled)
    std::cout << "\n=== Test 6: Transparent Wake Management ===\n";
    std::cout << "Testing multiple refreshes with auto-sleep enabled...\n";
    auto display2 = create_display<EPD27>(device, DisplayMode::BlackWhite, Orientation::Portrait0, true);

    if (!display2) {
      std::cerr << "Failed to create display for transparent wake test: " << display2.error().what() << "\n";
      return EXIT_FAILURE;
    }

    std::cout << "First render (display will sleep after)...\n";
    display2->clear();
    display2->draw_string(10, 10, "FIRST RENDER", Font::font16(), Color::Black, Color::White);
    display2->draw_string(10, 30, "Display sleeps", Font::font12(), Color::Black, Color::White);
    display2->draw_string(10, 45, "after this", Font::font12(), Color::Black, Color::White);
    if (auto result = display2->refresh(); !result) {
      std::cerr << "First refresh failed: " << result.error().what() << "\n";
      return EXIT_FAILURE;
    }
    std::cout << "First refresh complete. Display is now in sleep mode.\n";
    std::cout << "Waiting 3 seconds...\n";
    std::this_thread::sleep_for(std::chrono::seconds(3));

    std::cout << "\nSecond render (should auto-wake transparently)...\n";
    display2->clear();
    display2->draw_string(10, 10, "SECOND RENDER", Font::font16(), Color::Black, Color::White);
    display2->draw_string(10, 30, "Auto-wake", Font::font12(), Color::Black, Color::White);
    display2->draw_string(10, 45, "worked!", Font::font12(), Color::Black, Color::White);
    if (auto result = display2->refresh(); !result) {
      std::cerr << "Second refresh failed: " << result.error().what() << "\n";
      return EXIT_FAILURE;
    }
    std::cout << "Second refresh complete - transparent wake successful!\n";
    std::cout << "Test 6 passed!\n";

    std::cout << "\nSUCCESS: All tests completed successfully!\n";
    std::cout << "Transparent sleep/wake management is working correctly.\n";

    return EXIT_SUCCESS;

  } catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
    return EXIT_FAILURE;
  }
}
