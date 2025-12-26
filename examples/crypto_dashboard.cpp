#include <atomic>
#include <cmath>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <curl/curl.h>
#include <epaper/device.hpp>
#include <epaper/draw.hpp>
#include <epaper/drivers/epd27.hpp>
#include <epaper/font.hpp>
#include <epaper/screen.hpp>
#include <expected>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using json = nlohmann::json;

using namespace epaper;

// Global flag for clean shutdown
std::atomic<bool> g_running{true};
EPD27 *g_epd27 = nullptr;

// Signal handler for clean shutdown
void signal_handler(int signum) {
  std::cout << "\n\nReceived signal " << signum << " - Shutting down gracefully...\n";
  g_running = false;
}

// Simple JSON parser for our specific needs
struct CryptoPrice {
  double price = 0.0;
  double change_24h = 0.0;
  std::vector<double> history;
  bool valid = false;
};

struct WalletBalance {
  double btc_balance = 0.0;
  double eth_balance = 0.0;
};

// HTTP client using libcurl
class HTTPClient {
public:
  HTTPClient() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl_ = curl_easy_init();
  }

  ~HTTPClient() {
    if (curl_) {
      curl_easy_cleanup(curl_);
    }
    curl_global_cleanup();
  }

  auto get(const std::string &url) -> std::expected<std::string, std::string> {
    if (!curl_) {
      return std::expected<std::string, std::string>{std::unexpect, "Failed to initialize curl"};
    }

    response_data_.clear();

    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response_data_);
    curl_easy_setopt(curl_, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode res = curl_easy_perform(curl_);

    if (res != CURLE_OK) {
      return std::expected<std::string, std::string>{std::unexpect, std::string{"curl_easy_perform() failed: "} +
                                                                        curl_easy_strerror(res)};
    }

    return response_data_;
  }

private:
  static auto write_callback(void *contents, size_t size, size_t nmemb, std::string *userp) -> size_t {
    userp->append(static_cast<char *>(contents), size * nmemb);
    return size * nmemb;
  }

  CURL *curl_ = nullptr;
  std::string response_data_;
};

// No longer needed - using nlohmann/json library

// Fetch crypto prices from CoinGecko
auto fetch_crypto_prices(HTTPClient &client) -> std::expected<std::pair<CryptoPrice, CryptoPrice>, std::string> {
  std::string url = "https://api.coingecko.com/api/v3/simple/price?ids=bitcoin,ethereum&vs_"
                    "currencies=usd&include_24hr_change=true";

  auto response = client.get(url);
  if (!response) {
    return std::expected<std::pair<CryptoPrice, CryptoPrice>, std::string>{std::unexpect, response.error()};
  }

  CryptoPrice btc, eth;

  try {
    // Parse JSON response
    auto j = json::parse(*response);

    // Parse Bitcoin
    if (j.contains("bitcoin")) {
      if (j["bitcoin"].contains("usd")) {
        btc.price = j["bitcoin"]["usd"].get<double>();
        btc.valid = true;
        std::cout << "  BTC: $" << btc.price;
      }
      if (j["bitcoin"].contains("usd_24h_change")) {
        btc.change_24h = j["bitcoin"]["usd_24h_change"].get<double>();
        std::cout << " (" << (btc.change_24h >= 0 ? "+" : "") << btc.change_24h << "%)\n";
      } else {
        std::cout << "\n";
      }
    }

    // Parse Ethereum
    if (j.contains("ethereum")) {
      if (j["ethereum"].contains("usd")) {
        eth.price = j["ethereum"]["usd"].get<double>();
        eth.valid = true;
        std::cout << "  ETH: $" << eth.price;
      }
      if (j["ethereum"].contains("usd_24h_change")) {
        eth.change_24h = j["ethereum"]["usd_24h_change"].get<double>();
        std::cout << " (" << (eth.change_24h >= 0 ? "+" : "") << eth.change_24h << "%)\n";
      } else {
        std::cout << "\n";
      }
    }

  } catch (const json::exception &e) {
    return std::expected<std::pair<CryptoPrice, CryptoPrice>, std::string>{
        std::unexpect, std::string("JSON parse error: ") + e.what()};
  }

  if (!btc.valid && !eth.valid) {
    return std::expected<std::pair<CryptoPrice, CryptoPrice>, std::string>{
        std::unexpect, "Failed to parse any prices from API response"};
  }

  return std::make_pair(btc, eth);
}

