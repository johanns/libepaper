#pragma once

#include "types.hpp"
#include <expected>
#include <filesystem>
#include <string>

namespace crypto_dashboard {

/// Loads wallet configuration from JSON file
class WalletConfigLoader {
public:
  /// Load wallet configuration from a JSON file
  /// @param filepath Path to the JSON configuration file
  /// @return WalletConfig on success, error message on failure
  [[nodiscard]] static auto load(const std::filesystem::path &filepath) -> std::expected<WalletConfig, std::string>;

  /// Check if a configuration file exists
  [[nodiscard]] static auto exists(const std::filesystem::path &filepath) -> bool;

  /// Create an example configuration file
  [[nodiscard]] static auto create_example(const std::filesystem::path &filepath) -> bool;
};

} // namespace crypto_dashboard
