cmake_minimum_required(VERSION 3.20)
project(hft-cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -mtune=native -Wall -Wextra")

# Enable testing
enable_testing()

# Add subdirectories
add_subdirectory(projects/1-cache-optimized-orderbook)
add_subdirectory(projects/2-waitfree-ringbuffer)
add_subdirectory(projects/3-lockfree-message-queue)
add_subdirectory(projects/4-numa-optimization)
add_subdirectory(projects/5-latency-timer)
add_subdirectory(projects/6-cache-optimization)
add_subdirectory(projects/7-event-loop)
