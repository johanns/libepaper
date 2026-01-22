#pragma once

#include "types.hpp"
#include <epaper/core/display.hpp>
#include <string>

namespace crypto_dashboard {

/// Screen types for the rotating display
enum class ScreenType {
  Combined,     ///< BTC and ETH prices with side-by-side 30-day graphs
  BTCDedicated, ///< BTC price with stacked 30-day and 6-month graphs
  ETHDedicated  ///< ETH price with stacked 30-day and 6-month graphs
};

/// Renders the cryptocurrency dashboard to an e-Paper display
/// Uses composition to hold drawing context
template <typename DisplayT> class DashboardRenderer {
public:
  explicit DashboardRenderer(DisplayT &display) : display_(display) {}

  /// Render the dashboard based on screen type
  void render(ScreenType screen_type, const CryptoPrice &btc, const CryptoPrice &eth, const PriceHistory &btc_30d,
              const PriceHistory &eth_30d, const PriceHistory &btc_6mo, const PriceHistory &eth_6mo);

  /// Render an error message on the display
  void render_error(const std::string &error_message);

  /// Clear the display
  void clear();

private:
  // Screen-specific rendering methods
  void render_combined_screen(const CryptoPrice &btc, const CryptoPrice &eth, const PriceHistory &btc_30d,
                              const PriceHistory &eth_30d);
  void render_btc_dedicated_screen(const CryptoPrice &btc, const PriceHistory &btc_30d, const PriceHistory &btc_6mo);
  void render_eth_dedicated_screen(const CryptoPrice &eth, const PriceHistory &eth_30d, const PriceHistory &eth_6mo);

  // Drawing helper methods
  void draw_header();
  void draw_price_section(const CryptoPrice &btc, const CryptoPrice &eth);
  void draw_price_section_single(const CryptoPrice &price, const std::string &label);
  void draw_price_indicator(size_t x, size_t y, bool positive, double change);
  void draw_charts_side_by_side(const PriceHistory &btc_history, const PriceHistory &eth_history);
  void draw_charts_stacked(const PriceHistory &history_30d, const PriceHistory &history_6mo);
  void draw_line_chart(size_t x, size_t y, size_t width, size_t height, const std::vector<double> &data);

  // Formatting helpers
  [[nodiscard]] static auto format_price(double price) -> std::string;
  [[nodiscard]] static auto format_change(double change) -> std::string;

  DisplayT &display_;
};

} // namespace crypto_dashboard
