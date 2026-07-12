#include <cstddef>
#include <new>
#include <utility>
#include <cstring>
#include <type_traits>

template <typename T, std::size_t Capacity>
class ObjectPool {
    static_assert(sizeof(T) >= sizeof(void*),
        "TODO: figure out why this assert exists — what would break without it?");
    // can fit ptr to next free

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

    // Construct a T in a free slot using the given constructor args.
    // Returns nullptr if pool is exhausted.
    template <typename... Args>
    T* acquire(Args&&... args) {
        if (!free_list_head_)
            return nullptr;

        void* block = free_list_head_;
        free_list_head_ = *reinterpret_cast<void**>(block);

        return new(block) T(std::forward<Args>(args)...);
    }

    // Return a slot to the pool. Caller must not use ptr after this.
    void release(T* ptr) {
        ptr->~T();
       
        *reinterpret_cast<void**>(ptr) = free_list_head_;
        free_list_head_ = ptr;
    }

private:
    void advanve();
    // TODO: pick your storage representation. Options:
    //   (a) alignas(alignof(T)) unsigned char storage_[Capacity][sizeof(T)];
    //   (b) alignas(alignof(T)) std::byte storage_[Capacity * sizeof(T)];
    // pick one, one sentence why it doesn't matter much here / does matter

    // Prefer byte to prevent accidental arithmetic, and adheres to aliasing rules (so uchar)
    // Also prevents waste
    alignas(alignof(T)) std::byte storage_[Capacity * sizeof(T)];
    void* free_list_head_ = nullptr;
};