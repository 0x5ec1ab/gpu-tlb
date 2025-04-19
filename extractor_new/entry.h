#ifndef _ENTRY_H_
#define _ENTRY_H_
#include <cstdint>

enum pagetype
{
    PAGE_512M = 0,
    PAGE_2M = 1,
    PAGE_64K = 2,
    PAGE_4K = 3,
};
class ENTRY
{
public:
    uint64_t addr;
    uint8_t flags;
    uint8_t V;
    uint8_t A;
    uint64_t index;
    uint64_t entry_bits = 0;
    pagetype small = PAGE_4K;
    uint64_t virt_addr = 0;

    ENTRY();
    ENTRY(uint64_t, uint8_t, uint8_t, uint8_t, uint64_t, uint64_t);
    ENTRY(uint64_t, uint8_t, uint8_t, uint8_t, uint64_t, uint64_t, pagetype);
    ~ENTRY();
};

#endif