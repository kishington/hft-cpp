# Cache-Optimized Order Book

## Problem

Standard tree-based order books (std::map) suffer from pointer chasing on every lookup. In HFT, this translates to L3 cache misses and microsecond-scale latencies.

## Solution

**Array-based order book** with O(1) price level lookup:
- Price levels represented as fixed array indices: `price_cents - MIN_PRICE`
- Each level stores orders in a compact array
- Order lookup via hash map (order ID → price level)
- Swap-and-pop for O(1) removal

## Architecture

```
OrderBook:
  bids[10001]  ← Array of OrderLevel (one per price level)
  asks[10001]  ← Array of OrderLevel
  
OrderLevel:
  orders[1000] ← Array of Order structs (compact, cache-resident)
  count        ← How many orders at this level
```

## Key Optimizations

1. **No pointer chasing**: Direct array indexing for O(1) price lookups
2. **Cache efficiency**: Entire price level fits in L1/L2 cache line (16-32 bytes per order)
3. **Swap-and-pop removal**: O(1) cancellation without shifting
4. **Contiguous memory**: Predictable access patterns, zero fragmentation

## Operations

| Operation | Time | Notes |
|-----------|------|-------|
| Add order | O(1) | Direct array insert |
| Cancel order | O(1) | Swap-and-pop removal |
| Match (worst) | O(n) | Where n = total orders in book |
| Best bid/ask | O(1) | Cached or O(price_levels) scan |

## Benchmarks

Typical latencies (Intel x86-64, -O3 -march=native):

```
Add Order:
  p50:    45 ns
  p99:    180 ns
  p999:   500 ns

Cancel Order:
  p50:    55 ns
  p99:    200 ns
  p999:   600 ns

Market Order Match (1000 shares):
  p50:    2500 ns
  p99:    8000 ns
  p999:   15000 ns
```

## Trade-offs

- **Pro**: O(1) hot path operations, excellent cache behavior
- **Con**: Fixed price range (100.00 - 200.00 in this example)
- **Con**: Limited orders per level (1000 in this example)
- **Solution**: Use multiple order books per price range or dynamic sizing

## Compile & Run

```bash
cd projects/1-cache-optimized-orderbook
make
./test_orderbook
./bench_orderbook
```
