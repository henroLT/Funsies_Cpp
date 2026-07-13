#include <cstddef>
#include <new>
#include <utility>
#include <cstring>
#include <type_traits>

template <typename T, std::size_t Capacity>
class ObjectPool {
    static_assert(sizeof(T) >= sizeof(void*), "need enough room for ptr to next")

public:
    ObjectPool() {
        for (std::size_t i = 0; i < Capacity - 1; ++i) {
            auto* current = storage_ + i * sizeof(T);
            auto* next = storage_ + (i + 1) * sizeof(T);

            memcpy(current, &next, sizeof(next));
        }

        void* null = nullptr;
        memcpy(storage_ + (Capacity - 1) * sizeof(T), &null, sizeof(null));
        free_list_head_ = storage_;
    }

    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;

    // Construct a T in a free slot using the given constructor args.
    // Returns nullptr if pool is exhausted.
    template <typename... Args>
    T* acquire(Args&&... args) {
        if (!free_list_head_)
            return nullptr;

        void* block = free_list_head_;
        memcpy(&free_list_head_, block, sizeof(void*));
        return new(block) T(std::forward<Args>(args)...);
    }

    // Return a slot to the pool. Caller must not use ptr after this.
    void release(T* ptr) {
        ptr->~T(); // destructor cleans up innards, delete frees heap memory (this is not)
       
        memcpy(ptr, &free_list_head_, sizeof(void*));
        free_list_head_ = ptr;
    }

private:
    alignas(alignof(T)) std::byte storage_[Capacity * sizeof(T)];
    void* free_list_head_ = nullptr;
};