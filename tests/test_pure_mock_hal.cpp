#include "epaper/core/device.hpp"
#include "epaper/drivers/mock_driver.hpp"
#include <cassert>
#include <iostream>

using namespace epaper;

auto main() -> int {
  std::cout << "=======================================" << std::endl;
  std::cout << "  Mock Driver Test" << std::endl;
  std::cout << "=======================================" << std::endl;
  std::cout << "Testing MockDriver with Device (simplified architecture)..." << std::endl;

  // Create Device instance
  Device device;
  auto init_result = device.init();
  assert(init_result.has_value());

  // Create MockDriver using Device
  MockDriver driver{device, 176, 264, false};

  std::cout << "Driver created successfully!" << std::endl;

  // Test initialization
  auto driver_init_result = driver.init(DisplayMode::BlackWhite);
  assert(driver_init_result.has_value());
  assert(driver.is_initialized());
  std::cout << "✓ init() succeeded" << std::endl;

  // Verify dimensions
  assert(driver.width() == 176);
  assert(driver.height() == 264);
  std::cout << "✓ Dimensions correct: " << driver.width() << "x" << driver.height() << std::endl;

  // Create test buffer
  const auto buffer_size = driver.buffer_size();
  std::vector<std::byte> buffer(buffer_size, std::byte{0xFF});
  std::cout << "✓ Created " << buffer_size << " byte buffer" << std::endl;

  // Test display
  auto display_result = driver.display(buffer);
  assert(display_result.has_value());
  assert(driver.display_called());
  std::cout << "✓ display() succeeded" << std::endl;

  // Verify buffer was stored
  assert(driver.last_buffer().size() == buffer_size);
  std::cout << "✓ Buffer stored correctly" << std::endl;

  // Test sleep
  auto sleep_result = driver.sleep();
  assert(sleep_result.has_value());
  assert(driver.is_asleep());
  std::cout << "✓ sleep() succeeded" << std::endl;

  // Test wake
  auto wake_result = driver.wake();
  assert(wake_result.has_value());
  assert(!driver.is_asleep());
  std::cout << "✓ wake() succeeded" << std::endl;

  // Verify call tracking
  assert(driver.init_count() == 1);
  assert(driver.display_count() == 1);
  assert(driver.sleep_count() == 1);
  assert(driver.wake_count() == 1);
  std::cout << "✓ Call tracking works: init=" << driver.init_count() << " display=" << driver.display_count()
            << " sleep=" << driver.sleep_count() << " wake=" << driver.wake_count() << std::endl;

  std::cout << "\n=======================================" << std::endl;
  std::cout << "  Mock Driver Test: PASSED" << std::endl;
  std::cout << "=======================================" << std::endl;

  return 0;
}
