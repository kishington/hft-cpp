build:
	mkdir -p build
	cd build && cmake .. && make -j$(nproc)

test:
	cd build && make test

bench:
	cd build && ./projects/1-cache-optimized-orderbook/bench_orderbook
	cd build && ./projects/2-waitfree-ringbuffer/bench_ringbuffer
	cd build && ./projects/3-lockfree-message-queue/bench_queue
	cd build && ./projects/5-latency-timer/bench_timer
	cd build && ./projects/6-cache-optimization/bench_cache

clean:
	rm -rf build

.PHONY: build test bench clean
