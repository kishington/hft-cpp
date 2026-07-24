#pragma once

#include <array>
#include <cstring>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <algorithm>

struct Order {
  int id;
  int qty;
  int64_t timestamp;
};

struct Trade {
  int buy_id;
  int sell_id;
  int price_cents;
  int qty;
};

enum class Side { BUY, SELL };

class CacheOptimizedOrderBook {
private:
  static constexpr int MIN_PRICE = 10000;      // 100.00 * 100
  static constexpr int MAX_PRICE = 20000;      // 200.00 * 100
  static constexpr int PRICE_LEVELS = MAX_PRICE - MIN_PRICE + 1;
  static constexpr int MAX_ORDERS_PER_LEVEL = 1000;

  struct OrderLevel {
    std::array<Order, MAX_ORDERS_PER_LEVEL> orders;
    uint16_t count = 0;
  };

  std::array<OrderLevel, PRICE_LEVELS> bids;
  std::array<OrderLevel, PRICE_LEVELS> asks;
  std::unordered_map<int, std::pair<Side, uint16_t>> order_index;
  std::vector<Trade> trade_log;
  int64_t timestamp_counter = 0;

public:
  CacheOptimizedOrderBook() = default;

  void add_order(int id, int price_cents, int qty, Side side);
  void cancel_order(int id);
  std::vector<Trade> match_market_order(int qty, Side side);

  int best_bid() const;
  int best_ask() const;
  int get_bid_size_at(int price_cents) const;
  int get_ask_size_at(int price_cents) const;
  const std::vector<Trade>& get_trades() const { return trade_log; }
  void clear() {
    bids = {};
    asks = {};
    order_index.clear();
    trade_log.clear();
    timestamp_counter = 0;
  }
};
