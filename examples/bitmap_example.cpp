#include <epaper/device.hpp>
#include <epaper/draw.hpp>
#include <epaper/drivers/epd27.hpp>
#include <epaper/screen.hpp>
#include <iostream>

using namespace epaper;

int main() {
  // Initialize device
  Device device;
  if (auto result = device.init(); !result) {
    std::cerr << "Failed to initialize device\n";
    return 1;
  }

  // Create driver for 2.7" display
  EPD27 epd27(device);

  // Initialize in black/white mode
  if (auto result = epd27.init(DisplayMode::BlackWhite); !result) {
    std::cerr << "Failed to initialize display\n";
    return 1;
  }

  epd27.clear();

  // Create screen and drawing interface
  Screen screen(epd27);
  Draw draw(screen);

  // Example 1: Draw a simple checkerboard pattern using raw pixel data
  constexpr std::size_t pattern_size = 32;
  std::vector<Color> checkerboard;
  checkerboard.reserve(pattern_size * pattern_size);

  for (std::size_t y = 0; y < pattern_size; ++y) {
    for (std::size_t x = 0; x < pattern_size; ++x) {
      // Create 4x4 checkerboard
      const bool is_white = ((x / 4) + (y / 4)) % 2 == 0;
      checkerboard.push_back(is_white ? Color::White : Color::Black);
    }
  }

  // Draw checkerboard at original size
  draw.draw_bitmap(10, 10, checkerboard, pattern_size, pattern_size);

  // Draw checkerboard scaled 2x
  draw.draw_bitmap(50, 10, checkerboard, pattern_size, pattern_size, pattern_size * 2, pattern_size * 2);

  // Example 2: Draw a gradient pattern
  std::vector<Color> gradient;
  gradient.reserve(64 * 16);

  for (std::size_t y = 0; y < 16; ++y) {
    for (std::size_t x = 0; x < 64; ++x) {
      // Create horizontal gradient
      if (x < 16) {
        gradient.push_back(Color::Black);
      } else if (x < 32) {
        gradient.push_back(Color::Gray2);
      } else if (x < 48) {
        gradient.push_back(Color::Gray1);
      } else {
        gradient.push_back(Color::White);
      }
    }
  }

  draw.draw_bitmap(10, 120, gradient, 64, 16);

  // Example 3: Draw from PNG file
  std::cout << "Loading test images...\n";
  if (auto result = draw.draw_bitmap_from_file(10, 150, "images/logo.png"); result) {
    std::cout << "  ✓ Loaded logo.png\n";
  } else {
    std::cerr << "  ✗ Failed to load logo.png\n";
  }

  // Example 4: Draw scaled icon
  if (auto result = draw.draw_bitmap_from_file(100, 150, "images/icon_battery.png", 48, 24); result) {
    std::cout << "  ✓ Loaded and scaled icon_battery.png (48x24)\n";
  } else {
    std::cerr << "  ✗ Failed to load battery icon\n";
  }

  // Example 5: Draw from JPEG file
  if (auto result = draw.draw_bitmap_from_file(10, 180, "images/circles.jpg", 50, 50); result) {
    std::cout << "  ✓ Loaded circles.jpg (50x50)\n";
  } else {
    std::cerr << "  ✗ Failed to load circles.jpg\n";
  }

  // Example 6: Draw from BMP file
  if (auto result = draw.draw_bitmap_from_file(70, 180, "images/checkerboard_64.bmp", 40, 40); result) {
    std::cout << "  ✓ Loaded checkerboard_64.bmp (40x40)\n";
  } else {
    std::cerr << "  ✗ Failed to load checkerboard_64.bmp\n";
  }

  // Add some text
  draw.draw_string(10, 200, "Bitmap Drawing Demo", Font::font16(), Color::Black, Color::White);

  // Refresh display
  screen.refresh();

  // Wait a bit
  device.delay_ms(5000);

  // Put display to sleep
  epd27.sleep();

  std::cout << "Bitmap drawing demo completed successfully!\n";
  return 0;
}

