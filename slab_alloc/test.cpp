#include "Order.h"
#include "Pool.h"

#include <cassert>
#include <random>
#include <vector>

constexpr int CAP = 100;

struct Counted {
    static int alive;
    unsigned char* padding[8];

    Counted() { ++alive; }
    ~Counted() { --alive; }
};

int Counted::alive = 0;

void test_capacity() {
    ObjectPool<Order, CAP> pool;

    std::vector<Order*> ptrs;

    for (int i = 0; i < CAP; ++i) {
        Order* p = pool.acquire();
        assert(p != nullptr);
        ptrs.push_back(p);
    }

    assert(pool.acquire() == nullptr);

    for (Order* p : ptrs)
        pool.release(p);

    ptrs.clear();

    for (int i = 0; i < CAP; ++i) {
        Order* p = pool.acquire();
        assert(p != nullptr);
        ptrs.push_back(p);
    }

    assert(pool.acquire() == nullptr);
}

void test_lifo() {
    ObjectPool<Order, CAP> pool;

    Order* a = pool.acquire();
    Order* b = pool.acquire();
    Order* c = pool.acquire();

    pool.release(a);
    pool.release(b);
    pool.release(c);

    assert(pool.acquire() == c);
    assert(pool.acquire() == b);
    assert(pool.acquire() == a);
}

void test_capacity_one() {
    ObjectPool<Order, 1> pool;

    Order* p = pool.acquire();
    assert(p != nullptr);

    assert(pool.acquire() == nullptr);

    pool.release(p);

    assert(pool.acquire() == p);
}

void test_interleaved() {
    ObjectPool<Order, CAP> pool;

    Order* a = pool.acquire();
    Order* b = pool.acquire();

    pool.release(a);

    Order* c = pool.acquire();
    assert(c == a);

    pool.release(b);
    pool.release(c);

    assert(pool.acquire() != nullptr);
    assert(pool.acquire() != nullptr);
}

void test_stress() {
    ObjectPool<Order, CAP> pool;

    std::vector<Order*> allocated;
    std::mt19937 rng(123);

    for (int i = 0; i < 100000; ++i) {
        if (allocated.empty() || (rng() & 1)) {
            if (Order* p = pool.acquire())
                allocated.push_back(p);
        } else {
            std::size_t idx = rng() % allocated.size();
            pool.release(allocated[idx]);
            allocated.erase(allocated.begin() + idx);
        }
    }

    for (Order* p : allocated)
        pool.release(p);
}

void test_ctor_dtor() {
    ObjectPool<Counted, 10> pool;

    Counted* a = pool.acquire();
    Counted* b = pool.acquire();

    assert(Counted::alive == 2);

    pool.release(a);
    assert(Counted::alive == 1);

    pool.release(b);
    assert(Counted::alive == 0);
}

int main() {
    test_capacity();
    test_lifo();
    test_capacity_one();
    test_interleaved();
    test_stress();
    test_ctor_dtor();

    return 0;
}