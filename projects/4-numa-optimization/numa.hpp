#pragma once

#include <vector>
#include <thread>
#include <sched.h>
#include <numaif.h>
#include <numa.h>
#include <cstring>

class NUMAOptimizer {
public:
  static int get_numa_node() {
    return numa_node_of_cpu(sched_getcpu());
  }

  static bool set_thread_affinity(std::thread::native_handle_type handle,
                                   int cpu_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_id, &cpuset);
    return pthread_setaffinity_np(handle, sizeof(cpu_set_t), &cpuset) == 0;
  }

  static void* allocate_local(size_t bytes, int numa_node) {
    void* ptr = nullptr;
    numa_alloc_onnode(ptr, bytes, numa_node);
    return ptr;
  }

  static void deallocate_local(void* ptr, size_t bytes) {
    numa_free(ptr, bytes);
  }
};

template <typename T>
class NUMAArray {
private:
  std::vector<T*> node_buffers;
  std::vector<size_t> node_sizes;
  int num_nodes;
  size_t total_size;

public:
  NUMAArray(size_t size, int nodes = -1)
      : num_nodes(nodes == -1 ? numa_num_nodes() : nodes),
        total_size(size) {
    if (num_nodes <= 0) num_nodes = 1;

    size_t per_node = (size + num_nodes - 1) / num_nodes;
    node_buffers.resize(num_nodes);
    node_sizes.resize(num_nodes, per_node);

    for (int i = 0; i < num_nodes; i++) {
      node_buffers[i] = (T*)numa_alloc_onnode(per_node * sizeof(T), i);
      if (!node_buffers[i]) {
        throw std::runtime_error("Failed to allocate NUMA memory");
      }
    }
  }

  ~NUMAArray() {
    for (int i = 0; i < num_nodes; i++) {
      if (node_buffers[i]) {
        numa_free(node_buffers[i], node_sizes[i] * sizeof(T));
      }
    }
  }

  T* get_node_buffer(int node) { return node_buffers[node]; }

  size_t get_node_size(int node) { return node_sizes[node]; }

  int get_num_nodes() const { return num_nodes; }

  T& at_local(size_t idx) {
    int node = NUMAOptimizer::get_numa_node();
    size_t local_idx = idx % node_sizes[node];
    return node_buffers[node][local_idx];
  }
};
