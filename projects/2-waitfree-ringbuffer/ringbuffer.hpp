#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <cstring>

template <typename T, size_t SIZE = 4096>
class WaitFreeRingBuffer {
  static_assert((SIZE & (SIZE - 1)) == 0, "SIZE must be power of 2");
  static constexpr size_t MASK = SIZE - 1;

public:
  struct alignas(64) WriteState {
    std::atomic<uint64_t> pos{0};
  };

  struct alignas(64) ReadState {
    std::atomic<uint64_t> pos{0};
  };

private:
  struct Slot {
    T data;
  };

  std::array<Slot, SIZE> buffer;
  WriteState write_state;
  ReadState read_state;

public:
  WaitFreeRingBuffer() = default;

  // Single-Producer, Single-Consumer (SPSC) - Wait-free
  bool push_spsc(const T& val) {
    uint64_t write = write_state.pos.load(std::memory_order_relaxed);
    uint64_t read = read_state.pos.load(std::memory_order_acquire);

    // Check if buffer full
    if ((write - read) >= SIZE) {
      return false;  // Ring buffer full
    }

    buffer[write & MASK].data = val;
    write_state.pos.store(write + 1, std::memory_order_release);
    return true;
  }

  bool pop_spsc(T& val) {
    uint64_t read = read_state.pos.load(std::memory_order_relaxed);
    uint64_t write = write_state.pos.load(std::memory_order_acquire);

    if (read >= write) {
      return false;  // Buffer empty
    }

    val = buffer[read & MASK].data;
    read_state.pos.store(read + 1, std::memory_order_release);
    return true;
  }

  // Multi-Producer, Multi-Consumer (MPMC) - Lock-free with CAS
  bool push_mpmc(const T& val) {
    while (true) {
      uint64_t write = write_state.pos.load(std::memory_order_relaxed);
      uint64_t read = read_state.pos.load(std::memory_order_acquire);

      if ((write - read) >= SIZE) {
        return false;  // Full
      }

      if (write_state.pos.compare_exchange_weak(write, write + 1,
                                                 std::memory_order_release,
                                                 std::memory_order_relaxed)) {
        buffer[write & MASK].data = val;
        return true;
      }
    }
  }

  bool pop_mpmc(T& val) {
    while (true) {
      uint64_t read = read_state.pos.load(std::memory_order_relaxed);
      uint64_t write = write_state.pos.load(std::memory_order_acquire);

      if (read >= write) {
        return false;  // Empty
      }

      val = buffer[read & MASK].data;

      if (read_state.pos.compare_exchange_weak(read, read + 1,
                                                std::memory_order_release,
                                                std::memory_order_relaxed)) {
        return true;
      }
    }
  }

  size_t approximate_size() const {
    return write_state.pos.load(std::memory_order_relaxed) -
           read_state.pos.load(std::memory_order_relaxed);
  }

  void reset() {
    write_state.pos.store(0, std::memory_order_relaxed);
    read_state.pos.store(0, std::memory_order_relaxed);
  }
};