// Fetch price history for charts
auto fetch_price_history(HTTPClient &client, const std::string &coin_id, int days = 1) -> std::vector<double> {
  std::string url = "https://api.coingecko.com/api/v3/coins/" + coin_id +
                    "/market_chart?vs_currency=usd&days=" + std::to_string(days);

  auto response = client.get(url);
  if (!response) {
    std::cerr << "    Failed to fetch price history for " << coin_id << "\n";
    return {};
  }

  std::vector<double> prices;

  try {
    // Parse JSON response
    auto j = json::parse(*response);

    // Extract prices array: [[timestamp, price], [timestamp, price], ...]
    if (j.contains("prices") && j["prices"].is_array()) {
      for (const auto &price_point : j["prices"]) {
        if (price_point.is_array() && price_point.size() >= 2) {
          // Second element is the price
          prices.push_back(price_point[1].get<double>());
        }
      }
    }

    std::cout << "    Fetched " << prices.size() << " price points for " << coin_id << " (" << days << " days)\n";

  } catch (const json::exception &e) {
    std::cerr << "    JSON parse error for " << coin_id << ": " << e.what() << "\n";
    return {};
  }

  return prices;
}

// Fetch wallet balances
auto fetch_wallet_balances(HTTPClient &client, const std::vector<std::string> &btc_addresses,
                           const std::vector<std::string> &eth_addresses) -> WalletBalance {
  WalletBalance balance;

  // Fetch Bitcoin balances
  for (const auto &address : btc_addresses) {
    std::string url = "https://blockchain.info/q/addressbalance/" + address + "?confirmations=6";
    auto response = client.get(url);
    if (response) {
      try {
        double satoshis = std::stod(*response);
        balance.btc_balance += satoshis / 100000000.0; // Convert to BTC
      } catch (...) {
      }
    }
  }

  // Fetch Ethereum balances (using Etherscan API)
  for (const auto &address : eth_addresses) {
    std::string url = "https://api.etherscan.io/api?module=account&action=balance&address=" + address + "&tag=latest";
    auto response = client.get(url);
    if (response) {
      try {
        auto j = json::parse(*response);
        if (j.contains("result")) {
          std::string result_str = j["result"].get<std::string>();
          double wei = std::stod(result_str);
          balance.eth_balance += wei / 1e18; // Convert to ETH
        }
      } catch (const json::exception &e) {
        std::cerr << "    Failed to parse ETH balance: " << e.what() << "\n";
      } catch (const std::exception &e) {
        std::cerr << "    Failed to convert ETH balance: " << e.what() << "\n";
      }
    }
  }

  return balance;
}

// Graph rendering functions
auto draw_line_chart(Draw &draw, size_t x, size_t y, size_t width, size_t height, const std::vector<double> &data)
    -> void {
  if (data.empty()) {
    return;
  }

  // Find min and max
  double min_val = data[0];
  double max_val = data[0];
  for (const auto &val : data) {
    if (val < min_val)
      min_val = val;
    if (val > max_val)
      max_val = val;
  }

  double range = max_val - min_val;
  if (range < 0.0001) {
    range = 1.0;
  }

  // Draw border
  draw.draw_rectangle(x, y, x + width, y + height, Color::Black, DotPixel::Pixel1x1, DrawFill::Empty);

  // Plot data points
  for (size_t i = 1; i < data.size(); ++i) {
    size_t x1 = x + ((i - 1) * width) / (data.size() - 1);
    size_t y1 = y + height - static_cast<size_t>(((data[i - 1] - min_val) / range) * height);
    size_t x2 = x + (i * width) / (data.size() - 1);
    size_t y2 = y + height - static_cast<size_t>(((data[i] - min_val) / range) * height);

    draw.draw_line(x1, y1, x2, y2, Color::Black, DotPixel::Pixel1x1, LineStyle::Solid);
  }
}

