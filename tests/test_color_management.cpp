#include "epaper/color/color_manager.hpp"
#include "epaper/core/types.hpp"
#include <cassert>
#include <iostream>

using namespace epaper;

void verify(bool condition, const char *message) {
  if (!condition) {
    std::cerr << "FAILED: " << message << std::endl;
    std::exit(1);
  }
}

void test_to_rgb() {
  std::cout << "Testing to_rgb..." << std::endl;

  verify(ColorManager::to_rgb(Color::Black) == colors::Black, "Black -> RGB Black");
  verify(ColorManager::to_rgb(Color::White) == colors::White, "White -> RGB White");
  verify(ColorManager::to_rgb(Color::Red) == colors::Red, "Red -> RGB Red");
}

void test_convert_bwr() {
  std::cout << "Testing convert<BWR>..." << std::endl;

  // Exact matches
  auto b = ColorManager::convert<DisplayMode::BWR>(colors::Black);
  verify(!b.get_bw_bit(), "Black should have BW bit 0 (black ink)");

  auto w = ColorManager::convert<DisplayMode::BWR>(colors::White);
  verify(w.get_bw_bit(), "White should have BW bit 1 (no ink)");
  verify(w.get_color_bit(), "White should have Color bit 1 (no ink)");

  auto r = ColorManager::convert<DisplayMode::BWR>(colors::Red);
  verify(!r.get_color_bit(), "Red should have Color bit 0 (red ink)");

  // Nearest Neighbor
  // Light Red -> Red
  RGB light_red{200, 50, 50};
  auto lr = ColorManager::convert<DisplayMode::BWR>(light_red);
  verify(!lr.get_color_bit(), "Light Red should map to Red");

  // Dark Blue -> Black
  RGB dark_blue{0, 0, 50};
  auto db = ColorManager::convert<DisplayMode::BWR>(dark_blue);
  verify(!db.get_bw_bit(), "Dark Blue should map to Black");
}

void test_convert_bwy() {
  std::cout << "Testing convert<BWY>..." << std::endl;

  auto y = ColorManager::convert<DisplayMode::BWY>(colors::Yellow);
  verify(!y.get_color_bit(), "Yellow should have Color bit 0 (yellow ink)");

  // Red -> Black (equidistant 255 to Black (0) and Yellow (255,255,0)? Wait.
  // Red(255,0,0). Black(0,0,0). DistSq = 255^2 = 65025.
  // Yellow(255,255,0). DistSq = (255-255)^2 + (0-255)^2 + 0 = 0 + 65025 = 65025.
  // Equidistant. Logic picks Black if not (dist < black && dist < white).
  // Implementation: if (d_yellow < d_black && d_yellow < d_white) -> Yellow.
  // 65025 < 65025 is False. So it picks Black (or White check).
  auto r = ColorManager::convert<DisplayMode::BWY>(colors::Red);
  verify(!r.get_bw_bit(), "Red should map to Black on BWY (tie-break)");
}

void test_dither() {
  std::cout << "Testing dither_image..." << std::endl;
  // Create a 2x2 gradient
  std::vector<uint8_t> rgb = {0, 0, 0, 255, 255, 255, 128, 128, 128, 0, 0, 0};

  int count = 0;
  ColorManager::dither_image<DisplayMode::BlackWhite>(rgb, 2, 2,
                                                      [&](size_t x, size_t y, DeviceColor<DisplayMode::BlackWhite> c) {
                                                        count++;
                                                        // Just verifying it compiles and runs callback
                                                        if (x == 0 && y == 0) {
                                                          verify(!c.is_white, "0,0 Black");
                                                        }
                                                        if (x == 1 && y == 0) {
                                                          verify(c.is_white, "1,0 White");
                                                        }
                                                        // 128 gray might dither to black or white depending on error
                                                        // from previous? (0,0) Black -> Error 0. (1,0) White -> Error
                                                        // 0. (0,1) Gray 128. From top neighbors? (0,0)->(1,0) 7/16
                                                        // error. (0,0)->(0,1) ???
                                                      });
  verify(count == 4, "Callback called 4 times");
}

auto main() -> int {
  test_to_rgb();
  test_convert_bwr();
  test_convert_bwy();
  test_dither();
  std::cout << "All ColorManager tests PASSED." << std::endl;
  return 0;
}
