#include <thread>
#include <vector>
#include <cassert>
#include <cstdio>

#include "SpscRingBuffer.h"
#include "Tick.h"

constexpr size_t N = 5'000'000;

int main() {
    SpscRingBuffer<Tick, 1024> ring;
    std::atomic<bool> done{false};

    std::thread producer([&] {
        for (uint64_t i = 0; i < N; ++i) {
            Tick t{i, 100.0 + (i % 50), 100, 1};
            while (!ring.try_push(t)) {}
        }
        done.store(true, std::memory_order_release);
    });

    std::thread consumer([&] {
        uint64_t expected = 0;
        while (expected < N) {
            if (auto t = ring.try_pop()) {
                if (t->ts_ns != expected) {
                    fprintf(stderr, "MISMATCH: expected %llu got %llu\n",
                            (unsigned long long)expected, (unsigned long long)t->ts_ns);
                    std::abort();
                }
                ++expected;
            }
        }
    });

    producer.join();
    consumer.join();
    printf("OK: %zu ticks verified, no drops/corruption\n", N);
    return 0;
}