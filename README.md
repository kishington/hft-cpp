# HFT C++ Technical Interview Projects

Core infrastructure for high-frequency trading systems. **Pure technical focus** — no business logic.

## Projects

1. **Cache-Optimized Order Book** — Array-based, O(1) operations, no pointer chasing
2. **Wait-Free Ring Buffer** — SPSC/MPMC lock-free queue, pre-allocated
3. **Lock-Free Message Queue** — Sub-microsecond latency, atomic operations
4. **NUMA Optimization** — CPU affinity, memory locality
5. **Sub-Microsecond Latency Timer** — rdtsc-based, nanosecond precision
6. **Cache Optimization** — Memory layout analysis, L1/L2 efficiency
7. **Busy-Spin Event Loop** — No context switches, sustained latency

## Build

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Run All Tests

```bash
cd build && make test
```

## Run Benchmarks

```bash
cd build
./projects/1-cache-optimized-orderbook/bench_orderbook
./projects/2-waitfree-ringbuffer/bench_ringbuffer
./projects/3-lockfree-message-queue/bench_queue
./projects/5-latency-timer/bench_timer
./projects/6-cache-optimization/bench_cache
```
