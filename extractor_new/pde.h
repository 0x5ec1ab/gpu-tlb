#ifndef _PDE_H_
#define _PDE_H_

#include <cstdint>
#include <iostream>
#include <map>
#include <bitset>
#include <vector>
#include "entry.h"

enum PDEType { PD3, PD2, PD1, PD0, PTE0, PAGE512M, PAGE2M, PAGE64K, PAGE4K};
class PDE {
protected:
  uint64_t phy_addr;
  void *base_addr;
  void *entry_addr;
  std::map<int, ENTRY* > pde_entry;

  PDEType type;

  ENTRY self_entry;

public:
  PDE(uint64_t, void*, PDEType, ENTRY);

  ~PDE();
};

#endif