auto draw_histogram(Draw &draw, size_t x, size_t y, size_t width, size_t height, const std::vector<double> &data)
    -> void {
  if (data.empty()) {
    return;
  }

  // Find min and max for price range
  double min_val = data[0];
  double max_val = data[0];
  for (const auto &val : data) {
    if (val < min_val)
      min_val = val;
    if (val > max_val)
      max_val = val;
  }

  double range = max_val - min_val;
  if (range < 0.0001) {
    range = 1.0;
  }

  // Create histogram bins (12 bins for good resolution)
  constexpr size_t num_bins = 12;
  std::vector<size_t> bins(num_bins, 0);

  // Count prices in each bin
  for (const auto &val : data) {
    size_t bin_idx = static_cast<size_t>(((val - min_val) / range) * (num_bins - 1));
    if (bin_idx >= num_bins)
      bin_idx = num_bins - 1;
    bins[bin_idx]++;
  }

  // Find max frequency for scaling
  size_t max_freq = 1;
  for (const auto &count : bins) {
    if (count > max_freq)
      max_freq = count;
  }

  // Draw border
  draw.draw_rectangle(x, y, x + width, y + height, Color::Black, DotPixel::Pixel1x1, DrawFill::Empty);

  // Draw histogram bars
  size_t bar_width = width / num_bins;
  for (size_t i = 0; i < num_bins; ++i) {
    if (bins[i] > 0) {
      size_t bar_height = (bins[i] * (height - 2)) / max_freq;
      size_t bar_x = x + 1 + (i * bar_width);
      size_t bar_y = y + height - 1 - bar_height;

      // Draw filled bar
      draw.draw_rectangle(bar_x, bar_y, bar_x + bar_width - 1, y + height - 1, Color::Black, DotPixel::Pixel1x1,
                          DrawFill::Full);
    }
  }
}

auto draw_price_indicator(Draw &draw, size_t x, size_t y, double change_24h) -> void {
  if (change_24h > 0) {
    // Up arrow
    draw.draw_line(x + 3, y + 5, x + 3, y, Color::Black, DotPixel::Pixel1x1);
    draw.draw_line(x + 3, y, x + 1, y + 2, Color::Black, DotPixel::Pixel1x1);
    draw.draw_line(x + 3, y, x + 5, y + 2, Color::Black, DotPixel::Pixel1x1);
  } else if (change_24h < 0) {
    // Down arrow
    draw.draw_line(x + 3, y, x + 3, y + 5, Color::Black, DotPixel::Pixel1x1);
    draw.draw_line(x + 3, y + 5, x + 1, y + 3, Color::Black, DotPixel::Pixel1x1);
    draw.draw_line(x + 3, y + 5, x + 5, y + 3, Color::Black, DotPixel::Pixel1x1);
  }
}

// Format price with commas
auto format_price(double price) -> std::string {
  std::ostringstream ss;
  ss.precision(2);
  ss << std::fixed << price;
  std::string str = ss.str();

  // Add commas
  size_t decimal_pos = str.find('.');
  if (decimal_pos == std::string::npos) {
    decimal_pos = str.length();
  }

  int insert_pos = static_cast<int>(decimal_pos) - 3;
  while (insert_pos > 0) {
    str.insert(static_cast<size_t>(insert_pos), ",");
    insert_pos -= 3;
  }

  return "$" + str;
}

