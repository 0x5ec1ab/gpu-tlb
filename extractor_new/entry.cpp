
#include "entry.h"
ENTRY::ENTRY(): addr(0), flags(0), V(0), A(0), index(0), entry_bits(0) {}
ENTRY::ENTRY(uint64_t addr, uint8_t flags, uint8_t V, uint8_t A, uint64_t index, uint64_t entry_bits): addr(addr), flags(flags), V(V), A(A), index(index), entry_bits(entry_bits) {}
ENTRY::ENTRY(uint64_t addr, uint8_t flags, uint8_t V, uint8_t A, uint64_t index, uint64_t entry_bits, pagetype small): addr(addr), flags(flags), V(V), A(A), index(index), entry_bits(entry_bits), small(small) {}
ENTRY::~ENTRY() {}
