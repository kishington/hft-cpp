# Cache Optimization

## Problem

Memory access latency depends entirely on cache hierarchy:
- **L1 cache hit**: 4-5 ns (~1-2 cycles)
- **L2 cache hit**: 12-15 ns (~4-5 cycles)
- **L3 cache hit**: 40-75 ns (~15-25 cycles)
- **RAM access**: 50-200 ns (NUMA-dependent)

A single L3 miss costs 15-20x more than an L1 hit. In HFT, this difference matters.

## Solution

**Cache-aware data layout and access patterns:**
1. Align data structures to cache line boundaries (64 bytes)
2. Use sequential access patterns (predictable prefetching)
3. Software prefetching for known access patterns
4. Fit working set in L1/L2 cache when possible
5. Minimize cache line bouncing between threads

## Architecture

```
Cache Hierarchy (Intel Skylake per core):
  L1 Instruction: 32 KB (4-cycle latency)
  L1 Data:        32 KB (4-cycle latency)
  L2 Unified:     256 KB (12-cycle latency)
  L3 Shared:      8 MB per core (40-cycle latency)
  RAM:            All memory (100-200 cycles)
  
  Cache line size: 64 bytes
  Prefetch distance: 8-16 cache lines ahead
```

## Key Optimizations

1. **Cache line alignment**: Align structures to 64 bytes
   - Ensures one logical unit per cache line
   - Prevents false sharing between threads
   - Predictable cache behavior

2. **Sequential access**: Stride-1 access patterns
   - Hardware prefetcher detects and prefetches next lines
   - Minimal cache misses (10-20%)
   - ~1-2 ns per element overhead

3. **Software prefetching**: `__builtin_prefetch()`
   - Hint CPU to load data in advance
   - Reduces stalls on cache misses
   - Effective for known irregular patterns
   - ~8-16 elements ahead recommended

4. **Working set sizing**: Fit data in cache
   - L1: 64 KB total (32 KB data)
   - L2: 256 KB per core
   - L3: 8-20 MB shared
   - RAM: Unlimited but slow

5. **Strided access avoidance**: Large strides cause misses
   - Stride > 64 bytes: L1 miss
   - Stride > 4 KB: L2 miss
   - Stride > 8 MB: L3 miss

## Operations

| Pattern | Latency | Throughput | Notes |
|---------|---------|------------|-------|
| L1 sequential | 1.5 ns/elem | 20+ GB/s | Optimal |
| L2 sequential | 4 ns/elem | 8 GB/s | Good |
| L3 sequential | 8-10 ns/elem | 2 GB/s | Acceptable |
| RAM sequential | 20-30 ns/elem | 200-400 MB/s | Slow |
| Random access | 40-100 ns/elem | 100-200 MB/s | Very slow |

## Typical Benchmarks (Intel x86-64, Skylake)

```
Sequential (L1-bound, 4 KB array):
  0.8 ns/elem, 50 GB/s

Sequential (L2-bound, 128 KB array):
  3.2 ns/elem, 12 GB/s

Sequential (L3-bound, 8 MB array):
  8.5 ns/elem, 4.7 GB/s

Strided (stride=1024, RAM-bound):
  25 ns/elem, 160 MB/s

Random access (RAM-bound):
  80 ns/elem, 50 MB/s
```

## HFT Cache Strategy

1. **Order book**: Keep recent price levels in L1 (64 orders × 8 bytes = 512 B)
2. **Position tracking**: Array of positions in L2 (256 KB = 32k positions)
3. **Historical data**: Compressed log in L3 (8 MB = 1M ticks)
4. **Archive**: Full history on disk/SSD

Result: Hot data stays in L1, warm in L2, cold in L3.

## Compile & Run

```bash
cd projects/6-cache-optimization
make
./test_cache
./bench_cache
```

**Recommended compiler flags**:
```bash
g++ -O3 -march=native -fno-tree-vectorize bench_cache.cpp -o bench_cache
```
