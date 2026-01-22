#pragma once

#include <expected>
#include <memory>
#include <string>

// Forward declare CURL to avoid including curl.h in header
using CURL = void;

namespace crypto_dashboard {

/// HTTP client for making GET requests
/// Uses RAII to manage cURL resources
class HTTPClient {
public:
  HTTPClient();
  ~HTTPClient();

  // Non-copyable but movable
  HTTPClient(const HTTPClient &) = delete;
  auto operator=(const HTTPClient &) -> HTTPClient & = delete;
  HTTPClient(HTTPClient &&) noexcept;
  auto operator=(HTTPClient &&) noexcept -> HTTPClient &;

  /// Perform HTTP GET request
  /// @param url The URL to fetch
  /// @return Response body on success, error message on failure
  [[nodiscard]] auto get(const std::string &url) const -> std::expected<std::string, std::string>;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

} // namespace crypto_dashboard
