# Branch Prediction & CPU Optimization

## Problem

Modern CPUs execute instructions speculatively (out-of-order). A branch prediction miss stalls the pipeline:
- **Prediction hit**: ~0.5 ns (continues execution)
- **Prediction miss**: 10-20 ns (pipeline flush)

In tight loops processing market data, branch misses can devastate throughput.

## Solution

**Branch elimination techniques:**
1. Branchless arithmetic (conditional moves)
2. Lookup tables (LUT) to replace conditionals
3. Vectorization to expose parallelism
4. Data layout optimization (AoS vs SoA)
5. Compiler hints (`__builtin_expect`)

## Architecture

```
Branch Prediction penalty:
  Modern CPU pipeline: 12-20 stages
  Misprediction flushes pipeline: 10-20 cycles lost
  
Branchless computation:
  All instructions executed (no speculation)
  No pipeline flushes
  Conditional moves (cmov): ~1 cycle latency
```

## Key Optimizations

1. **Branchless conditionals**: Use arithmetic instead of if/else
   ```cpp
   // Branch (unpredictable on random data)
   if (x > threshold) sum += x;
   
   // Branchless (no pipeline stall)
   sum += x * (x > threshold ? 1 : 0);
   ```

2. **Lookup tables**: Pre-compute all results
   ```cpp
   lut[256];  // Pre-compute condition results
   sum += lut[value];  // O(1) table lookup
   ```

3. **Compiler hints**: Guide predictor
   ```cpp
   if (__builtin_expect(likely_condition, 1)) { ... }
   ```

4. **Data layout**: 
   - **AoS**: Array of Structures (poor cache, unpredictable)
   - **SoA**: Structure of Arrays (better cache, vectorizable)

5. **Vectorization**: Process multiple elements per iteration
   ```cpp
   // 4 elements at once = fewer mispredictions per element
   for (i += 4) {
     sum += f(a[i]) + f(a[i+1]) + f(a[i+2]) + f(a[i+3]);
   }
   ```

## Operations

| Technique | Latency | Throughput | Notes |
|-----------|---------|------------|-------|
| Branching (predictable) | 0.5 ns/elem | 10+ GB/s | If trained |
| Branching (random) | 8-15 ns/elem | 500 MB/s | Pipeline stalls |
| Branchless | 1-2 ns/elem | 5+ GB/s | Conditional move |
| Lookup table | 0.5 ns/elem | 10+ GB/s | L1 cache hit |
| Vectorized | 0.2 ns/elem | 50+ GB/s | 4-way parallelism |

## Typical Benchmarks (Intel x86-64, Skylake)

```
Sorted data (predictable branch):
  if/else:     1.2 ns/elem   (branch predictor trained)
  Branchless:  2.1 ns/elem   (cmov overhead)
  
Random data (unpredictable branch):
  if/else:    12.0 ns/elem   (pipeline flushes)
  Branchless:  2.1 ns/elem   (no stalls)
  Lookup tab:  0.8 ns/elem   (L1 cache)
  
Vectorized (4-way):
  Scalar:      2.1 ns/elem
  Vectorized:  0.6 ns/elem   (4x better)
```

## HFT Application

Market data processing has unpredictable branches:
- Price up? Bid higher
- Inventory high? Send less
- Risk limit exceeded? Stop trading

Solutions:
1. **Lookup table**: Pre-compute decision for all (price, inventory, risk) combinations
2. **Branchless math**: Use arithmetic for order sizing
3. **Vectorization**: Process multiple symbols in parallel
4. **SoA layout**: Keep decision data sequential for prefetching

Result: Consistent sub-microsecond latency regardless of data patterns.

## Compile & Run

```bash
cd projects/7-branch-prediction
make
./test_branch
./bench_branch
```

**Recommended flags**:
```bash
g++ -O3 -march=native -fno-tree-vectorize bench_branch.cpp -o bench_branch
# -fno-tree-vectorize: disable auto-vectorization to measure scalar baseline
```
