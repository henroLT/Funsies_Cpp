#include <cstdint>

struct Order {
    uint64_t order_id;
    uint64_t ts_ns;
    double   price;
    uint32_t qty;
    uint32_t symbol_id;
    uint8_t  side;      // 0 = buy, 1 = sell
};