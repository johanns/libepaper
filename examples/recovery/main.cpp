#include <chrono>
#include <epaper/core/device.hpp>
#include <epaper/core/display.hpp>
#include <epaper/drivers/epd27.hpp> // Using safe monochrome driver
#include <iostream>
#include <thread>

using namespace epaper;

// Safe cleaning cycle for E-Paper
auto perform_cleaning_cycle(auto &display) -> void {
  std::cout << "Cleaning cycle: WHITE -> BLACK -> WHITE\n";

  // 1. Clear to White
  display.clear(Color::White);
  std::cout << "  Refleshing White...\n";
  if (auto res = display.refresh(); !res) {
    std::cerr << "  Failed to refresh white: " << res.error().what() << "\n";
    return;
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  // 2. Fill Black
  display.clear(Color::Black);
  std::cout << "  Refreshing Black...\n";
  if (auto res = display.refresh(); !res) {
    std::cerr << "  Failed to refresh black: " << res.error().what() << "\n";
    return;
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  // 3. Clear to White again
  display.clear(Color::White);
  std::cout << "  Refreshing White...\n";
  if (auto res = display.refresh(); !res) {
    std::cerr << "  Failed to refresh white: " << res.error().what() << "\n";
    return;
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(2000));
}

auto main() -> int {
  std::cout << "==========================================\n";
  std::cout << "  E-Paper Hardware Recovery Tool\n";
  std::cout << "  (Ghosting Removal Utility)\n";
  std::cout << "==========================================\n";

  Device device;
  if (auto res = device.init(); !res) {
    std::cerr << "Failed to init device: " << res.error().what() << "\n";
    return 1;
  }

  // Force strictly BlackWhite mode for safety
  std::cout << "Initializing display in SAFE MODE (BlackWhite)...\n";
  auto display_res = create_display<EPD27, MonoFramebuffer>(device, DisplayMode::BlackWhite);
  if (!display_res) {
    std::cerr << "Failed to create display: " << display_res.error().what() << "\n";
    return 1;
  }

  auto display = std::move(display_res.value());

  std::cout << "Starting 5 aggressive cleaning cycles...\n";
  std::cout << "DO NOT POWER OFF THE DEVICE.\n\n";

  for (int i = 1; i <= 5; ++i) {
    std::cout << "Cycle " << i << "/5\n";
    perform_cleaning_cycle(display);
  }

  std::cout << "\nRecovery complete. Please inspect the screen.\n";
  std::cout << "If ghosting persists, power off for 1 hour and try again.\n";

  return 0;
}
