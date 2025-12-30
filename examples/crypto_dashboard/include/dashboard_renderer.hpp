#pragma once

#include "types.hpp"
#include <epaper/display.hpp>
#include <string>

namespace crypto_dashboard {

/// Renders the cryptocurrency dashboard to an e-Paper display
/// Uses composition to hold drawing context
class DashboardRenderer {
public:
  DashboardRenderer(epaper::Display &display);

  /// Render the complete dashboard
  void render(const CryptoPrice &btc, const CryptoPrice &eth, const PriceHistory &btc_history,
              const PriceHistory &eth_history, const WalletBalance &wallet);

  /// Render an error message on the display
  void render_error(const std::string &error_message);

  /// Clear the display
  void clear();

private:
  // Drawing helper methods
  void draw_header();
  void draw_price_section(const CryptoPrice &btc, const CryptoPrice &eth);
  void draw_price_indicator(size_t x, size_t y, bool positive, double change);
  void draw_charts(const CryptoPrice &btc, const CryptoPrice &eth, const PriceHistory &btc_history,
                   const PriceHistory &eth_history);
  void draw_line_chart(size_t x, size_t y, size_t width, size_t height, const std::vector<double> &data);
  void draw_wallet_section(const CryptoPrice &btc, const CryptoPrice &eth, const WalletBalance &wallet);

  // Formatting helpers
  [[nodiscard]] static auto format_price(double price) -> std::string;
  [[nodiscard]] static auto format_change(double change) -> std::string;

  epaper::Display &display_;
};

} // namespace crypto_dashboard
