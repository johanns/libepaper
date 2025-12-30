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

/// Application configuration
struct AppConfig {
  int screen_flip_interval_seconds = 60; // Interval between screen rotations
  int data_fetch_interval_seconds = 900; // Interval between data fetches (15 minutes)
};

} // namespace crypto_dashboard