// Render the dashboard
auto render_dashboard(Screen &screen, Draw &draw, const CryptoPrice &btc, const CryptoPrice &eth,
                      const WalletBalance &wallet) -> void {
  screen.clear(Color::White);

  // Check if we have valid data
  if (!btc.valid && !eth.valid) {
    draw.draw_string(10, 50, "ERROR: Failed to", Font::font16(), Color::Black, Color::White);
    draw.draw_string(10, 70, "fetch crypto prices", Font::font16(), Color::Black, Color::White);
    draw.draw_string(10, 95, "Check network", Font::font12(), Color::Black, Color::White);
    draw.draw_string(10, 110, "connection", Font::font12(), Color::Black, Color::White);
    screen.refresh();
    return;
  }

  // Header
  draw.draw_string(5, 2, "CRYPTO DASHBOARD", Font::font12(), Color::Black, Color::White);

  // Bitcoin section
  std::string btc_price_str = "BTC: " + format_price(btc.price);
  draw.draw_string(5, 16, btc_price_str, Font::font12(), Color::Black, Color::White);

  // Price indicator
  draw_price_indicator(draw, 120, 18, btc.change_24h);

  // 24h change
  std::ostringstream btc_change;
  btc_change.precision(2);
  btc_change << std::fixed << (btc.change_24h >= 0 ? "+" : "") << btc.change_24h << "%";
  draw.draw_string(130, 16, btc_change.str(), Font::font12(), Color::Black, Color::White);

  // Line chart for BTC (30-day trend)
  if (!btc.history.empty()) {
    draw_line_chart(draw, 5, 32, 100, 25, btc.history);
  }

  // Separator
  draw.draw_line(5, 62, 255, 62, Color::Black, DotPixel::Pixel1x1, LineStyle::Solid);

  // Ethereum section
  std::string eth_price_str = "ETH: " + format_price(eth.price);
  draw.draw_string(5, 66, eth_price_str, Font::font12(), Color::Black, Color::White);

  // Price indicator
  draw_price_indicator(draw, 120, 68, eth.change_24h);

  // 24h change
  std::ostringstream eth_change;
  eth_change.precision(2);
  eth_change << std::fixed << (eth.change_24h >= 0 ? "+" : "") << eth.change_24h << "%";
  draw.draw_string(130, 66, eth_change.str(), Font::font12(), Color::Black, Color::White);

  // Line chart for ETH (30-day trend)
  if (!eth.history.empty()) {
    draw_line_chart(draw, 5, 82, 100, 18, eth.history);
  }

  // Separator
  draw.draw_line(5, 105, 255, 105, Color::Black, DotPixel::Pixel1x1, LineStyle::Solid);

  // Wallets section
  draw.draw_string(5, 109, "WALLETS", Font::font12(), Color::Black, Color::White);

  // BTC balance
  std::ostringstream btc_balance;
  btc_balance.precision(4);
  btc_balance << std::fixed << "BTC: " << wallet.btc_balance << " BTC";
  draw.draw_string(5, 123, btc_balance.str(), Font::font12(), Color::Black, Color::White);

  double btc_value = wallet.btc_balance * btc.price;
  draw.draw_string(150, 123, format_price(btc_value), Font::font12(), Color::Black, Color::White);

  // ETH balance
  std::ostringstream eth_balance;
  eth_balance.precision(4);
  eth_balance << std::fixed << "ETH: " << wallet.eth_balance << " ETH";
  draw.draw_string(5, 137, eth_balance.str(), Font::font12(), Color::Black, Color::White);

  double eth_value = wallet.eth_balance * eth.price;
  draw.draw_string(150, 137, format_price(eth_value), Font::font12(), Color::Black, Color::White);

  // Total value (adjusted to fit within 176 pixel height)
  double total_value = btc_value + eth_value;
  draw.draw_string(5, 154, "Total:", Font::font12(), Color::Black, Color::White);
  draw.draw_string(50, 152, format_price(total_value), Font::font16(), Color::Black, Color::White);

  screen.refresh();
}

// Load wallet addresses from config file
auto load_wallet_config(const std::string &filename) -> std::pair<std::vector<std::string>, std::vector<std::string>> {
  std::vector<std::string> btc_addresses;
  std::vector<std::string> eth_addresses;

  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cout << "Config file not found, using example addresses\n";
    // Use example addresses for demonstration
    btc_addresses.push_back("1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa");         // Genesis block
    eth_addresses.push_back("0xde0b295669a9fd93d5f28d9ec85e40f4cb697bae"); // Example
    return {btc_addresses, eth_addresses};
  }

  std::string line;
  bool in_btc_section = false;
  bool in_eth_section = false;

  while (std::getline(file, line)) {
    // Remove whitespace
    line.erase(0, line.find_first_not_of(" \t\r\n"));
    line.erase(line.find_last_not_of(" \t\r\n") + 1);

    if (line.find("\"bitcoin\"") != std::string::npos) {
      in_btc_section = true;
      in_eth_section = false;
    } else if (line.find("\"ethereum\"") != std::string::npos) {
      in_btc_section = false;
      in_eth_section = true;
    }

    // Extract addresses (simple parser for quoted strings)
    if ((in_btc_section || in_eth_section) && line.find("\"") != std::string::npos) {
      size_t first_quote = line.find("\"");
      size_t second_quote = line.find("\"", first_quote + 1);
      if (second_quote != std::string::npos) {
        std::string address = line.substr(first_quote + 1, second_quote - first_quote - 1);
        if (!address.empty() && address != "bitcoin" && address != "ethereum") {
          if (in_btc_section) {
            btc_addresses.push_back(address);
          } else if (in_eth_section) {
            eth_addresses.push_back(address);
          }
        }
      }
    }
  }

  // If no addresses found, use examples
  if (btc_addresses.empty()) {
    btc_addresses.push_back("1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa");
  }
  if (eth_addresses.empty()) {
    eth_addresses.push_back("0xde0b295669a9fd93d5f28d9ec85e40f4cb697bae");
  }

  return {btc_addresses, eth_addresses};
}

