#include "dashboard_renderer.hpp"
#include <algorithm>
#include <epaper/display.hpp>
#include <epaper/font.hpp>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace epaper;

namespace crypto_dashboard {

DashboardRenderer::DashboardRenderer(Display &display) : display_(display) {}

void DashboardRenderer::render(ScreenType screen_type, const CryptoPrice &btc, const CryptoPrice &eth,
                               const PriceHistory &btc_30d, const PriceHistory &eth_30d, const PriceHistory &btc_6mo,
                               const PriceHistory &eth_6mo) {
  // No need to manually wake - library handles it transparently
  std::cout << "  [Renderer] Clearing display...\n";
  std::cout.flush();
  display_.clear();

  std::cout << "  [Renderer] Drawing screen content...\n";
  std::cout.flush();

  switch (screen_type) {
  case ScreenType::Combined:
    render_combined_screen(btc, eth, btc_30d, eth_30d);
    break;
  case ScreenType::BTCDedicated:
    render_btc_dedicated_screen(btc, btc_30d, btc_6mo);
    break;
  case ScreenType::ETHDedicated:
    render_eth_dedicated_screen(eth, eth_30d, eth_6mo);
    break;
  }

  std::cout << "  [Renderer] Refreshing display (this may take a few seconds)...\n";
  std::cout.flush();

  // Note: refresh() blocks and cannot be interrupted by signals
  // If Ctrl-C is pressed during refresh, exit will happen after refresh completes
  if (auto result = display_.refresh(); !result) {
    std::cerr << "  [Renderer] Warning: Failed to refresh display: " << result.error().what() << "\n";
  } else {
    std::cout << "  [Renderer] Display refresh complete.\n";
    std::cout.flush();
  }
}

void DashboardRenderer::render_error(const std::string &error_message) {
  display_.clear();

  // Draw error title
  display_.draw_string(5, 5, "Error", Font::font16(), Color::Black, Color::White);

  // Draw error message (word wrap if needed)
  display_.draw_string(5, 30, error_message, Font::font12(), Color::Black, Color::White);

  // Draw retry message
  display_.draw_string(5, 60, "Retrying...", Font::font12(), Color::Black, Color::White);

  if (auto result = display_.refresh(); !result) {
    std::cerr << "Warning: Failed to refresh display: " << result.error().what() << "\n";
  }
}

void DashboardRenderer::clear() {
  display_.clear();
  if (auto result = display_.refresh(); !result) {
    std::cerr << "Warning: Failed to refresh display: " << result.error().what() << "\n";
  }
}

void DashboardRenderer::draw_header() {
  // Title
  display_.draw_string(5, 2, "CRYPTO DASHBOARD", Font::font16(), Color::Black, Color::White);

  // Horizontal line under title
  display_.draw_line(0, 20, display_.effective_width(), 20, Color::Black);
}

void DashboardRenderer::draw_price_section(const CryptoPrice &btc, const CryptoPrice &eth) {
  constexpr size_t section_y = 24;

  // Bitcoin price
  display_.draw_string(5, section_y, "BTC", Font::font12(), Color::Black, Color::White);
  display_.draw_string(40, section_y, format_price(btc.price), Font::font12(), Color::Black, Color::White);
  draw_price_indicator(120, section_y, btc.is_positive_change(), btc.change_24h);

  // Ethereum price
  display_.draw_string(5, section_y + 14, "ETH", Font::font12(), Color::Black, Color::White);
  display_.draw_string(40, section_y + 14, format_price(eth.price), Font::font12(), Color::Black, Color::White);
  draw_price_indicator(120, section_y + 14, eth.is_positive_change(), eth.change_24h);
}

void DashboardRenderer::draw_price_indicator(size_t x, size_t y, bool positive, double change) {
  const std::string change_str = format_change(change);

  // Draw arrow indicator
  if (positive) {
    // Up arrow (▲)
    display_.draw_string(x, y, "^", Font::font12(), Color::Black, Color::White);
  } else {
    // Down arrow (▼)
    display_.draw_string(x, y, "v", Font::font12(), Color::Black, Color::White);
  }

  // Draw percentage
  display_.draw_string(x + 10, y, change_str, Font::font12(), Color::Black, Color::White);
}

void DashboardRenderer::render_combined_screen(const CryptoPrice &btc, const CryptoPrice &eth,
                                               const PriceHistory &btc_30d, const PriceHistory &eth_30d) {
  draw_header();
  draw_price_section(btc, eth);
  draw_charts_side_by_side(btc_30d, eth_30d);
}

void DashboardRenderer::render_btc_dedicated_screen(const CryptoPrice &btc, const PriceHistory &btc_30d,
                                                    const PriceHistory &btc_6mo) {
  draw_header();
  draw_price_section_single(btc, "BTC");
  draw_charts_stacked(btc_30d, btc_6mo);
}

void DashboardRenderer::render_eth_dedicated_screen(const CryptoPrice &eth, const PriceHistory &eth_30d,
                                                    const PriceHistory &eth_6mo) {
  draw_header();
  draw_price_section_single(eth, "ETH");
  draw_charts_stacked(eth_30d, eth_6mo);
}

void DashboardRenderer::draw_price_section_single(const CryptoPrice &price, const std::string &label) {
  constexpr size_t section_y = 24;

  display_.draw_string(5, section_y, label, Font::font16(), Color::Black, Color::White);
  display_.draw_string(50, section_y, format_price(price.price), Font::font16(), Color::Black, Color::White);
  draw_price_indicator(180, section_y, price.is_positive_change(), price.change_24h);
}

void DashboardRenderer::draw_charts_side_by_side(const PriceHistory &btc_history, const PriceHistory &eth_history) {
  // Display: 264x176, available space after header (y=20) and prices (y=24-52)
  constexpr size_t chart_y = 52;
  constexpr size_t chart_height = 106; // 176 - 52 - 14 (label) - 4 (margin) = 106
  constexpr size_t chart_width = 120;  // (264 - 5 - 10 - 5) / 2 = 122, use 120 for safety
  constexpr size_t chart_spacing = 12; // 5 (left) + 120 + 12 + 120 + 7 (right) = 264

  // BTC chart (left)
  display_.draw_string(5, chart_y, "BTC 30d", Font::font12(), Color::Black, Color::White);
  if (!btc_history.empty()) {
    draw_line_chart(5, chart_y + 14, chart_width, chart_height, btc_history.prices);
  }

  // ETH chart (right)
  const size_t eth_chart_x = 5 + chart_width + chart_spacing;
  display_.draw_string(eth_chart_x, chart_y, "ETH 30d", Font::font12(), Color::Black, Color::White);
  if (!eth_history.empty()) {
    draw_line_chart(eth_chart_x, chart_y + 14, chart_width, chart_height, eth_history.prices);
  }
}

void DashboardRenderer::draw_charts_stacked(const PriceHistory &top_history, const PriceHistory &bottom_history) {
  // Display: 264x176 (pixels 0-175)
  // Layout: Header (0-20) + Price (24-43) + Charts (44-175)
  // Available for charts: 176 - 44 = 132 pixels
  // Split evenly: 14 (label) + 49 (chart) + 4 (space) + 14 (label) + 49 (chart) + 2 (margin) = 132
  // Both charts are 49 pixels tall for visual symmetry
  // draw_rectangle draws TO (y+height), so y=126 + height=49 draws TO y=175 (last valid pixel)

  constexpr size_t chart_start_y = 44;
  constexpr size_t chart_height = 49; // 49 pixels to stay within 0-175 bounds
  constexpr size_t chart_width = 254; // 264 - 5 (left) - 5 (right) = 254
  constexpr size_t chart_spacing = 4;

  // Top chart (30-day)
  // Label: y=44-57, Chart: y=58-106 (height=49), draws TO y=107
  display_.draw_string(5, chart_start_y, top_history.symbol + " 30d", Font::font12(), Color::Black, Color::White);
  if (!top_history.empty()) {
    draw_line_chart(5, chart_start_y + 14, chart_width, chart_height, top_history.prices);
  }

  // Bottom chart (6-month)
  // Label: y=111-124, Chart: y=126-174 (height=49), draws TO y=175 (safe!)
  const size_t bottom_chart_y = chart_start_y + 14 + chart_height + chart_spacing;
  display_.draw_string(5, bottom_chart_y, bottom_history.symbol + " 6mo", Font::font12(), Color::Black, Color::White);
  if (!bottom_history.empty()) {
    draw_line_chart(5, bottom_chart_y + 14, chart_width, chart_height, bottom_history.prices);
  }
}

void DashboardRenderer::draw_line_chart(size_t x, size_t y, size_t width, size_t height,
                                        const std::vector<double> &data) {
  if (data.empty()) {
    return;
  }

  // Draw border
  display_.draw_rectangle(x, y, x + width, y + height, Color::Black);

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
    display_.draw_line(x, middle_y, x + width, middle_y, Color::Black);
    return;
  }

  // Draw line chart
  for (size_t i = 1; i < data.size(); ++i) {
    const double x_scale = static_cast<double>(width) / static_cast<double>(data.size() - 1);
    const size_t x1 = x + static_cast<size_t>((i - 1) * x_scale);
    const size_t x2 = x + static_cast<size_t>(i * x_scale);

    const size_t y1 = y + height - static_cast<size_t>(((data[i - 1] - min_val) / range) * static_cast<double>(height));
    const size_t y2 = y + height - static_cast<size_t>(((data[i] - min_val) / range) * static_cast<double>(height));

    display_.draw_line(x1, y1, x2, y2, Color::Black);
  }
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
