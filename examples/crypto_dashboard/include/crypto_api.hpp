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

/// Bitcoin blockchain API implementation
class BitcoinBlockchainAPI {
public:
  explicit BitcoinBlockchainAPI(const HTTPClient &client) : client_(client) {}

  /// Fetch balance for a Bitcoin address
  [[nodiscard]] auto fetch_balance(const std::string &address) const -> std::expected<double, std::string>;

private:
  const HTTPClient &client_;
};

/// Etherscan API implementation
class EtherscanAPI {
public:
  explicit EtherscanAPI(const HTTPClient &client, std::string api_key)
      : client_(client), api_key_(std::move(api_key)) {}

  /// Fetch balance for an Ethereum address
  [[nodiscard]] auto fetch_balance(const std::string &address) const -> std::expected<double, std::string>;

  [[nodiscard]] auto has_api_key() const noexcept -> bool { return !api_key_.empty(); }

private:
  const HTTPClient &client_;
  std::string api_key_;
};

/// High-level API aggregator that composes multiple API clients
/// Demonstrates composition over inheritance
class CryptoDataFetcher {
public:
  CryptoDataFetcher(const HTTPClient &client, std::string etherscan_api_key = "");

  /// Fetch prices for Bitcoin and Ethereum
  [[nodiscard]] auto fetch_crypto_prices() const -> std::expected<std::pair<CryptoPrice, CryptoPrice>, std::string>;

  /// Fetch price history for a cryptocurrency
  [[nodiscard]] auto fetch_price_history(const std::string &symbol, int days) const
      -> std::expected<PriceHistory, std::string>;

  /// Fetch wallet balances
  [[nodiscard]] auto fetch_wallet_balances(const std::vector<std::string> &btc_addresses,
                                           const std::vector<std::string> &eth_addresses) const -> WalletBalance;

private:
  const HTTPClient &client_;
  CoinGeckoAPI coingecko_api_;
  BitcoinBlockchainAPI bitcoin_api_;
  EtherscanAPI etherscan_api_;
};

} // namespace crypto_dashboard
