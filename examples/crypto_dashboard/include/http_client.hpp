#pragma once

#include <expected>
#include <memory>
#include <string>

// Forward declare CURL to avoid including curl.h in header
typedef void CURL;

namespace crypto_dashboard {

/// HTTP client for making GET requests
/// Uses RAII to manage cURL resources
class HTTPClient {
public:
  HTTPClient();
  ~HTTPClient();

  // Non-copyable but movable
  HTTPClient(const HTTPClient &) = delete;
  HTTPClient &operator=(const HTTPClient &) = delete;
  HTTPClient(HTTPClient &&) noexcept;
  HTTPClient &operator=(HTTPClient &&) noexcept;

  /// Perform HTTP GET request
  /// @param url The URL to fetch
  /// @return Response body on success, error message on failure
  [[nodiscard]] auto get(const std::string &url) const -> std::expected<std::string, std::string>;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

} // namespace crypto_dashboard
