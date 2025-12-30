/**
 * @file bmp_debug_example.cpp
 * @brief Example demonstrating BMP export for debugging display layouts
 *
 * This example shows how to save framebuffer contents to BMP files
 * for debugging purposes without waiting for slow display refresh.
 */

#include <epaper/device.hpp>
#include <epaper/display.hpp>
#include <epaper/drivers/epd27.hpp>
#include <epaper/font.hpp>
#include <iostream>

using namespace epaper;

int main() {
  std::cout << "BMP Debug Export Example\n";
  std::cout << "========================\n\n";

  // Initialize device
  std::cout << "Initializing device...\n";
  Device device;
  if (auto result = device.init(); !result) {
    std::cerr << "Device init failed: " << result.error().what() << "\n";
    return 1;
  }

  // Create display in landscape mode
  std::cout << "Creating display...\n";
  auto display_result = create_display<EPD27>(device, DisplayMode::BlackWhite, Orientation::Landscape90);

  if (!display_result) {
    std::cerr << "Display creation failed: " << display_result.error().what() << "\n";
    return 1;
  }

  auto display = std::move(display_result.value());

  // Test 1: Simple text layout
  std::cout << "\nTest 1: Simple text layout\n";
  display.clear(Color::White);
  display.draw_rectangle(0, 0, display.effective_width() - 1, display.effective_height() - 1, Color::Black,
                         DotPixel::Pixel1x1, DrawFill::Empty);
  display.draw_string(10, 10, "BMP Export Test", Font::font24(), Color::Black, Color::White);
  display.draw_string(10, 40, "This is saved to BMP", Font::font16(), Color::Black, Color::White);
  display.draw_string(10, 60, "without display refresh!", Font::font12(), Color::Black, Color::White);

  if (auto result = display.save_framebuffer_to_bmp("test1_text.bmp"); !result) {
    std::cerr << "  Failed to save BMP: " << result.error().what() << "\n";
  } else {
    std::cout << "  ✓ Saved to test1_text.bmp\n";
  }

  // Test 2: Graphics and shapes
  std::cout << "\nTest 2: Graphics and shapes\n";
  display.clear(Color::White);
  display.draw_string(10, 5, "Shapes Demo", Font::font16(), Color::Black, Color::White);

  // Draw various shapes
  display.draw_rectangle(20, 30, 80, 90, Color::Black, DotPixel::Pixel1x1, DrawFill::Empty);
  display.draw_rectangle(100, 30, 160, 90, Color::Black, DotPixel::Pixel1x1, DrawFill::Full);
  display.draw_circle(220, 60, 30, Color::Black, DotPixel::Pixel1x1, DrawFill::Empty);
  display.draw_line(20, 110, 250, 110, Color::Black, DotPixel::Pixel1x1);
  display.draw_line(20, 120, 250, 150, Color::Black, DotPixel::Pixel2x2);

  if (auto result = display.save_framebuffer_to_bmp("test2_shapes.bmp"); !result) {
    std::cerr << "  Failed to save BMP: " << result.error().what() << "\n";
  } else {
    std::cout << "  ✓ Saved to test2_shapes.bmp\n";
  }

  // Test 3: Font samples
  std::cout << "\nTest 3: Font samples\n";
  display.clear(Color::White);
  display.draw_string(5, 5, "Font Sizes:", Font::font16(), Color::Black, Color::White);
  display.draw_string(5, 30, "Font 8", Font::font8(), Color::Black, Color::White);
  display.draw_string(5, 45, "Font 12", Font::font12(), Color::Black, Color::White);
  display.draw_string(5, 65, "Font 16", Font::font16(), Color::Black, Color::White);
  display.draw_string(5, 90, "Font 20", Font::font20(), Color::Black, Color::White);
  display.draw_string(5, 120, "Font 24", Font::font24(), Color::Black, Color::White);

  if (auto result = display.save_framebuffer_to_bmp("test3_fonts.bmp"); !result) {
    std::cerr << "  Failed to save BMP: " << result.error().what() << "\n";
  } else {
    std::cout << "  ✓ Saved to test3_fonts.bmp\n";
  }

  // Test 4: Numbers and decimals
  std::cout << "\nTest 4: Numbers and decimals\n";
  display.clear(Color::White);
  display.draw_string(5, 5, "Numbers:", Font::font16(), Color::Black, Color::White);
  display.draw_string(5, 30, "Integer:", Font::font12(), Color::Black, Color::White);
  display.draw_number(90, 30, 12345, Font::font12(), Color::Black, Color::White);
  display.draw_string(5, 50, "Decimal:", Font::font12(), Color::Black, Color::White);
  display.draw_decimal(90, 50, 3.14159, 3, Font::font12(), Color::Black, Color::White);
  display.draw_string(5, 70, "Price:", Font::font12(), Color::Black, Color::White);
  display.draw_string(70, 70, "$", Font::font12(), Color::Black, Color::White);
  display.draw_decimal(80, 70, 42599.99, 2, Font::font12(), Color::Black, Color::White);

  if (auto result = display.save_framebuffer_to_bmp("test4_numbers.bmp"); !result) {
    std::cerr << "  Failed to save BMP: " << result.error().what() << "\n";
  } else {
    std::cout << "  ✓ Saved to test4_numbers.bmp\n";
  }

  std::cout << "\n✅ All BMP files created successfully!\n";
  std::cout << "\nYou can view these files on any device to verify\n";
  std::cout << "the layout before waiting for slow display refresh.\n";
  std::cout << "\nUsage in your code:\n";
  std::cout << "  display->clear(Color::White);\n";
  std::cout << "  // ... draw your content ...\n";
  std::cout << "  display->save_framebuffer_to_bmp(\"debug.bmp\");  // Debug!\n";
  std::cout << "  display->refresh();  // Now update display\n";

  return 0;
}
