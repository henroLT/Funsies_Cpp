#include <atomic>
#include <cstddef>
#include <optional>


template <class T, std::size_t Capacity>
class SpscRingBuffer {
    static_assert((Capacity & (Capacity - 1)) == 0, "Cap must be power of 2");

public:
    SpscRingBuffer() : head_(0), tail_(0) {}
    
    bool try_push(const T& item) {
        std::size_t tail = tail_.load(std::memory_order_relaxed);
        std::size_t next_tail = (tail + 1) & (Capacity - 1);

        std::size_t head = head_.load(std::memory_order_acquire);
        if (next_tail == head)
            return false;
        
        buffer_[tail] = item;
        tail_.store(next_tail, std::memory_order_release);
        return true;
    }

    std::optional<T> try_pop() {
        std::size_t head = head_.load(std::memory_order_relaxed);
        std::size_t tail = tail_.load(std::memory_order_acquire);

        if (head == tail)
            return std::nullopt;

        T value = buffer_[head];
        head_.store((head + 1) & (Capacity - 1), std::memory_order_release);

        return value;
    }


private:
    alignas(64) std::atomic<std::size_t> head_;
    alignas(64) std::atomic<std::size_t> tail_;

    alignas(64) T buffer_[Capacity];
};
