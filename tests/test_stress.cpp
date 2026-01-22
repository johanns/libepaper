#include "test_config.hpp"
#include <chrono>
#include <cstdlib>
#include <epaper/core/device.hpp>
#include <epaper/core/display.hpp>
#include <epaper/graphics/font.hpp>
#include <iostream>
#include <thread>
#include <vector>

using namespace epaper;

auto main() -> int {
  std::cout << "=======================================\n";
  std::cout << "  Stress Test\n";
  std::cout << "=======================================\n";
  std::cout << "This test performs rapid operations and stress conditions.\n";
  std::cout << "WARNING: This test takes several minutes to complete.\n\n";

  try {
    // Initialize device
    std::cout << "Initializing device...\n";
    Device device;
    if (auto result = device.init(); !result) {
      std::cerr << "Device initialization failed: " << result.error().what() << "\n";
      return EXIT_FAILURE;
    }

    // Create display with auto-sleep disabled for stress testing
    auto display =
        create_display<TestDriver, MonoFramebuffer>(device, DisplayMode::BlackWhite, Orientation::Portrait0, false);
    if (!display) {
      std::cerr << "Display initialization failed: " << display.error().what() << "\n";
      return EXIT_FAILURE;
    }

    const auto width = display->effective_width();
    const auto height = display->effective_height();
    std::cout << "Display size: " << width << "x" << height << "\n";
    std::cout << "Auto-sleep: " << (display->auto_sleep_enabled() ? "ON" : "OFF") << "\n\n";

    // Test 1: Rapid drawing operations
    std::cout << "=== Test 1: Rapid Drawing Operations ===\n";
    std::cout << "Performing 1000 rapid drawing operations...\n";

    auto start_time = std::chrono::steady_clock::now();

    display->clear(Color::White);
    display->draw(display->text("STRESS TEST")
                      .at(5, 5)
                      .font(&Font::font16())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text("Test 1: Rapid Draws")
                      .at(5, 25)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());

    for (int i = 0; i < 1000; ++i) {
      const auto x = static_cast<std::size_t>(i % width);
      const auto y = static_cast<std::size_t>(40 + (i % 100));
      display->set_pixel(x, y, (i % 2 == 0) ? Color::Black : Color::White);

      if (i % 100 == 0) {
        std::cout << "  Progress: " << i << "/1000\r" << std::flush;
      }
    }

    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::cout << "\n  Completed in " << duration.count() << " ms\n";
    display->draw(display->text("1000 draws: OK")
                      .at(5, 150)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    std::cout << "✓ Rapid drawing operations completed.\n\n";

    // Test 2: Multiple refreshes
    std::cout << "=== Test 2: Multiple Refreshes ===\n";
    std::cout << "Performing 5 consecutive refreshes...\n";
    std::cout << "NOTE: Each refresh takes several seconds.\n";

    start_time = std::chrono::steady_clock::now();

    for (int i = 1; i <= 5; ++i) {
      std::cout << "  Refresh " << i << "/5...\n";

      display->clear(Color::White);
      display->draw(display->text("STRESS TEST")
                        .at(5, 5)
                        .font(&Font::font16())
                        .foreground(Color::Black)
                        .background(Color::White)
                        .build());
      display->draw(display->text("Test 2: Refreshes")
                        .at(5, 25)
                        .font(&Font::font12())
                        .foreground(Color::Black)
                        .background(Color::White)
                        .build());

      std::string refresh_msg = "Refresh " + std::to_string(i) + " of 5";
      display->draw(display->rectangle()
                        .top_left(5, 45)
                        .bottom_right(171, 100)
                        .color(Color::Black)
                        .border_width(DotPixel::Pixel2x2)
                        .fill(DrawFill::Empty)
                        .build());
      display->draw(display->text(refresh_msg)
                        .at(20, 60)
                        .font(&Font::font16())
                        .foreground(Color::Black)
                        .background(Color::White)
                        .build());

      // Draw progress bar
      const std::size_t bar_width = 140;
      const std::size_t filled_width = (bar_width * i) / 5;
      display->draw(display->rectangle()
                        .top_left(20, 85)
                        .bottom_right(20 + bar_width, 95)
                        .color(Color::Black)
                        .border_width(DotPixel::Pixel1x1)
                        .fill(DrawFill::Empty)
                        .build());
      if (filled_width > 0) {
        display->draw(display->rectangle()
                          .top_left(21, 86)
                          .bottom_right(20 + filled_width, 94)
                          .color(Color::Black)
                          .border_width(DotPixel::Pixel1x1)
                          .fill(DrawFill::Full)
                          .build());
      }

      if (auto result = display->refresh(); !result) {
        std::cerr << "  Refresh " << i << " failed: " << result.error().what() << "\n";
        return EXIT_FAILURE;
      }
    }

    end_time = std::chrono::steady_clock::now();
    duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);

    std::cout << "  Completed in " << duration.count() << " seconds\n";
    std::cout << "✓ Multiple refreshes completed.\n\n";

    // Test 3: Complex scene rendering
    std::cout << "=== Test 3: Complex Scene Rendering ===\n";
    std::cout << "Rendering complex scene with many elements...\n";

    start_time = std::chrono::steady_clock::now();

    display->clear(Color::White);
    display->draw(display->text("STRESS TEST")
                      .at(5, 5)
                      .font(&Font::font16())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text("Test 3: Complex Scene")
                      .at(5, 25)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());

    // Draw grid of circles
    std::cout << "  Drawing circles...\n";
    for (std::size_t y = 50; y < height - 20; y += 20) {
      for (std::size_t x = 10; x < width - 10; x += 20) {
        display->draw(display->circle()
                          .center(x, y)
                          .radius(5)
                          .color(Color::Black)
                          .border_width(DotPixel::Pixel1x1)
                          .fill(DrawFill::Empty)
                          .build());
      }
    }

    // Draw grid of rectangles
    std::cout << "  Drawing rectangles...\n";
    for (std::size_t y = 55; y < height - 20; y += 20) {
      for (std::size_t x = 15; x < width - 10; x += 20) {
        display->draw(display->rectangle()
                          .top_left(x, y)
                          .bottom_right(x + 8, y + 8)
                          .color(Color::Black)
                          .border_width(DotPixel::Pixel1x1)
                          .fill(DrawFill::Full)
                          .build());
      }
    }

    end_time = std::chrono::steady_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::cout << "  Rendering completed in " << duration.count() << " ms\n";
    std::cout << "  Refreshing complex scene...\n";

    if (auto result = display->refresh(); !result) {
      std::cerr << "  Refresh failed: " << result.error().what() << "\n";
      return EXIT_FAILURE;
    }
    std::cout << "✓ Complex scene rendering completed.\n\n";

    // Test 4: Memory stress (large bitmap)
    std::cout << "=== Test 4: Memory Stress Test ===\n";
    std::cout << "Creating and drawing large bitmap...\n";

    start_time = std::chrono::steady_clock::now();

    display->clear(Color::White);
    display->draw(display->text("STRESS TEST")
                      .at(5, 5)
                      .font(&Font::font16())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text("Test 4: Memory Stress")
                      .at(5, 25)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());

    // Create a large bitmap (as large as the display)
    std::cout << "  Allocating large bitmap (" << width << "x" << height << " pixels)...\n";
    std::vector<Color> large_bitmap;
    large_bitmap.reserve(width * height);

    for (std::size_t y = 0; y < height; ++y) {
      for (std::size_t x = 0; x < width; ++x) {
        // Create checkerboard pattern
        const bool is_white = ((x / 8) + (y / 8)) % 2 == 0;
        large_bitmap.push_back(is_white ? Color::White : Color::Black);
      }
      if (y % 50 == 0) {
        std::cout << "  Progress: " << y << "/" << height << "\r" << std::flush;
      }
    }

    std::cout << "\n  Drawing large bitmap...\n";
    display->draw_bitmap(0, 0, large_bitmap, width, height);

    end_time = std::chrono::steady_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::cout << "  Completed in " << duration.count() << " ms\n";
    std::cout << "  Refreshing...\n";

    if (auto result = display->refresh(); !result) {
      std::cerr << "  Refresh failed: " << result.error().what() << "\n";
      return EXIT_FAILURE;
    }
    std::cout << "✓ Memory stress test completed.\n\n";

    // Test 5: Rapid clear operations
    std::cout << "=== Test 5: Rapid Clear Operations ===\n";
    std::cout << "Performing rapid clear operations...\n";

    start_time = std::chrono::steady_clock::now();

    for (int i = 0; i < 100; ++i) {
      display->clear(Color::White);
      display->clear(Color::Black);
      if (i % 10 == 0) {
        std::cout << "  Progress: " << i << "/100\r" << std::flush;
      }
    }

    end_time = std::chrono::steady_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::cout << "\n  Completed in " << duration.count() << " ms\n";
    std::cout << "✓ Rapid clear operations completed.\n\n";

    // Final summary display
    std::cout << "=== All Stress Tests Complete ===\n";
    display->clear(Color::White);
    display->draw(display->text("STRESS TEST")
                      .at(5, 5)
                      .font(&Font::font16())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    display->draw(display->text("All Tests Complete!")
                      .at(5, 25)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());

    display->draw(display->rectangle()
                      .top_left(5, 45)
                      .bottom_right(171, 220)
                      .color(Color::Black)
                      .border_width(DotPixel::Pixel2x2)
                      .fill(DrawFill::Empty)
                      .build());

    std::size_t y_pos = 55;
    display->draw(display->text("Completed Tests:")
                      .at(10, y_pos)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    y_pos += 18;

    display->draw(display->text("1. Rapid drawing")
                      .at(10, y_pos)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    y_pos += 12;
    display->draw(display->text("1000 operations")
                      .at(15, y_pos)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    y_pos += 18;

    display->draw(display->text("2. Multiple refreshes")
                      .at(10, y_pos)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    y_pos += 12;
    display->draw(display->text("5 consecutive")
                      .at(15, y_pos)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    y_pos += 18;

    display->draw(display->text("3. Complex scene")
                      .at(10, y_pos)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    y_pos += 12;
    display->draw(display->text("Many elements")
                      .at(15, y_pos)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    y_pos += 18;

    display->draw(display->text("4. Memory stress")
                      .at(10, y_pos)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    y_pos += 12;
    display->draw(display->text("Large bitmap")
                      .at(15, y_pos)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    y_pos += 18;

    display->draw(display->text("5. Rapid clears")
                      .at(10, y_pos)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    y_pos += 12;
    display->draw(display->text("100 operations")
                      .at(15, y_pos)
                      .font(&Font::font8())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());
    y_pos += 20;

    display->draw(display->text("System stable!")
                      .at(10, y_pos)
                      .font(&Font::font12())
                      .foreground(Color::Black)
                      .background(Color::White)
                      .build());

    std::cout << "Final refresh...\n";
    if (auto result = display->refresh(); !result) {
      std::cerr << "Final refresh failed: " << result.error().what() << "\n";
      return EXIT_FAILURE;
    }

    std::cout << "\n=======================================\n";
    std::cout << "  Stress Test: PASSED\n";
    std::cout << "=======================================\n";
    std::cout << "\nVisual verification checklist:\n";
    std::cout << "  [ ] All 5 stress tests completed without crashes\n";
    std::cout << "  [ ] Rapid drawing operations performed correctly\n";
    std::cout << "  [ ] Multiple refreshes executed successfully\n";
    std::cout << "  [ ] Complex scene rendered without artifacts\n";
    std::cout << "  [ ] Large bitmap created and displayed\n";
    std::cout << "  [ ] Rapid clear operations completed\n";
    std::cout << "  [ ] No memory leaks (monitor RAM if possible)\n";
    std::cout << "  [ ] System remained stable throughout\n";
    std::cout << "  [ ] Final summary display is visible\n";

    return EXIT_SUCCESS;

  } catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
    return EXIT_FAILURE;
  }
}
