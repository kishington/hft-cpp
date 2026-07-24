# Wait-Free Ring Buffer

## Problem

HFT systems need to pass market data between threads with:
- Minimal latency (sub-microsecond)
- No blocking (wait-free or lock-free)
- High throughput (millions of messages/sec)
- No heap allocations in hot path

Mutex-based queues incur context switches and unpredictable latency.

## Solution

**Circular pre-allocated buffer** with atomic operations:
- SPSC (Single-Producer, Single-Consumer): wait-free, no CAS loops
- MPMC (Multi-Producer, Multi-Consumer): lock-free with compare-and-swap
- Power-of-2 size for bitmask operations (no division)
- Cache-line aligned head/tail (64 bytes padding) to prevent false sharing

## Architecture

```
WaitFreeRingBuffer:
  buffer[SIZE]       ← Pre-allocated slot array
  write_state (64)   ← Atomic write position (cache-line aligned)
  read_state (64)    ← Atomic read position (cache-line aligned)
  
  Index = position & (SIZE - 1)  ← O(1) modulo via bitmask
```

## Key Optimizations

1. **No malloc/free in hot path**: Pre-allocated at construction
2. **Wait-free SPSC**: No retry loops, guaranteed progress
3. **Lock-free MPMC**: CAS-based coordination, no mutexes
4. **Cache-line alignment**: Separate read/write state on different cache lines
   - Prevents false sharing between threads
   - Enables independent read/write scaling
5. **Power-of-2 sizing**: Bitmask `&` instead of slow modulo `%`
6. **Memory ordering**: `acquire/release` semantics for correct synchronization

## Operations

| Operation | SPSC Time | MPMC Time | Notes |
|-----------|-----------|-----------|-------|
| push | O(1) wait-free | O(1) lock-free | May fail if full |
| pop | O(1) wait-free | O(1) lock-free | May fail if empty |

## Memory Ordering

```cpp
// SPSC push: no synchronization needed for single writer
write_pos.store(write + 1, memory_order_release);

// SPSC pop: only read reader's position
read = read_pos.load(memory_order_acquire);

// MPMC: CAS provides full acquire/release semantics
compare_exchange_weak(old, new, memory_order_release, memory_order_relaxed)
```

## Benchmarks (Typical - Intel x86-64, -O3)

```
SPSC:
  Push p50:   28 ns  (wait-free, no retries)
  Pop p50:    32 ns
  Throughput: ~30M msgs/sec

MPMC (2 threads each side):
  Push p50:   185 ns  (CAS retry loops)
  Pop p50:    195 ns
  Throughput: ~5M msgs/sec
```

## Trade-offs

- **Pro**: Wait-free SPSC (best latency)
- **Pro**: Pre-allocated, no GC pauses
- **Pro**: Lock-free MPMC with minimal contention
- **Con**: Fixed buffer size
- **Con**: May lose data if full (producer's responsibility to handle)

## Compile & Run

```bash
cd projects/2-waitfree-ringbuffer
make
./test_ringbuffer
./bench_ringbuffer
```
