#include "epaper/device.hpp"
#include "epaper/display.hpp"
#include "epaper/drivers/epd27.hpp"
#include "epaper/font.hpp"
#include <cstdlib>
#include <expected>
#include <iostream>
#include <thread>

using namespace epaper;

auto demo_black_white(Display &display) -> void {
  std::cout << "Running Black & White demo...\n";

  // Clear screen
  display.clear(Color::White);

  // Draw points
  display.draw_point(10, 80, Color::Black, DotPixel::Pixel1x1);
  display.draw_point(10, 90, Color::Black, DotPixel::Pixel2x2);
  display.draw_point(10, 100, Color::Black, DotPixel::Pixel3x3);

  // Draw lines
  display.draw_line(20, 70, 70, 120, Color::Black, DotPixel::Pixel1x1, LineStyle::Solid);
  display.draw_line(70, 70, 20, 120, Color::Black, DotPixel::Pixel1x1, LineStyle::Solid);

  // Draw rectangles
  display.draw_rectangle(20, 70, 70, 120, Color::Black, DotPixel::Pixel1x1, DrawFill::Empty);
  display.draw_rectangle(80, 70, 130, 120, Color::Black, DotPixel::Pixel1x1, DrawFill::Full);

  // Draw circles
  display.draw_circle(45, 95, 20, Color::Black, DotPixel::Pixel1x1, DrawFill::Empty);
  display.draw_circle(105, 95, 20, Color::White, DotPixel::Pixel1x1, DrawFill::Full);

  // Draw dotted line
  display.draw_line(85, 95, 125, 95, Color::Black, DotPixel::Pixel1x1, LineStyle::Dotted);
  display.draw_line(105, 75, 105, 115, Color::Black, DotPixel::Pixel1x1, LineStyle::Dotted);

  // Draw text
  display.draw_string(10, 0, "Waveshare", Font::font16(), Color::Black, Color::White);
  display.draw_string(10, 20, "Hello World", Font::font12(), Color::White, Color::Black);

  // Draw numbers
  display.draw_number(10, 33, 123456789, Font::font12(), Color::Black, Color::White);
  display.draw_number(10, 50, 987654321, Font::font16(), Color::White, Color::Black);

  // Refresh display
  std::cout << "Refreshing display...\n";
  display.refresh();

  std::cout << "Black & White demo complete.\n";
}

auto demo_grayscale(Display &display) -> void {
  std::cout << "Running Grayscale demo...\n";

  // Clear screen
  display.clear(Color::White);

  // Draw points in different gray levels
  display.draw_point(10, 80, Color::Gray1, DotPixel::Pixel1x1);
  display.draw_point(10, 90, Color::Gray1, DotPixel::Pixel2x2);
  display.draw_point(10, 100, Color::Gray1, DotPixel::Pixel3x3);

  // Draw lines
  display.draw_line(20, 70, 70, 120, Color::Gray1, DotPixel::Pixel1x1, LineStyle::Solid);
  display.draw_line(70, 70, 20, 120, Color::Gray1, DotPixel::Pixel1x1, LineStyle::Solid);

  // Draw rectangles with different fills
  display.draw_rectangle(20, 70, 70, 120, Color::Gray1, DotPixel::Pixel1x1, DrawFill::Empty);
  display.draw_rectangle(80, 70, 130, 120, Color::Gray1, DotPixel::Pixel1x1, DrawFill::Full);

  // Draw circles
  display.draw_circle(45, 95, 20, Color::Gray1, DotPixel::Pixel1x1, DrawFill::Empty);
  display.draw_circle(105, 95, 20, Color::Gray2, DotPixel::Pixel1x1, DrawFill::Full);

  // Draw text with gray levels
  display.draw_string(10, 0, "Waveshare", Font::font16(), Color::Gray1, Color::White);
  display.draw_string(10, 20, "Hello World", Font::font12(), Color::Gray2, Color::White);

  // Draw numbers
  display.draw_number(10, 33, 123456789, Font::font12(), Color::Gray1, Color::Gray2);
  display.draw_number(10, 50, 987654321, Font::font16(), Color::White, Color::Gray1);

  // Draw decimal number
  display.draw_decimal(10, 130, 3.14159, 3, Font::font12(), Color::Gray1, Color::White);

  // Refresh display
  std::cout << "Refreshing display...\n";
  display.refresh();

  std::cout << "Grayscale demo complete.\n";
}

auto main() -> int {
  try {
    std::cout << "Modern C++ E-Paper Display Demo\n";
    std::cout << "================================\n\n";

    // Initialize BCM2835 device
    std::cout << "Initializing BCM2835 device...\n";
    Device device;

    if (auto result = device.init(); !result) {
      std::cerr << "Failed to initialize device: " << to_string(result.error()) << "\n";
      return EXIT_FAILURE;
    }

    std::cout << "Device initialized successfully.\n\n";

    // Black & White Mode Demo
    std::cout << "\n--- Black & White Mode ---\n";
    auto display_bw = create_display<EPD27>(device, DisplayMode::BlackWhite, Orientation::Landscape90);
    if (!display_bw) {
      std::cerr << "Failed to initialize display: " << to_string(display_bw.error()) << "\n";
      return EXIT_FAILURE;
    }

    std::cout << "Display initialized (B/W mode).\n";
    std::cout << "Display size: " << display_bw->width() << "x" << display_bw->height() << " pixels\n";
    std::cout << "Effective size: " << display_bw->effective_width() << "x" << display_bw->effective_height()
              << " pixels\n\n";

    // Run black & white demo
    demo_black_white(*display_bw);

    // Wait before switching modes
    std::cout << "\nWaiting 3 seconds...\n";
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // Grayscale Mode Demo
    std::cout << "\n--- 4-Level Grayscale Mode ---\n";
    auto display_gray = create_display<EPD27>(device, DisplayMode::Grayscale4);
    if (!display_gray) {
      std::cerr << "Failed to initialize grayscale mode: " << to_string(display_gray.error()) << "\n";
      return EXIT_FAILURE;
    }

    std::cout << "Display initialized (Grayscale mode).\n\n";

    // Run grayscale demo
    demo_grayscale(*display_gray);

    display_gray->sleep();
    // Note: Display automatically enters sleep mode after refresh (auto-sleep enabled by default)
    std::cout << "\nDemo completed successfully!\n";
    return EXIT_SUCCESS;

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return EXIT_FAILURE;
  } catch (...) {
    std::cerr << "Unknown error occurred.\n";
    return EXIT_FAILURE;
  }
}