auto main() -> int {
  try {
    std::cout << "Crypto Dashboard Demo\n";
    std::cout << "====================\n";
    std::cout << "Press Ctrl+C to stop gracefully\n\n";

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
    g_epd27 = &epd27; // Set global pointer for signal handler

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

    // Load wallet configuration
    std::cout << "Loading wallet configuration...\n";
    auto [btc_addresses, eth_addresses] = load_wallet_config("wallets.json");
    std::cout << "Loaded " << btc_addresses.size() << " BTC addresses and " << eth_addresses.size()
              << " ETH addresses\n\n";

    // Create HTTP client
    HTTPClient client;

    // Main update loop
    int update_count = 0;
    while (g_running) {
      update_count++;
      std::cout << "Fetching data (update " << update_count << ")...\n";

      // Fetch current prices
      std::cout << "  Fetching crypto prices...\n";
      auto prices = fetch_crypto_prices(client);
      if (!prices) {
        std::cerr << "  Failed to fetch prices: " << prices.error() << "\n";

        // Display error on screen
        CryptoPrice empty_btc, empty_eth;
        WalletBalance empty_wallet;
        render_dashboard(screen, draw, empty_btc, empty_eth, empty_wallet);

        // Wait before retry
        std::cout << "  Waiting 30 seconds before retry...\n\n";
        for (int i = 0; i < 30 && g_running; ++i) {
          std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        continue;
      }

      auto [btc, eth] = *prices;
      std::cout << "  BTC: $" << btc.price << " (" << btc.change_24h << "%)\n";
      std::cout << "  ETH: $" << eth.price << " (" << eth.change_24h << "%)\n";

      // Fetch price history
      if (g_running) {
        std::cout << "  Fetching price history (30 days)...\n";
        btc.history = fetch_price_history(client, "bitcoin", 30);
        eth.history = fetch_price_history(client, "ethereum", 30);
        std::cout << "  BTC history: " << btc.history.size() << " points (30d)\n";
        std::cout << "  ETH history: " << eth.history.size() << " points (30d)\n";
      }

      // Fetch wallet balances
      if (g_running) {
        std::cout << "  Fetching wallet balances...\n";
        auto wallet = fetch_wallet_balances(client, btc_addresses, eth_addresses);
        std::cout << "  BTC balance: " << wallet.btc_balance << " BTC\n";
        std::cout << "  ETH balance: " << wallet.eth_balance << " ETH\n";

        // Render dashboard
        std::cout << "  Rendering dashboard...\n";
        render_dashboard(screen, draw, btc, eth, wallet);

        std::cout << "Dashboard updated successfully!\n\n";
      }

      // Wait before next update (interruptible)
      if (g_running) {
        std::cout << "Waiting 5 minutes before next update (Ctrl+C to stop)...\n\n";
        for (int i = 0; i < 300 && g_running; ++i) { // 300 seconds = 5 minutes
          std::this_thread::sleep_for(std::chrono::seconds(1));
        }
      }
    }

    // Clean shutdown
    std::cout << "\nCleaning up...\n";
    std::cout << "Clearing display...\n";
    screen.clear(Color::White);
    screen.refresh();

    std::cout << "Putting display to sleep...\n";
    epd27.sleep();

    std::cout << "\nShutdown complete!\n";
    return EXIT_SUCCESS;

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";

    // Try to clean up display on error
    if (g_epd27) {
      try {
        std::cout << "Attempting to clear display before exit...\n";
        g_epd27->clear();
        g_epd27->sleep();
      } catch (...) {
        // Ignore cleanup errors
      }
    }

    return EXIT_FAILURE;
  }
}
