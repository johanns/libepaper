#pragma once

#include <string>
#include <vector>

namespace crypto_dashboard {

/// Represents cryptocurrency price information
struct CryptoPrice {
  std::string symbol;      // e.g., "BTC", "ETH"
  std::string name;        // e.g., "Bitcoin", "Ethereum"
  double price = 0.0;      // Current price in USD
  double change_24h = 0.0; // 24-hour percentage change
  bool valid = false;      // Whether data is valid

  [[nodiscard]] constexpr auto is_valid() const noexcept -> bool { return valid; }
  [[nodiscard]] constexpr auto is_positive_change() const noexcept -> bool { return change_24h > 0.0; }
};

/// Represents price history over time
struct PriceHistory {
  std::string symbol;         // e.g., "BTC", "ETH"
  std::vector<double> prices; // Historical prices
  int days = 0;               // Number of days covered

  [[nodiscard]] auto size() const noexcept -> size_t { return prices.size(); }
  [[nodiscard]] auto empty() const noexcept -> bool { return prices.empty(); }
  [[nodiscard]] auto min_price() const -> double;
  [[nodiscard]] auto max_price() const -> double;
};

/// Represents wallet balance information
struct WalletBalance {
  double btc_balance = 0.0;          // Bitcoin balance
  double eth_balance = 0.0;          // Ethereum balance
  bool eth_api_key_required = false; // Whether ETH API key was missing

  [[nodiscard]] constexpr auto has_eth_balance() const noexcept -> bool { return !eth_api_key_required; }

  [[nodiscard]] constexpr auto total_value(double btc_price, double eth_price) const noexcept -> double {
    double total = btc_balance * btc_price;
    if (has_eth_balance()) {
      total += eth_balance * eth_price;
    }
    return total;
  }
};

/// Configuration for cryptocurrency wallets
struct WalletConfig {
  std::vector<std::string> btc_addresses;
  std::vector<std::string> eth_addresses;

  [[nodiscard]] auto has_btc_addresses() const noexcept -> bool { return !btc_addresses.empty(); }
  [[nodiscard]] auto has_eth_addresses() const noexcept -> bool { return !eth_addresses.empty(); }
};

/// Application configuration
struct AppConfig {
  WalletConfig wallets;
  std::string etherscan_api_key;
  int update_interval_seconds = 30;

  [[nodiscard]] auto has_etherscan_key() const noexcept -> bool { return !etherscan_api_key.empty(); }
};

} // namespace crypto_dashboard
