#include "wallet_config.hpp"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace crypto_dashboard {

auto WalletConfigLoader::load(const std::filesystem::path &filepath) -> std::expected<WalletConfig, std::string> {

  if (!exists(filepath)) {
    return std::unexpected("Configuration file not found: " + filepath.string());
  }

  try {
    std::ifstream file(filepath);
    if (!file.is_open()) {
      return std::unexpected("Failed to open configuration file: " + filepath.string());
    }

    json j;
    file >> j;

    WalletConfig config;

    // Load Bitcoin addresses
    if (j.contains("bitcoin_addresses") && j["bitcoin_addresses"].is_array()) {
      for (const auto &addr : j["bitcoin_addresses"]) {
        if (addr.is_string()) {
          config.btc_addresses.push_back(addr.get<std::string>());
        }
      }
    }

    // Load Ethereum addresses
    if (j.contains("ethereum_addresses") && j["ethereum_addresses"].is_array()) {
      for (const auto &addr : j["ethereum_addresses"]) {
        if (addr.is_string()) {
          config.eth_addresses.push_back(addr.get<std::string>());
        }
      }
    }

    return config;

  } catch (const json::exception &e) {
    return std::unexpected(std::string("JSON parse error: ") + e.what());
  } catch (const std::exception &e) {
    return std::unexpected(std::string("Error loading config: ") + e.what());
  }
}

auto WalletConfigLoader::exists(const std::filesystem::path &filepath) -> bool {
  return std::filesystem::exists(filepath);
}

auto WalletConfigLoader::create_example(const std::filesystem::path &filepath) -> bool {
  try {
    json j;
    j["bitcoin_addresses"] = json::array({
        "1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa", // Example: Genesis block address
        "3J98t1WpEZ73CNmYviecrnyiWrnqRhWNLy"  // Example: Another address
    });
    j["ethereum_addresses"] = json::array({
        "0xde0B295669a9FD93d5F28D9Ec85E40f4cb697BAe", // Example: Ethereum Foundation
        "0x00000000219ab540356cBB839Cbe05303d7705Fa"  // Example: ETH2 Deposit Contract
    });

    std::ofstream file(filepath);
    if (!file.is_open()) {
      return false;
    }

    file << j.dump(2) << "\n"; // Pretty print with 2-space indent
    return true;

  } catch (...) {
    return false;
  }
}

} // namespace crypto_dashboard
