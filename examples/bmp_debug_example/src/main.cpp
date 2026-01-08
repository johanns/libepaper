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
  display.draw(display.rectangle()
                   .top_left(0, 0)
                   .bottom_right(display.effective_width() - 1, display.effective_height() - 1)
                   .color(Color::Black)
                   .border_width(DotPixel::Pixel1x1)
                   .fill(DrawFill::Empty)
                   .build());
  display.draw(display.text("BMP Export Test")
                   .at(10, 10)
                   .font(&Font::font24())
                   .foreground(Color::Black)
                   .background(Color::White)
                   .build());
  display.draw(display.text("This is saved to BMP")
                   .at(10, 40)
                   .font(&Font::font16())
                   .foreground(Color::Black)
                   .background(Color::White)
                   .build());
  display.draw(display.text("without display refresh!")
                   .at(10, 60)
                   .font(&Font::font12())
                   .foreground(Color::Black)
                   .background(Color::White)
                   .build());

  if (auto result = display.save_framebuffer_to_bmp("test1_text.bmp"); !result) {
    std::cerr << "  Failed to save BMP: " << result.error().what() << "\n";
  } else {
    std::cout << "  ✓ Saved to test1_text.bmp\n";
  }

  // Test 2: Graphics and shapes
  std::cout << "\nTest 2: Graphics and shapes\n";
  display.clear(Color::White);
  display.draw(display.text("Shapes Demo")
                   .at(10, 5)
                   .font(&Font::font16())
                   .foreground(Color::Black)
                   .background(Color::White)
                   .build());

  // Draw various shapes
  display.draw(display.rectangle()
                   .top_left(20, 30)
                   .bottom_right(80, 90)
                   .color(Color::Black)
                   .border_width(DotPixel::Pixel1x1)
                   .fill(DrawFill::Empty)
                   .build());
  display.draw(display.rectangle()
                   .top_left(100, 30)
                   .bottom_right(160, 90)
                   .color(Color::Black)
                   .border_width(DotPixel::Pixel1x1)
                   .fill(DrawFill::Full)
                   .build());
  display.draw(display.circle()
                   .center(220, 60)
                   .radius(30)
                   .color(Color::Black)
                   .border_width(DotPixel::Pixel1x1)
                   .fill(DrawFill::Empty)
                   .build());
  display.draw(display.line().from(20, 110).to(250, 110).color(Color::Black).width(DotPixel::Pixel1x1).build());
  display.draw(display.line().from(20, 120).to(250, 150).color(Color::Black).width(DotPixel::Pixel2x2).build());

  if (auto result = display.save_framebuffer_to_bmp("test2_shapes.bmp"); !result) {
    std::cerr << "  Failed to save BMP: " << result.error().what() << "\n";
  } else {
    std::cout << "  ✓ Saved to test2_shapes.bmp\n";
  }

  // Test 3: Font samples
  std::cout << "\nTest 3: Font samples\n";
  display.clear(Color::White);
  display.draw(display.text("Font Sizes:")
                   .at(5, 5)
                   .font(&Font::font16())
                   .foreground(Color::Black)
                   .background(Color::White)
                   .build());
  display.draw(
      display.text("Font 8").at(5, 30).font(&Font::font8()).foreground(Color::Black).background(Color::White).build());
  display.draw(display.text("Font 12")
                   .at(5, 45)
                   .font(&Font::font12())
                   .foreground(Color::Black)
                   .background(Color::White)
                   .build());
  display.draw(display.text("Font 16")
                   .at(5, 65)
                   .font(&Font::font16())
                   .foreground(Color::Black)
                   .background(Color::White)
                   .build());
  display.draw(display.text("Font 20")
                   .at(5, 90)
                   .font(&Font::font20())
                   .foreground(Color::Black)
                   .background(Color::White)
                   .build());
  display.draw(display.text("Font 24")
                   .at(5, 120)
                   .font(&Font::font24())
                   .foreground(Color::Black)
                   .background(Color::White)
                   .build());

  if (auto result = display.save_framebuffer_to_bmp("test3_fonts.bmp"); !result) {
    std::cerr << "  Failed to save BMP: " << result.error().what() << "\n";
  } else {
    std::cout << "  ✓ Saved to test3_fonts.bmp\n";
  }

  // Test 4: Numbers and decimals
  std::cout << "\nTest 4: Numbers and decimals\n";
  display.clear(Color::White);
  display.draw(display.text("Numbers:")
                   .at(5, 5)
                   .font(&Font::font16())
                   .foreground(Color::Black)
                   .background(Color::White)
                   .build());
  display.draw(display.text("Integer:")
                   .at(5, 30)
                   .font(&Font::font12())
                   .foreground(Color::Black)
                   .background(Color::White)
                   .build());
  display.draw(display.text()
                   .number(12345)
                   .at(90, 30)
                   .font(&Font::font12())
                   .foreground(Color::Black)
                   .background(Color::White)
                   .build());
  display.draw(display.text("Decimal:")
                   .at(5, 50)
                   .font(&Font::font12())
                   .foreground(Color::Black)
                   .background(Color::White)
                   .build());
  display.draw(display.text()
                   .decimal(3.14159, 3)
                   .at(90, 50)
                   .font(&Font::font12())
                   .foreground(Color::Black)
                   .background(Color::White)
                   .build());
  display.draw(
      display.text("Price:").at(5, 70).font(&Font::font12()).foreground(Color::Black).background(Color::White).build());
  display.draw(
      display.text("$").at(70, 70).font(&Font::font12()).foreground(Color::Black).background(Color::White).build());
  display.draw(display.text()
                   .decimal(42599.99, 2)
                   .at(80, 70)
                   .font(&Font::font12())
                   .foreground(Color::Black)
                   .background(Color::White)
                   .build());

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
