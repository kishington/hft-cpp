# Lock-Free Message Queue

## Problem

Market data feeds need to dispatch messages to multiple trading threads:
- Sub-microsecond latency
- Lock-free (no mutexes, no blocking)
- Pre-allocated (no malloc/free in hot path)
- Bounded memory footprint

## Solution

**Lock-free linked list with pre-allocated node pool:**
- Nodes allocated at startup from fixed pool
- Enqueue/dequeue use compare-and-swap (CAS) atomics
- FIFO ordering maintained by head/tail pointers
- Zero allocation/deallocation in hot path

## Architecture

```
LockFreeMessageQueue:
  node_pool[POOL_SIZE]  ← Pre-allocated nodes (no malloc)
  free_list             ← Atomic stack of available nodes
  head                  ← Atomic queue head (cache-line aligned)
  tail                  ← Atomic queue tail (cache-line aligned)
  
  Node:
    timestamp_ns        ← Ingestion timestamp
    message[MESSAGE_SIZE]  ← Fixed-size payload
    next (atomic)       ← Link to next node
```

## Key Optimizations

1. **Pre-allocated pool**: All memory allocated in constructor
   - No malloc/free during runtime
   - Prevents GC pauses and fragmentation
   - Bounded memory usage

2. **Lock-free CAS**: Compare-and-swap for atomicity
   - No mutexes, no blocking
   - Minimal contention between producers/consumers

3. **Separate head/tail**: Independent pointers
   - Each on separate cache line (64 bytes padding)
   - Prevents false sharing
   - Producers and consumers don't interfere

4. **Memory ordering**: `acquire/release` semantics
   - CAS provides full synchronization
   - Relaxed loads where safe

5. **FIFO ordering**: Strict ordering guarantees
   - Messages processed in ingestion order
   - Important for market data consistency

## Operations

| Operation | Time | Notes |
|-----------|------|-------|
| enqueue | O(1) lock-free | Allocates from free list with CAS |
| dequeue | O(1) lock-free | Links head pointer with CAS |
| enqueue (when full) | fail | Returns false, producer retries |
| dequeue (when empty) | fail | Returns false, consumer spins |

## Benchmarks (Typical - Intel x86-64, -O3)

```
Single-threaded:
  Enqueue p50:   250 ns
  Dequeue p50:   280 ns
  Throughput:    ~3M msgs/sec

Concurrent (2 producers, 2 consumers):
  Enqueue p50:   800 ns  (CAS contention)
  Dequeue p50:   850 ns
  Throughput:    ~1.2M msgs/sec
```

## Trade-offs

- **Pro**: Lock-free, no blocking
- **Pro**: Pre-allocated, zero GC latency
- **Pro**: FIFO ordering guarantees
- **Con**: Fixed pool size (bounded by startup memory)
- **Con**: CAS retry loops under contention
- **Con**: Fixed message size

## Typical HFT Usage

```cpp
// Market data feed
std::thread feed([&]() {
  while (true) {
    MarketData tick = receive_from_exchange();
    uint8_t msg[256];
    serialize_tick(msg, tick);
    
    while (!q.enqueue(msg, 256, get_time_ns())) {
      // Retry if queue full (rare in normal operation)
    }
  }
});

// Trading thread
std::thread trader([&]() {
  while (true) {
    uint8_t msg[256];
    size_t len;
    int64_t ts;
    
    if (q.dequeue(msg, len, ts)) {
      MarketData tick = deserialize_tick(msg);
      int64_t latency_ns = get_time_ns() - ts;
      process_market_data(tick, latency_ns);
    }
  }
});
```

## Compile & Run

```bash
cd projects/3-lockfree-message-queue
make
./test_queue
./bench_queue
```
