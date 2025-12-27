#include "dashboard_renderer.hpp"
#include <algorithm>
#include <iomanip>
#include <sstream>

using namespace epaper;

namespace crypto_dashboard {

DashboardRenderer::DashboardRenderer(Screen &screen, Draw &draw) : screen_(screen), draw_(draw) {}

void DashboardRenderer::render(const CryptoPrice &btc, const CryptoPrice &eth, const PriceHistory &btc_history,
                               const PriceHistory &eth_history, const WalletBalance &wallet) {
  screen_.clear();

  draw_header();
  draw_price_section(btc, eth);
  draw_charts(btc, eth, btc_history, eth_history);
  draw_wallet_section(btc, eth, wallet);

  screen_.refresh();
}

void DashboardRenderer::render_error(const std::string &error_message) {
  screen_.clear();

  // Draw error title
  draw_.draw_string(5, 5, "Error", Font::font16(), Color::Black, Color::White);

  // Draw error message (word wrap if needed)
  draw_.draw_string(5, 30, error_message, Font::font12(), Color::Black, Color::White);

  // Draw retry message
  draw_.draw_string(5, 60, "Retrying...", Font::font12(), Color::Black, Color::White);

  screen_.refresh();
}

void DashboardRenderer::clear() {
  screen_.clear();
  screen_.refresh();
}

void DashboardRenderer::draw_header() {
  // Title
  draw_.draw_string(5, 2, "CRYPTO DASHBOARD", Font::font16(), Color::Black, Color::White);

  // Horizontal line under title
  draw_.draw_line(0, 20, screen_.effective_width(), 20, Color::Black);
}

void DashboardRenderer::draw_price_section(const CryptoPrice &btc, const CryptoPrice &eth) {
  constexpr size_t section_y = 24;

  // Bitcoin price
  draw_.draw_string(5, section_y, "BTC", Font::font12(), Color::Black, Color::White);
  draw_.draw_string(40, section_y, format_price(btc.price), Font::font12(), Color::Black, Color::White);
  draw_price_indicator(120, section_y, btc.is_positive_change(), btc.change_24h);

  // Ethereum price
  draw_.draw_string(5, section_y + 14, "ETH", Font::font12(), Color::Black, Color::White);
  draw_.draw_string(40, section_y + 14, format_price(eth.price), Font::font12(), Color::Black, Color::White);
  draw_price_indicator(120, section_y + 14, eth.is_positive_change(), eth.change_24h);
}

void DashboardRenderer::draw_price_indicator(size_t x, size_t y, bool positive, double change) {
  const std::string change_str = format_change(change);

  // Draw arrow indicator
  if (positive) {
    // Up arrow (▲)
    draw_.draw_string(x, y, "^", Font::font12(), Color::Black, Color::White);
  } else {
    // Down arrow (▼)
    draw_.draw_string(x, y, "v", Font::font12(), Color::Black, Color::White);
  }

  // Draw percentage
  draw_.draw_string(x + 10, y, change_str, Font::font12(), Color::Black, Color::White);
}

void DashboardRenderer::draw_charts(const CryptoPrice &btc, const CryptoPrice &eth, const PriceHistory &btc_history,
                                    const PriceHistory &eth_history) {
  constexpr size_t chart_y = 54;      // Moved up from 58 to 50
  constexpr size_t chart_height = 45; // Reduced from 50 to 45
  constexpr size_t chart_width = 120;

  // BTC chart
  draw_.draw_string(5, chart_y, "BTC 30d", Font::font12(), Color::Black, Color::White);
  if (!btc_history.empty()) {
    draw_line_chart(5, chart_y + 12, chart_width, chart_height, btc_history.prices);
  }

  // ETH chart
  draw_.draw_string(135, chart_y, "ETH 30d", Font::font12(), Color::Black, Color::White);
  if (!eth_history.empty()) {
    draw_line_chart(135, chart_y + 12, chart_width, chart_height, eth_history.prices);
  }
}

void DashboardRenderer::draw_line_chart(size_t x, size_t y, size_t width, size_t height,
                                        const std::vector<double> &data) {
  if (data.empty()) {
    return;
  }

  // Draw border
  draw_.draw_rectangle(x, y, x + width, y + height, Color::Black);

  if (data.size() < 2) {
    return;
  }

  // Find min and max for scaling
  const double min_val = *std::ranges::min_element(data);
  const double max_val = *std::ranges::max_element(data);
  const double range = max_val - min_val;

  if (range < 0.0001) {
    // Flat line - draw at middle
    const size_t middle_y = y + height / 2;
    draw_.draw_line(x, middle_y, x + width, middle_y, Color::Black);
    return;
  }

  // Draw line chart
  for (size_t i = 1; i < data.size(); ++i) {
    const double x_scale = static_cast<double>(width) / static_cast<double>(data.size() - 1);
    const size_t x1 = x + static_cast<size_t>((i - 1) * x_scale);
    const size_t x2 = x + static_cast<size_t>(i * x_scale);

    const size_t y1 = y + height - static_cast<size_t>(((data[i - 1] - min_val) / range) * static_cast<double>(height));
    const size_t y2 = y + height - static_cast<size_t>(((data[i] - min_val) / range) * static_cast<double>(height));

    draw_.draw_line(x1, y1, x2, y2, Color::Black);
  }
}

void DashboardRenderer::draw_wallet_section(const CryptoPrice &btc, const CryptoPrice &eth,
                                            const WalletBalance &wallet) {
  constexpr size_t section_y = 120;

  // Section title
  draw_.draw_string(5, section_y, "WALLETS", Font::font12(), Color::Black, Color::White);

  // BTC balance
  std::ostringstream btc_balance;
  btc_balance.precision(4);
  btc_balance << std::fixed << "BTC: " << wallet.btc_balance; // Removed " BTC" suffix
  draw_.draw_string(5, section_y + 14, btc_balance.str(), Font::font12(), Color::Black, Color::White);

  const double btc_value = wallet.btc_balance * btc.price;
  draw_.draw_string(150, section_y + 14, format_price(btc_value), Font::font12(), Color::Black, Color::White);

  // ETH balance
  if (wallet.has_eth_balance()) {
    std::ostringstream eth_balance;
    eth_balance.precision(4);
    eth_balance << std::fixed << "ETH: " << wallet.eth_balance; // Removed " ETH" suffix
    draw_.draw_string(5, section_y + 28, eth_balance.str(), Font::font12(), Color::Black, Color::White);

    const double eth_value = wallet.eth_balance * eth.price;
    draw_.draw_string(150, section_y + 28, format_price(eth_value), Font::font12(), Color::Black, Color::White);
  } else {
    draw_.draw_string(5, section_y + 28, "ETH: API key required", Font::font12(), Color::Black, Color::White);
  }

  // Total value
  const double total_value = wallet.total_value(btc.price, eth.price);
  draw_.draw_string(5, section_y + 45, "Total:", Font::font12(), Color::Black, Color::White);
  draw_.draw_string(50, section_y + 43, format_price(total_value), Font::font16(), Color::Black, Color::White);
}

auto DashboardRenderer::format_price(double price) -> std::string {
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(2) << "$" << price;
  return oss.str();
}

auto DashboardRenderer::format_change(double change) -> std::string {
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(2);
  if (change > 0) {
    oss << "+";
  }
  oss << change << "%";
  return oss.str();
}

} // namespace crypto_dashboard
