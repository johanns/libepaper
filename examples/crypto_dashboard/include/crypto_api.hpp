#pragma once

#include "http_client.hpp"
#include "types.hpp"
#include <expected>
#include <string>
#include <vector>

namespace crypto_dashboard {

/// Base interface for cryptocurrency price APIs
/// Uses composition - contains an HTTPClient rather than inheriting
class CryptoPriceAPI {
public:
  explicit CryptoPriceAPI(const HTTPClient &client) : client_(client) {}
  virtual ~CryptoPriceAPI() = default;

  /// Fetch current price for a cryptocurrency
  [[nodiscard]] virtual auto fetch_price(const std::string &symbol) const
      -> std::expected<CryptoPrice, std::string> = 0;

  /// Fetch price history for a cryptocurrency
  [[nodiscard]] virtual auto fetch_history(const std::string &symbol, int days) const
      -> std::expected<PriceHistory, std::string> = 0;

protected:
  const HTTPClient &client_;
};

/// CoinGecko API implementation
class CoinGeckoAPI final : public CryptoPriceAPI {
public:
  explicit CoinGeckoAPI(const HTTPClient &client) : CryptoPriceAPI(client) {}

  [[nodiscard]] auto fetch_price(const std::string &symbol) const -> std::expected<CryptoPrice, std::string> override;

  [[nodiscard]] auto fetch_history(const std::string &symbol, int days) const
      -> std::expected<PriceHistory, std::string> override;

private:
  [[nodiscard]] static auto symbol_to_coingecko_id(const std::string &symbol) -> std::string;
};

/// High-level API aggregator that composes multiple API clients
/// Demonstrates composition over inheritance
class CryptoDataFetcher {
public:
  explicit CryptoDataFetcher(const HTTPClient &client);

  /// Fetch prices for Bitcoin and Ethereum
  [[nodiscard]] auto fetch_crypto_prices() const -> std::expected<std::pair<CryptoPrice, CryptoPrice>, std::string>;

  /// Fetch price history for a cryptocurrency
  [[nodiscard]] auto fetch_price_history(const std::string &symbol, int days) const
      -> std::expected<PriceHistory, std::string>;

private:
  const HTTPClient &client_;
  CoinGeckoAPI coingecko_api_;
};

} // namespace crypto_dashboard
