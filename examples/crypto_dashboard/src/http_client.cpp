#include "http_client.hpp"
#include <curl/curl.h>
#include <sstream>

namespace crypto_dashboard {

// Pimpl implementation to hide cURL details
struct HTTPClient::Impl {
  CURL *curl = nullptr;

  Impl() {
    curl = curl_easy_init();
    if (curl) {
      // Set common options
      curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
      curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L); // 10 second timeout for faster shutdown
      curl_easy_setopt(curl, CURLOPT_USERAGENT, "CryptoDashboard/1.0");
    }
  }

  ~Impl() {
    if (curl) {
      curl_easy_cleanup(curl);
    }
  }

  // Non-copyable
  Impl(const Impl &) = delete;
  Impl &operator=(const Impl &) = delete;

  // Movable
  Impl(Impl &&other) noexcept : curl(other.curl) { other.curl = nullptr; }

  Impl &operator=(Impl &&other) noexcept {
    if (this != &other) {
      if (curl) {
        curl_easy_cleanup(curl);
      }
      curl = other.curl;
      other.curl = nullptr;
    }
    return *this;
  }

  static auto write_callback(void *contents, size_t size, size_t nmemb, void *userp) -> size_t {
    size_t total_size = size * nmemb;
    auto *str = static_cast<std::string *>(userp);
    str->append(static_cast<char *>(contents), total_size);
    return total_size;
  }
};

HTTPClient::HTTPClient() : impl_(std::make_unique<Impl>()) {
  if (!impl_->curl) {
    throw std::runtime_error("Failed to initialize cURL");
  }
}

HTTPClient::~HTTPClient() = default;

HTTPClient::HTTPClient(HTTPClient &&) noexcept = default;
HTTPClient &HTTPClient::operator=(HTTPClient &&) noexcept = default;

auto HTTPClient::get(const std::string &url) const -> std::expected<std::string, std::string> {
  if (!impl_->curl) {
    return std::unexpected("cURL not initialized");
  }

  std::string response_data;

  // Set URL and callbacks
  curl_easy_setopt(impl_->curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(impl_->curl, CURLOPT_WRITEFUNCTION, Impl::write_callback);
  curl_easy_setopt(impl_->curl, CURLOPT_WRITEDATA, &response_data);

  // Perform request
  CURLcode res = curl_easy_perform(impl_->curl);

  if (res != CURLE_OK) {
    std::ostringstream error_msg;
    error_msg << "cURL error: " << curl_easy_strerror(res);
    return std::unexpected(error_msg.str());
  }

  // Check HTTP response code
  long http_code = 0;
  curl_easy_getinfo(impl_->curl, CURLINFO_RESPONSE_CODE, &http_code);

  if (http_code != 200) {
    std::ostringstream error_msg;
    error_msg << "HTTP error: " << http_code;
    return std::unexpected(error_msg.str());
  }

  return response_data;
}

} // namespace crypto_dashboard
