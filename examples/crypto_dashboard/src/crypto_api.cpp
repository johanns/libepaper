#include "crypto_api.hpp"
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>

using json = nlohmann::json;

namespace crypto_dashboard {

// ============================================================================
// CoinGeckoAPI Implementation
// ============================================================================

auto CoinGeckoAPI::symbol_to_coingecko_id(const std::string &symbol) -> std::string {
  if (symbol == "BTC")
    return "bitcoin";
  if (symbol == "ETH")
    return "ethereum";
  return symbol;
}

auto CoinGeckoAPI::fetch_price(const std::string &symbol) const -> std::expected<CryptoPrice, std::string> {

  const auto coin_id = symbol_to_coingecko_id(symbol);
  const std::string url =
      "https://api.coingecko.com/api/v3/simple/price?ids=" + coin_id + "&vs_currencies=usd&include_24hr_change=true";

  auto response = client_.get(url);
  if (!response) {
    return std::unexpected(response.error());
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
    return std::unexpected(response.error());
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
// BitcoinBlockchainAPI Implementation
// ============================================================================

auto BitcoinBlockchainAPI::fetch_balance(const std::string &address) const -> std::expected<double, std::string> {

  // Use the multiaddr API which supports all address types including native SegWit (bc1...)
  const std::string url = "https://blockchain.info/multiaddr?active=" + address;

  auto response = client_.get(url);
  if (!response) {
    return std::unexpected(response.error());
  }

  try {
    // Parse JSON response
    auto j = json::parse(*response);

    // Extract balance from addresses array
    if (j.contains("addresses") && j["addresses"].is_array() && !j["addresses"].empty()) {
      const auto &addr_info = j["addresses"][0];
      if (addr_info.contains("final_balance")) {
        const long long satoshi = addr_info["final_balance"].get<long long>();
        const double btc = static_cast<double>(satoshi) / 1e8;
        return btc;
      }
    }

    return std::unexpected("Balance not found in API response");

  } catch (const json::exception &e) {
    return std::unexpected(std::string("Failed to parse BTC balance: ") + e.what());
  } catch (const std::exception &e) {
    return std::unexpected(std::string("Failed to convert BTC balance: ") + e.what());
  }
}

// ============================================================================
// EtherscanAPI Implementation
// ============================================================================

auto EtherscanAPI::fetch_balance(const std::string &address) const -> std::expected<double, std::string> {

  if (!has_api_key()) {
    return std::unexpected("Etherscan API key required");
  }

  const std::string url = "https://api.etherscan.io/v2/api?chainid=1&module=account&action=balance&address=" + address +
                          "&tag=latest&apikey=" + api_key_;

  auto response = client_.get(url);
  if (!response) {
    return std::unexpected(response.error());
  }

  try {
    auto j = json::parse(*response);

    // Check for V2 API status
    if (j.contains("status") && j["status"].get<std::string>() == "1" && j.contains("result")) {
      const std::string result_str = j["result"].get<std::string>();

      // Use long double to handle very large wei values
      const long double wei = std::stold(result_str);
      const double eth = static_cast<double>(wei / 1e18L);

      return eth;
    }

    // Error case
    std::string error_msg = "Etherscan API error";
    if (j.contains("message")) {
      error_msg += ": " + j["message"].get<std::string>();
    }
    return std::unexpected(error_msg);

  } catch (const json::exception &e) {
    return std::unexpected(std::string("JSON parse error: ") + e.what());
  } catch (const std::exception &e) {
    return std::unexpected(std::string("Failed to convert ETH balance: ") + e.what());
  }
}

// ============================================================================
// CryptoDataFetcher Implementation
// ============================================================================

CryptoDataFetcher::CryptoDataFetcher(const HTTPClient &client, std::string etherscan_api_key)
    : client_(client), coingecko_api_(client), bitcoin_api_(client),
      etherscan_api_(client, std::move(etherscan_api_key)) {}

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

auto CryptoDataFetcher::fetch_wallet_balances(const std::vector<std::string> &btc_addresses,
                                              const std::vector<std::string> &eth_addresses) const -> WalletBalance {

  std::cout << "  Fetching wallet balances...\n";

  WalletBalance balance;

  // Fetch Bitcoin balances
  for (const auto &address : btc_addresses) {
    auto result = bitcoin_api_.fetch_balance(address);
    if (result) {
      balance.btc_balance += *result;
      std::cout << "    BTC address " << address.substr(0, 8) << "...: " << *result << " BTC\n";
    } else {
      std::cerr << "    Failed to fetch BTC balance for " << address << "\n";
      std::cerr << "      Error: " << result.error() << "\n";
      std::cerr << "      (Skipping this address, continuing with others)\n";
    }
  }

  // Fetch Ethereum balances
  if (!etherscan_api_.has_api_key()) {
    balance.eth_api_key_required = true;
    std::cout << "    Etherscan API key not provided - skipping ETH balance fetch\n";
  } else {
    for (const auto &address : eth_addresses) {
      auto result = etherscan_api_.fetch_balance(address);
      if (result) {
        balance.eth_balance += *result;
        std::cout << "    ETH address " << address.substr(0, 10) << "...: " << *result << " ETH\n";
      } else {
        std::cerr << "    Failed to fetch ETH balance for " << address << "\n";
        std::cerr << "      Error: " << result.error() << "\n";
        std::cerr << "      (Skipping this address, continuing with others)\n";
      }
    }
  }

  return balance;
}

} // namespace crypto_dashboard
