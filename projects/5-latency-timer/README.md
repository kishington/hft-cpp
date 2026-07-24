# Sub-Microsecond Latency Timer

## Problem

Measuring HFT latencies requires nanosecond precision:
- `std::chrono` overhead: 50-200 ns per measurement
- System clock jitter: microseconds
- Need: sub-100ns overhead, sub-microsecond accuracy

## Solution

**rdtsc (Read Time Stamp Counter)** - CPU cycle counter:
- Direct hardware access via x86-64 instruction
- ~10 ns overhead per measurement
- Nanosecond-scale precision after calibration
- Deterministic ordering via `rdtscp` (serializing)

## Architecture

```
LatencyTimer:
  rdtsc()  ← Read CPU cycle counter (10 ns overhead)
  rdtscp() ← Same but with serialization (guarantees ordering)
  
  Calibration:
    1. Measure wall-clock time (std::chrono)
    2. Measure TSC difference over same interval
    3. Calculate: ns/cycle = wall_ns / tsc_cycles
    4. After: latency_ns = (tsc_delta) * (ns_per_cycle)
```

## Key Optimizations

1. **rdtscp instruction**: Prevents out-of-order measurement
   ```asm
   rdtscp    # Read TSC and CPU ID, serialize
   ```

2. **One-time calibration**: Avoids repeated overhead
   - Run once at startup: measure for 100ms
   - Calculate CPU frequency
   - Cached for all subsequent measurements

3. **Inline assembly**: Zero overhead in hot path
   - `inline` keyword ensures no function call
   - Direct register allocation
   - ~10 ns total overhead per start/stop pair

4. **No allocation**: Stateless measurements
   - No malloc/free
   - No syscalls
   - Runs in userspace

## Operations

| Operation | Latency | Notes |
|-----------|---------|-------|
| start() | ~10 ns | Reads TSC with rdtscp |
| stop() | ~10 ns | Reads TSC with rdtscp |
| latency_ns() | ~1 ns | Arithmetic only |
| Calibration | 100 ms | One-time at startup |

## Typical Measurements (Intel x86-64, Skylake)

```
Rdtscp overhead: ~10 ns per call

Operation latencies measured:
  NOP:              2-4 ns
  L1 cache read:    4-5 ns
  L2 cache read:    12-15 ns
  L3 cache read:    40-75 ns
  RAM read:         100-200 ns
  Function call:    8-12 ns
  Atomic load:      4-7 ns
  CAS (no contention): 8-15 ns
  Memcpy (256B):    500-800 ns
```

## Accuracy Considerations

1. **Turbo Boost**: Frequency varies, affecting ns/cycle
   - Solution: Disable turbo for consistent measurements
   - Or: Recalibrate per measurement

2. **CPU Frequency Scaling**: P-states change dynamically
   - Solution: Run measurements at fixed frequency
   - Use `cpupower` to set governor to `performance`

3. **Context switches**: Preemption ruins measurements
   - Solution: Pin thread to CPU core
   - Disable interrupts in extreme cases

4. **TSC sync**: Multi-socket systems may have TSC skew
   - Solution: Measure on same socket
   - Use `rdtscp` to ensure proper synchronization

## HFT Usage Pattern

```cpp
LatencyTimer timer;

while (true) {
  MarketData tick = receive_from_exchange();
  int64_t ingestion_time = get_current_ns();  // Wall clock
  
  timer.start();  // ~10 ns
  
  // Trading logic (usually 100-1000 ns)
  Order order = calculate_order(tick);
  
  timer.stop();   // ~10 ns
  
  int64_t decision_latency = timer.latency_ns();
  send_order(order);
  
  // Log: 150 ns decision latency
  report_latency(decision_latency);
}
```

## Compile & Run

```bash
cd projects/5-latency-timer
make
./test_timer
./bench_timer
```

**Platform Requirements**:
- x86-64 CPU (Intel/AMD)
- Linux (for `rdtsc` access)
- gcc/clang with inline asm support

**Recommended kernel settings**:
```bash
# Disable turbo boost
echo 1 > /sys/devices/system/cpu/intel_pstate/no_turbo

# Set scaling governor to performance
echo performance | tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor

# Disable hyperthreading (optional, for consistency)
# Edit /etc/default/grub: add 'noht' to GRUB_CMDLINE_LINUX
```
