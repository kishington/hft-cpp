#include "orderbook.hpp"
#include <cassert>
#include <iostream>

void test_add_and_cancel() {
  CacheOptimizedOrderBook book;
  book.add_order(1, 10050, 100, Side::BUY);
  book.add_order(2, 10060, 50, Side::SELL);

  assert(book.best_bid() == 10050);
  assert(book.best_ask() == 10060);

  book.cancel_order(1);
  assert(book.best_bid() == -1);
  std::cout << "✓ Add and cancel test passed\n";
}

void test_market_order_matching() {
  CacheOptimizedOrderBook book;
  book.add_order(1, 10050, 100, Side::BUY);
  book.add_order(2, 10050, 50, Side::BUY);
  book.add_order(3, 10060, 75, Side::SELL);

  auto trades = book.match_market_order(120, Side::BUY);
  assert(trades.size() >= 1);
  int total_matched = 0;
  for (const auto& t : trades) {
    total_matched += t.qty;
  }
  assert(total_matched == 75);
  std::cout << "✓ Market order matching test passed\n";
}

void test_partial_fills() {
  CacheOptimizedOrderBook book;
  book.add_order(1, 10050, 100, Side::SELL);
  book.add_order(2, 10050, 50, Side::SELL);

  auto trades = book.match_market_order(75, Side::BUY);
  assert(trades.size() >= 1);
  assert(trades[0].qty == 75);
  
  // Remaining 25 shares should still be at 10050
  assert(book.get_ask_size_at(10050) == 75);
  std::cout << "✓ Partial fills test passed\n";
}

void test_best_bid_ask() {
  CacheOptimizedOrderBook book;
  book.add_order(1, 10040, 100, Side::BUY);
  book.add_order(2, 10050, 50, Side::BUY);
  book.add_order(3, 10060, 100, Side::SELL);
  book.add_order(4, 10070, 50, Side::SELL);

  assert(book.best_bid() == 10050);
  assert(book.best_ask() == 10060);
  std::cout << "✓ Best bid/ask test passed\n";
}

int main() {
  test_add_and_cancel();
  test_market_order_matching();
  test_partial_fills();
  test_best_bid_ask();
  std::cout << "\nAll tests passed!\n";
  return 0;
}
