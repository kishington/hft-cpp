#include "orderbook.hpp"
#include <algorithm>

void CacheOptimizedOrderBook::add_order(int id, int price_cents, int qty, Side side) {
  if (price_cents < MIN_PRICE || price_cents > MAX_PRICE) {
    return;  // Out of range
  }

  int idx = price_cents - MIN_PRICE;
  auto& level = (side == Side::BUY) ? bids[idx] : asks[idx];

  if (level.count >= MAX_ORDERS_PER_LEVEL) {
    return;  // Level full
  }

  level.orders[level.count] = {id, qty, timestamp_counter++};
  level.count++;
  order_index[id] = {side, (uint16_t)idx};
}

void CacheOptimizedOrderBook::cancel_order(int id) {
  auto it = order_index.find(id);
  if (it == order_index.end()) {
    return;  // Order not found
  }

  auto [side, price_idx] = it->second;
  auto& level = (side == Side::BUY) ? bids[price_idx] : asks[price_idx];

  // Find and remove using swap-and-pop
  for (uint16_t i = 0; i < level.count; i++) {
    if (level.orders[i].id == id) {
      std::swap(level.orders[i], level.orders[--level.count]);
      break;
    }
  }

  order_index.erase(it);
}

std::vector<Trade> CacheOptimizedOrderBook::match_market_order(int qty, Side side) {
  std::vector<Trade> trades;
  auto& opposite_side = (side == Side::BUY) ? asks : bids;
  int start_idx = (side == Side::BUY) ? 0 : PRICE_LEVELS - 1;
  int direction = (side == Side::BUY) ? 1 : -1;

  for (int i = start_idx; qty > 0 && i >= 0 && i < PRICE_LEVELS; i += direction) {
    auto& level = opposite_side[i];
    while (qty > 0 && level.count > 0) {
      auto& order = level.orders[0];
      int match_qty = std::min(qty, order.qty);
      trades.push_back({side == Side::BUY ? -1 : order.id,
                        side == Side::BUY ? order.id : -1,
                        i + MIN_PRICE, match_qty});
      qty -= match_qty;

      if (match_qty == order.qty) {
        std::swap(level.orders[0], level.orders[--level.count]);
        order_index.erase(order.id);
      } else {
        order.qty -= match_qty;
      }
    }
  }

  trade_log.insert(trade_log.end(), trades.begin(), trades.end());
  return trades;
}

int CacheOptimizedOrderBook::best_bid() const {
  for (int i = PRICE_LEVELS - 1; i >= 0; i--) {
    if (bids[i].count > 0) {
      return i + MIN_PRICE;
    }
  }
  return -1;
}

int CacheOptimizedOrderBook::best_ask() const {
  for (int i = 0; i < PRICE_LEVELS; i++) {
    if (asks[i].count > 0) {
      return i + MIN_PRICE;
    }
  }
  return -1;
}

int CacheOptimizedOrderBook::get_bid_size_at(int price_cents) const {
  if (price_cents < MIN_PRICE || price_cents > MAX_PRICE) return 0;
  int idx = price_cents - MIN_PRICE;
  int total = 0;
  for (uint16_t i = 0; i < bids[idx].count; i++) {
    total += bids[idx].orders[i].qty;
  }
  return total;
}

int CacheOptimizedOrderBook::get_ask_size_at(int price_cents) const {
  if (price_cents < MIN_PRICE || price_cents > MAX_PRICE) return 0;
  int idx = price_cents - MIN_PRICE;
  int total = 0;
  for (uint16_t i = 0; i < asks[idx].count; i++) {
    total += asks[idx].orders[i].qty;
  }
  return total;
}
