#include "crypto_api.hpp"
#include "dashboard_renderer.hpp"
#include "http_client.hpp"
#include "types.hpp"

#include <epaper/core/device.hpp>
#include <epaper/core/display.hpp>
#include <epaper/drivers/epd27.hpp> // NOTE: Dashboard layout optimized for 2.7" display (264x176 landscape)

#include <CLI/CLI.hpp>

#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <span>
#include <thread>

using namespace epaper;
using namespace crypto_dashboard;

namespace {
// Global state for signal handling (anonymous namespace limits visibility to this TU)
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
std::atomic<bool> g_running{true};
} // namespace

void signal_handler(int signal) {
  std::cout << "\nReceived signal " << signal << ", shutting down gracefully...\n";
  std::cout << "Note: May take a few seconds to exit if display is refreshing...\n";
  g_running = false;
  // Note: If stuck in display refresh or HTTP request, will exit after operation completes
}

auto parse_arguments(std::span<char *> args) -> AppConfig {
  CLI::App app{"Crypto Dashboard - E-paper cryptocurrency display"};

  AppConfig config;

  app.add_option("--screen-flip-interval", config.screen_flip_interval_seconds,
                 "Interval between screen rotations (seconds)")
      ->check(CLI::PositiveNumber)
      ->default_val(60);

  app.add_option("--data-fetch-interval", config.data_fetch_interval_seconds, "Interval between data fetches (seconds)")
      ->check(CLI::PositiveNumber)
      ->default_val(900);

  try {
    app.parse(static_cast<int>(args.size()), args.data());
  } catch (const CLI::ParseError &e) {
    std::exit(app.exit(e));
  }

  return config;
}

