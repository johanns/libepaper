#include "types.hpp"
#include <algorithm>
#include <limits>

namespace crypto_dashboard {

auto PriceHistory::min_price() const -> double {
  if (prices.empty()) {
    return 0.0;
  }
  return *std::ranges::min_element(prices);
}

auto PriceHistory::max_price() const -> double {
  if (prices.empty()) {
    return 0.0;
  }
  return *std::ranges::max_element(prices);
}

} // namespace crypto_dashboard
