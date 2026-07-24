# NUMA Optimization

## Problem

On multi-socket systems (2+ sockets), memory access latency depends on locality:
- **Local access**: ~50-70 ns (L3 cache)
- **Remote access**: ~200-300 ns (remote socket)

This 3-5x difference is critical in HFT. Default OS memory allocation may place data on the wrong socket.

## Solution

**NUMA-aware memory allocation and thread affinity:**
- Allocate memory on the same NUMA node as threads
- Pin threads to specific CPU cores
- Avoid remote memory access in hot paths
- Separate read/write buffers per node to prevent contention

## Architecture

```
System layout (2-socket example):
  Socket 0: CPUs 0-7, RAM 0
  Socket 1: CPUs 8-15, RAM 1
  
Optimized layout:
  Thread pool 0 → CPUs 0-3 → RAM 0 (local)
  Thread pool 1 → CPUs 8-11 → RAM 1 (local)
  
NUMAArray:
  node_buffers[0] → allocated on Socket 0
  node_buffers[1] → allocated on Socket 1
```

## Key Optimizations

1. **NUMA-aware allocation**: `numa_alloc_onnode()`
   - Allocates memory local to specific NUMA node
   - Prevents default allocation on node 0

2. **Thread affinity**: `pthread_setaffinity_np()`
   - Pins thread to specific CPU cores
   - Keeps thread on same socket as its working set

3. **CPU-local access**: `numa_node_of_cpu()`
   - Queries which NUMA node a CPU belongs to
   - Guides memory allocation decisions

4. **Separate buffers per node**: Reduces coherency traffic
   - Each node owns its own memory region
   - No cache line bouncing across sockets

## Operations

| Operation | Cost | Notes |
|-----------|------|-------|
| Local access | 50-70 ns | Direct socket access |
| Remote access | 200-300 ns | Cross-socket latency |
| Allocation on node | O(1) | Guided placement |

## Typical Latencies (Intel Xeon, 2-socket)

```
Local memory read:
  p50:   65 ns
  p99:   120 ns

Remote memory read (to other socket):
  p50:   220 ns
  p99:   450 ns

Cross-socket penalty: ~3.5x
```

## HFT Strategy

1. **Input data → Node 0**: Market feeds thread on Socket 0
2. **Processing → Node 0**: Trading logic thread on Socket 0
3. **Output data → Node 0**: Order submission thread on Socket 0
4. **Monitoring → Node 1**: Risk/monitoring thread on Socket 1 (isolated)

Result: All hot-path operations stay on same socket.

## Compile & Run (Linux only)

```bash
cd projects/4-numa-optimization
make
./test_numa
./bench_numa
```

**Note**: Requires Linux with libnuma and proper BIOS settings (interleaved vs. local allocation).
