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

    auto display = create_display<TestDriver, MonoFramebuffer>(device, DisplayMode::BlackWhite);
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
    display->draw(display->text("AUTO-SLEEP TEST")
                      .at(10, 10)
                      .font(&Font::font16())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text("Test 1: Enabled")
                      .at(10, 30)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text("Auto-sleep: ON")
                      .at(10, 50)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->rectangle()
                      .top_left(10, 70)
                      .bottom_right(166, 110)
                      .color(Color::Black)
                      .border_width(DotPixel::Pixel1x1)
                      .fill(DrawFill::Empty)
                      .build());
    display->draw(display->text("Display will sleep")
                      .at(15, 80)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text("automatically after")
                      .at(15, 92)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text("this refresh.")
                      .at(15, 104)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());

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
    display->draw(display->text("AUTO-SLEEP TEST")
                      .at(10, 10)
                      .font(&Font::font16())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text("Test 2: Disabled")
                      .at(10, 30)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text("Auto-sleep: OFF")
                      .at(10, 50)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->rectangle()
                      .top_left(10, 70)
                      .bottom_right(166, 110)
                      .color(Color::Black)
                      .border_width(DotPixel::Pixel1x1)
                      .fill(DrawFill::Empty)
                      .build());
    display->draw(display->text("Display will NOT")
                      .at(15, 80)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text("sleep automatically")
                      .at(15, 92)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text("after this refresh.")
                      .at(15, 104)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());

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
      display->draw(display->text("AUTO-SLEEP TEST")
                        .at(10, 10)
                        .font(&Font::font16())
                        .foreground(Color::Black)
                        .background(Color::White)
                        .build());
      display->draw(display->text("Test 3: Multiple")
                        .at(10, 30)
                        .font(&Font::font12())
                        .foreground(Color::Black)
                        .background(Color::White)
                        .build());
      display->draw(display->text("Auto-sleep: OFF")
                        .at(10, 50)
                        .font(&Font::font12())
                        .foreground(Color::Black)
                        .background(Color::White)
                        .build());

      std::string refresh_msg = "Refresh " + std::to_string(i) + " of 3";
      display->draw(display->rectangle()
                        .top_left(10, 70)
                        .bottom_right(166, 120)
                        .color(Color::Black)
                        .border_width(DotPixel::Pixel2x2)
                        .fill(DrawFill::Empty)
                        .build());
      display->draw(display->text(refresh_msg)
                        .at(20, 85)
                        .font(&Font::font12())
                        .foreground(Color::Black)
                        .background(Color::White)
                        .build());

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
    display->draw(display->text("AUTO-SLEEP TEST")
                      .at(10, 10)
                      .font(&Font::font16())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text("Test 4: Re-enabled")
                      .at(10, 30)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text("Auto-sleep: ON")
                      .at(10, 50)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->rectangle()
                      .top_left(10, 70)
                      .bottom_right(166, 110)
                      .color(Color::Black)
                      .border_width(DotPixel::Pixel1x1)
                      .fill(DrawFill::Empty)
                      .build());
    display->draw(display->text("Display will sleep")
                      .at(15, 80)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text("automatically after")
                      .at(15, 92)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text("this refresh.")
                      .at(15, 104)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());

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
    display->draw(display->text("AUTO-SLEEP TEST")
                      .at(10, 5)
                      .font(&Font::font16())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text("Complete!")
                      .at(10, 25)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());

    display->draw(display->rectangle()
                      .top_left(5, 45)
                      .bottom_right(171, 230)
                      .color(Color::Black)
                      .border_width(DotPixel::Pixel1x1)
                      .fill(DrawFill::Empty)
                      .build());

    std::size_t y_pos = 55;
    display->draw(display->text("Tests Completed:")
                      .at(10, y_pos)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    y_pos += 15;

    display->draw(display->text("1. Default: ENABLED")
                      .at(10, y_pos)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    y_pos += 12;
    display->draw(display->text("Auto-sleep on by default")
                      .at(15, y_pos)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    y_pos += 15;

    display->draw(display->text("2. Disabled: OFF")
                      .at(10, y_pos)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    y_pos += 12;
    display->draw(display->text("Can be turned off")
                      .at(15, y_pos)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    y_pos += 15;

    display->draw(display->text("3. Multiple refreshes")
                      .at(10, y_pos)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    y_pos += 12;
    display->draw(display->text("Works without auto-sleep")
                      .at(15, y_pos)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    y_pos += 15;

    display->draw(display->text("4. Re-enabled: ON")
                      .at(10, y_pos)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    y_pos += 12;
    display->draw(display->text("Can be turned back on")
                      .at(15, y_pos)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    y_pos += 20;

    display->draw(display->text("Recommendation:")
                      .at(10, y_pos)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    y_pos += 15;
    display->draw(display->text("Keep auto-sleep ENABLED")
                      .at(10, y_pos)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    y_pos += 10;
    display->draw(display->text("to prevent burn-in.")
                      .at(10, y_pos)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());

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
    auto display2 =
        create_display<TestDriver, MonoFramebuffer>(device, DisplayMode::BlackWhite, Orientation::Portrait0, true);

    if (!display2) {
      std::cerr << "Failed to create display for transparent wake test: " << display2.error().what() << "\n";
      return EXIT_FAILURE;
    }

    std::cout << "First render (display will sleep after)...\n";
    display2->clear();
    display2->draw(display2->text("FIRST RENDER")
                       .at(10, 10)
                       .font(&Font::font16())
                       .foreground(Color::Black)
                       .background(Color::White)
                       .build());
    display2->draw(display2->text("Display sleeps")
                       .at(10, 30)
                       .font(&Font::font12())
                       .foreground(Color::Black)
                       .background(Color::White)
                       .build());
    display2->draw(display2->text("after this")
                       .at(10, 45)
                       .font(&Font::font12())
                       .foreground(Color::Black)
                       .background(Color::White)
                       .build());
    if (auto result = display2->refresh(); !result) {
      std::cerr << "First refresh failed: " << result.error().what() << "\n";
      return EXIT_FAILURE;
    }
    std::cout << "First refresh complete. Display is now in sleep mode.\n";
    std::cout << "Waiting 3 seconds...\n";
    std::this_thread::sleep_for(std::chrono::seconds(3));

    std::cout << "\nSecond render (should auto-wake transparently)...\n";
    display2->clear();
    display2->draw(display2->text("SECOND RENDER")
                       .at(10, 10)
                       .font(&Font::font16())
                       .foreground(Color::Black)
                       .background(Color::White)
                       .build());
    display2->draw(display2->text("Auto-wake")
                       .at(10, 30)
                       .font(&Font::font12())
                       .foreground(Color::Black)
                       .background(Color::White)
                       .build());
    display2->draw(display2->text("worked!")
                       .at(10, 45)
                       .font(&Font::font12())
                       .foreground(Color::Black)
                       .background(Color::White)
                       .build());
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
