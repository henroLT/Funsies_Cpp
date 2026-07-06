#include <cstdint>

struct Tick {
    uint64_t    ts_ns;
    double      price;
    uint32_t    quant;
    uint32_t    symbol;
};
static_assert(std::is_trivially_copyable_v<Tick>);