auto main(int argc, char *argv[]) -> int {
  try {
    std::cout << "Crypto Dashboard Demo\n";
    std::cout << "====================\n";
    std::cout << "Press Ctrl+C to stop gracefully\n\n";

    // Parse command line arguments and load configuration
    auto config = parse_arguments(std::span(argv, static_cast<std::size_t>(argc)));

    // Display configuration
    std::cout << "Screen flip interval: " << config.screen_flip_interval_seconds << " seconds\n";
    std::cout << "Data fetch interval: " << config.data_fetch_interval_seconds << " seconds ("
              << (config.data_fetch_interval_seconds / 60) << " minutes)\n\n";

    // Register signal handlers
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    // Initialize device
    std::cout << "Initializing device...\n";
    Device device;
    if (auto result = device.init(); !result) {
      std::cerr << "Failed to initialize device: " << result.error().what() << "\n";
      return EXIT_FAILURE;
    }

    // Create display using factory function in landscape mode
    // NOTE: Dashboard layout is currently optimized for 2.7" display (264x176 pixels in Landscape270)
    // Auto-sleep enabled - transparent wake management handles multiple refreshes automatically
    auto display =
        create_display<EPD27, MonoFramebuffer>(device, DisplayMode::BlackWhite, Orientation::Landscape270, true);
    if (!display) {
      std::cerr << "Failed to initialize display: " << display.error().what() << "\n";
      return EXIT_FAILURE;
    }

    std::cout << "Display size (landscape): " << display->effective_width() << "x" << display->effective_height()
              << " pixels\n\n";

    // Create API clients using composition
    HTTPClient http_client;
    CryptoDataFetcher data_fetcher(http_client);

    // Create renderer using template argument deduction
    DashboardRenderer renderer(*display);

    // Cached data (updated periodically, used for all screen rotations)
    CryptoPrice btc_price;
    CryptoPrice eth_price;
    PriceHistory btc_30d;
    PriceHistory eth_30d;
    PriceHistory btc_6mo;
    PriceHistory eth_6mo;
    bool data_valid = false;

    // Screen rotation state
    ScreenType current_screen = ScreenType::Combined;
    auto last_data_fetch = std::chrono::steady_clock::now();
    auto last_screen_flip = std::chrono::steady_clock::now();

    // Fetch initial data BEFORE rendering first screen (prevents "Error" message)
    std::cout << "Fetching initial data...\n";
    constexpr auto api_delay = std::chrono::milliseconds(5000);

    auto prices_result = data_fetcher.fetch_crypto_prices();
    if (prices_result) {
      auto [btc, eth] = *prices_result;
      btc_price = btc;
      eth_price = eth;

      // Fetch histories with delays to respect API rate limits
      if (g_running) {
        std::this_thread::sleep_for(api_delay);
        auto btc_30d_result = data_fetcher.fetch_price_history("BTC", 30);
        if (btc_30d_result) {
          btc_30d = *btc_30d_result;
        }
      }

      if (g_running) {
        std::this_thread::sleep_for(api_delay);
        auto eth_30d_result = data_fetcher.fetch_price_history("ETH", 30);
        if (eth_30d_result) {
          eth_30d = *eth_30d_result;
        }
      }

      if (g_running) {
        std::this_thread::sleep_for(api_delay);
        auto btc_6mo_result = data_fetcher.fetch_price_history("BTC", 180);
        if (btc_6mo_result) {
          btc_6mo = *btc_6mo_result;
        }
      }

      if (g_running) {
        std::this_thread::sleep_for(api_delay);
        auto eth_6mo_result = data_fetcher.fetch_price_history("ETH", 180);
        if (eth_6mo_result) {
          eth_6mo = *eth_6mo_result;
        }
      }

      data_valid = true;
      last_data_fetch = std::chrono::steady_clock::now(); // Update fetch time
      std::cout << "Initial data fetch complete.\n\n";
    } else {
      std::cerr << "Failed to fetch initial prices: " << prices_result.error() << "\n";
    }

    // Render initial screen with data (or error if fetch failed)
    std::cout << "Rendering initial screen...\n";
    if (data_valid && g_running) {
      renderer.render(current_screen, btc_price, eth_price, btc_30d, eth_30d, btc_6mo, eth_6mo);
      std::cout << "Initial screen rendered successfully.\n";
    } else if (g_running) {
      renderer.render_error("Failed to fetch initial data");
    }

    // Main loop
    int fetch_count = 0;
    int screen_flip_count = 0;

    std::cout << "\nStarting main loop...\n";
    std::cout << "Screen rotation: Combined -> BTC -> ETH -> Combined...\n\n";

    while (g_running) {
      const auto now = std::chrono::steady_clock::now();

      // Check if we need to fetch new data
      const auto time_since_fetch = std::chrono::duration_cast<std::chrono::seconds>(now - last_data_fetch).count();
      if (g_running && (!data_valid || time_since_fetch >= config.data_fetch_interval_seconds)) {
        ++fetch_count;
        std::cout << "Fetching data (fetch " << fetch_count << ")...\n";

        // Fetch crypto prices (check g_running first)
        if (!g_running) {
          break;
        }
        auto prices_result = data_fetcher.fetch_crypto_prices();
        if (!prices_result) {
          std::cerr << "Failed to fetch prices: " << prices_result.error() << "\n";
          if (g_running) {
            renderer.render_error(prices_result.error());
          }
          last_data_fetch = now;
          // Continue to screen rotation even if fetch failed
        } else {
          auto [btc, eth] = *prices_result;
          btc_price = btc;
          eth_price = eth;

          // Add delays between API calls to respect CoinGecko rate limits (free tier)
          // CoinGecko free API: ~10-50 requests/minute, so 5-second delay is safer
          constexpr auto api_delay = std::chrono::milliseconds(5000);

          // Fetch 30-day histories (check g_running before each)
          if (g_running) {
            std::this_thread::sleep_for(api_delay);
          }
          if (g_running) {
            auto btc_30d_result = data_fetcher.fetch_price_history("BTC", 30);
            if (btc_30d_result) {
              btc_30d = *btc_30d_result;
            } else {
              std::cerr << "    Warning: BTC 30d fetch failed: " << btc_30d_result.error() << "\n";
            }
          }

          if (g_running) {
            std::this_thread::sleep_for(api_delay);
          }
          if (g_running) {
            auto eth_30d_result = data_fetcher.fetch_price_history("ETH", 30);
            if (eth_30d_result) {
              eth_30d = *eth_30d_result;
            } else {
              std::cerr << "    Warning: ETH 30d fetch failed: " << eth_30d_result.error() << "\n";
            }
          }

          // Fetch 6-month histories (check g_running before each)
          if (g_running) {
            std::this_thread::sleep_for(api_delay);
          }
          if (g_running) {
            auto btc_6mo_result = data_fetcher.fetch_price_history("BTC", 180);
            if (btc_6mo_result) {
              btc_6mo = *btc_6mo_result;
            } else {
              std::cerr << "    Warning: BTC 6mo fetch failed: " << btc_6mo_result.error() << "\n";
            }
          }

          if (g_running) {
            std::this_thread::sleep_for(api_delay);
          }
          if (g_running) {
            auto eth_6mo_result = data_fetcher.fetch_price_history("ETH", 180);
            if (eth_6mo_result) {
              eth_6mo = *eth_6mo_result;
            } else {
              std::cerr << "    Warning: ETH 6mo fetch failed: " << eth_6mo_result.error() << "\n";
            }
          }

          // Only update and render if still running
          if (g_running) {
            data_valid = true;
            last_data_fetch = now;
            std::cout << "Data fetch complete.\n";
            std::cout << "  BTC valid: " << (btc_price.is_valid() ? "yes" : "no")
                      << ", ETH valid: " << (eth_price.is_valid() ? "yes" : "no") << "\n";

            // Re-render current screen immediately with new data
            std::cout << "Rendering screen with new data...\n";
            std::cout.flush(); // Ensure output is visible

            try {
              renderer.render(current_screen, btc_price, eth_price, btc_30d, eth_30d, btc_6mo, eth_6mo);
              std::cout << "Screen updated with new data.\n";
              std::cout.flush();
            } catch (const std::exception &e) {
              std::cerr << "Error during render: " << e.what() << "\n";
              if (g_running) {
                renderer.render_error("Render error: " + std::string(e.what()));
              }
            }

            // Exit immediately if shutdown was requested during render
            if (!g_running) {
              break;
            }
          }
        }
      }

      // Check if we need to rotate screen
      if (g_running) {
        const auto time_since_flip = std::chrono::duration_cast<std::chrono::seconds>(now - last_screen_flip).count();

        if (time_since_flip >= config.screen_flip_interval_seconds) {
          ++screen_flip_count;

          // Rotate to next screen
          switch (current_screen) {
          case ScreenType::Combined:
            current_screen = ScreenType::BTCDedicated;
            std::cout << "Rotating to BTC dedicated screen (flip " << screen_flip_count << ")...\n";
            break;
          case ScreenType::BTCDedicated:
            current_screen = ScreenType::ETHDedicated;
            std::cout << "Rotating to ETH dedicated screen (flip " << screen_flip_count << ")...\n";
            break;
          case ScreenType::ETHDedicated:
            current_screen = ScreenType::Combined;
            std::cout << "Rotating to combined screen (flip " << screen_flip_count << ")...\n";
            break;
          }

          // Render current screen with cached data
          if (g_running && data_valid) {
            renderer.render(current_screen, btc_price, eth_price, btc_30d, eth_30d, btc_6mo, eth_6mo);
            std::cout << "Screen rendered successfully.\n";
          } else if (g_running) {
            renderer.render_error("Waiting for data...");
          }

          // Exit immediately if shutdown was requested during render
          if (!g_running) {
            break;
          }

          last_screen_flip = now;
        }
      }

      // Sleep for 1 second before checking again (break early if shutdown requested)
      // Check g_running multiple times during sleep for faster exit
      for (int i = 0; i < 10 && g_running; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
    }

    // Clean shutdown
    std::cout << "Performing clean shutdown...\n";
    std::cout << "Clearing display...\n";

    display->clear();

    if (auto result = display->refresh(); !result) {
      std::cerr << "Failed to refresh display during shutdown: " << result.error().what() << "\n";
    }
    // Auto-sleep handles putting display to sleep after refresh

    std::cout << "Shutdown complete. Goodbye!\n";
    return EXIT_SUCCESS;

  } catch (const std::exception &e) {
    std::cerr << "Fatal error: " << e.what() << "\n";
    return EXIT_FAILURE;
  }
}
