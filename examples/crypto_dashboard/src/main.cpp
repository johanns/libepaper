#include "crypto_api.hpp"
#include "dashboard_renderer.hpp"
#include "http_client.hpp"
#include "types.hpp"
#include "wallet_config.hpp"

#include <epaper/device.hpp>
#include <epaper/draw.hpp>
#include <epaper/drivers/epd27.hpp>
#include <epaper/screen.hpp>

#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>

using namespace epaper;
using namespace crypto_dashboard;

// Global state for signal handling
std::atomic<bool> g_running{true};
EPD27 *g_epd27 = nullptr;

void signal_handler(int signal) {
  std::cout << "\nReceived signal " << signal << ", shutting down gracefully...\n";
  g_running = false;
  // Note: Display cleanup happens in main after loop exits
}

void print_usage(const char *program_name) {
  std::cout << "Usage: " << program_name << " [options]\n\n";
  std::cout << "Options:\n";
  std::cout << "  --etherscan-api-key=KEY    Etherscan API key for ETH balance fetching\n";
  std::cout << "  --config=PATH              Path to wallets.json config file (default: ./wallets.json)\n";
  std::cout << "  --interval=SECONDS         Update interval in seconds (default: 30)\n";
  std::cout << "  --help, -h                 Show this help message\n\n";
  std::cout << "Get a free Etherscan API key at: https://etherscan.io/apis\n";
}

auto parse_arguments(int argc, char *argv[]) -> AppConfig {
  AppConfig config;
  std::string config_path = "wallets.json";

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    if (arg == "--help" || arg == "-h") {
      print_usage(argv[0]);
      std::exit(EXIT_SUCCESS);
    } else if (arg.rfind("--etherscan-api-key=", 0) == 0) {
      config.etherscan_api_key = arg.substr(20);
    } else if (arg.rfind("--config=", 0) == 0) {
      config_path = arg.substr(9);
    } else if (arg.rfind("--interval=", 0) == 0) {
      try {
        config.update_interval_seconds = std::stoi(arg.substr(11));
      } catch (...) {
        std::cerr << "Invalid interval value\n";
      }
    } else {
      std::cerr << "Unknown option: " << arg << "\n";
      print_usage(argv[0]);
      std::exit(EXIT_FAILURE);
    }
  }

  // Load wallet configuration
  auto wallet_config = WalletConfigLoader::load(config_path);
  if (!wallet_config) {
    std::cerr << "Error: " << wallet_config.error() << "\n";
    std::cerr << "Creating example configuration file...\n";
    if (WalletConfigLoader::create_example(config_path + ".example")) {
      std::cerr << "Example created at: " << config_path << ".example\n";
      std::cerr << "Please edit and rename to: " << config_path << "\n";
    }
    std::exit(EXIT_FAILURE);
  }

  config.wallets = *wallet_config;

  return config;
}

auto main(int argc, char *argv[]) -> int {
  try {
    std::cout << "Crypto Dashboard Demo\n";
    std::cout << "====================\n";
    std::cout << "Press Ctrl+C to stop gracefully\n\n";

    // Parse command line arguments and load configuration
    auto config = parse_arguments(argc, argv);

    // Display configuration
    if (config.has_etherscan_key()) {
      std::cout << "Etherscan API key: " << config.etherscan_api_key.substr(0, 8) << "...\n";
    } else {
      std::cout << "No Etherscan API key provided. ETH balances will not be fetched.\n";
      std::cout << "Use --etherscan-api-key=YOUR_KEY to enable ETH balance tracking.\n";
    }

    std::cout << "BTC addresses: " << config.wallets.btc_addresses.size() << "\n";
    std::cout << "ETH addresses: " << config.wallets.eth_addresses.size() << "\n";
    std::cout << "Update interval: " << config.update_interval_seconds << " seconds\n\n";

    // Register signal handlers
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    // Initialize device
    std::cout << "Initializing device...\n";
    Device device;
    if (auto result = device.init(); !result) {
      std::cerr << "Failed to initialize device\n";
      return EXIT_FAILURE;
    }

    // Create EPD27 driver
    EPD27 epd27(device);
    g_epd27 = &epd27;

    if (auto result = epd27.init(DisplayMode::BlackWhite); !result) {
      std::cerr << "Failed to initialize display\n";
      return EXIT_FAILURE;
    }

    epd27.clear();

    // Create screen in landscape mode
    Screen screen(epd27, Orientation::Landscape90);
    Draw draw(screen);

    std::cout << "Display size (landscape): " << screen.effective_width() << "x" << screen.effective_height()
              << " pixels\n\n";

    // Create API clients using composition
    HTTPClient http_client;
    CryptoDataFetcher data_fetcher(http_client, config.etherscan_api_key);

    // Create renderer
    DashboardRenderer renderer(screen, draw);

    // Main update loop
    int update_count = 0;

    while (g_running) {
      ++update_count;
      std::cout << "Fetching data (update " << update_count << ")...\n";

      // Fetch crypto prices
      auto prices_result = data_fetcher.fetch_crypto_prices();
      if (!prices_result) {
        std::cerr << "Failed to fetch prices: " << prices_result.error() << "\n";
        renderer.render_error(prices_result.error());

        std::cout << "Waiting " << config.update_interval_seconds << " seconds before retry...\n\n";
        std::this_thread::sleep_for(std::chrono::seconds(config.update_interval_seconds));
        continue;
      }

      auto [btc, eth] = *prices_result;

      // Fetch price histories
      PriceHistory btc_history, eth_history;

      if (g_running) {
        auto btc_hist_result = data_fetcher.fetch_price_history("BTC", 30);
        if (btc_hist_result) {
          btc_history = *btc_hist_result;
        }
      }

      if (g_running) {
        auto eth_hist_result = data_fetcher.fetch_price_history("ETH", 30);
        if (eth_hist_result) {
          eth_history = *eth_hist_result;
        }
      }

      // Fetch wallet balances
      WalletBalance wallet;
      if (g_running) {
        wallet = data_fetcher.fetch_wallet_balances(config.wallets.btc_addresses, config.wallets.eth_addresses);
        std::cout << "  BTC balance: " << wallet.btc_balance << " BTC\n";
        if (wallet.has_eth_balance()) {
          std::cout << "  ETH balance: " << wallet.eth_balance << " ETH\n";
        } else {
          std::cout << "  ETH balance: API key required\n";
        }
      }

      // Render dashboard
      if (g_running) {
        std::cout << "  Rendering dashboard...\n";
        renderer.render(btc, eth, btc_history, eth_history, wallet);
        std::cout << "  Dashboard updated successfully!\n\n";
      }

      // Wait for next update
      std::cout << "Waiting " << config.update_interval_seconds << " seconds before next update...\n\n";
      for (int i = 0; i < config.update_interval_seconds && g_running; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
      }
    }

    // Clean shutdown (same as legacy version)
    std::cout << "Performing clean shutdown...\n";
    std::cout << "Clearing display...\n";
    screen.clear();
    screen.refresh(); // This takes 30-60 seconds but ensures clean display state

    std::cout << "Putting display to sleep...\n";
    epd27.sleep();

    std::cout << "Shutdown complete. Goodbye!\n";
    return EXIT_SUCCESS;

  } catch (const std::exception &e) {
    std::cerr << "Fatal error: " << e.what() << "\n";
    return EXIT_FAILURE;
  }
}
