#include "crypto_api.hpp"
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace crypto_dashboard {

// ============================================================================
// CoinGeckoAPI Implementation
// ============================================================================

auto CoinGeckoAPI::symbol_to_coingecko_id(const std::string &symbol) -> std::string {
  if (symbol == "BTC") {
    return "bitcoin";
  }
  if (symbol == "ETH") {
    return "ethereum";
  }
  return symbol;
}

auto CoinGeckoAPI::fetch_price(const std::string &symbol) const -> std::expected<CryptoPrice, std::string> {

  const auto coin_id = symbol_to_coingecko_id(symbol);
  const std::string url =
      "https://api.coingecko.com/api/v3/simple/price?ids=" + coin_id + "&vs_currencies=usd&include_24hr_change=true";

  auto response = client_.get(url);
  if (!response) {
    // Include URL in error for debugging
    return std::unexpected(response.error() + " (URL: " + url + ")");
  }

  try {
    auto j = json::parse(*response);

    if (!j.contains(coin_id)) {
      return std::unexpected("Coin not found in response");
    }

    const auto &coin_data = j[coin_id];

    CryptoPrice price;
    price.symbol = symbol;
    price.name = coin_id;
    price.price = coin_data.value("usd", 0.0);
    price.change_24h = coin_data.value("usd_24h_change", 0.0);
    price.valid = price.price > 0.0;

    return price;

  } catch (const json::exception &e) {
    return std::unexpected(std::string("JSON parse error: ") + e.what());
  }
}

auto CoinGeckoAPI::fetch_history(const std::string &symbol, int days) const
    -> std::expected<PriceHistory, std::string> {

  const auto coin_id = symbol_to_coingecko_id(symbol);
  const std::string url = "https://api.coingecko.com/api/v3/coins/" + coin_id +
                          "/market_chart?vs_currency=usd&days=" + std::to_string(days);

  auto response = client_.get(url);
  if (!response) {
    // Include URL in error for debugging
    return std::unexpected(response.error() + " (URL: " + url + ")");
  }

  try {
    auto j = json::parse(*response);

    if (!j.contains("prices") || !j["prices"].is_array()) {
      return std::unexpected("Invalid price history format");
    }

    PriceHistory history;
    history.symbol = symbol;
    history.days = days;

    for (const auto &price_point : j["prices"]) {
      if (price_point.is_array() && price_point.size() >= 2) {
        history.prices.push_back(price_point[1].get<double>());
      }
    }

    return history;

  } catch (const json::exception &e) {
    return std::unexpected(std::string("JSON parse error: ") + e.what());
  }
}

// ============================================================================
// CryptoDataFetcher Implementation
// ============================================================================

CryptoDataFetcher::CryptoDataFetcher(const HTTPClient &client) : client_(client), coingecko_api_(client) {}

auto CryptoDataFetcher::fetch_crypto_prices() const -> std::expected<std::pair<CryptoPrice, CryptoPrice>, std::string> {

  std::cout << "  Fetching crypto prices...\n";

  auto btc_result = coingecko_api_.fetch_price("BTC");
  if (!btc_result) {
    return std::unexpected("Failed to fetch BTC price: " + btc_result.error());
  }

  auto eth_result = coingecko_api_.fetch_price("ETH");
  if (!eth_result) {
    return std::unexpected("Failed to fetch ETH price: " + eth_result.error());
  }

  std::cout << "    BTC: $" << btc_result->price << " (" << (btc_result->change_24h > 0 ? "+" : "")
            << btc_result->change_24h << "%)\n";
  std::cout << "    ETH: $" << eth_result->price << " (" << (eth_result->change_24h > 0 ? "+" : "")
            << eth_result->change_24h << "%)\n";

  return std::make_pair(*btc_result, *eth_result);
}

auto CryptoDataFetcher::fetch_price_history(const std::string &symbol, int days) const
    -> std::expected<PriceHistory, std::string> {

  std::cout << "  Fetching " << symbol << " price history (" << days << " days)...\n";

  auto result = coingecko_api_.fetch_history(symbol, days);
  if (!result) {
    return std::unexpected("Failed to fetch " + symbol + " history: " + result.error());
  }

  std::cout << "    " << symbol << " history: " << result->size() << " points\n";

  return result;
}

} // namespace crypto_dashboard
