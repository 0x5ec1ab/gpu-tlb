#include "pde.h"


PDE::PDE(uint64_t phy_addr, void *base_addr, PDEType type,
         ENTRY self_entry) {
  this->phy_addr = phy_addr;
  this->type = type;
  this->self_entry = self_entry;
  this->base_addr = base_addr;
  this->entry_addr = (uint8_t *)this->base_addr + this->phy_addr;
}

PDE::~PDE() {
  for (auto it = this->pde_entry.begin(); it != this->pde_entry.end(); it++) {
    delete it->second;
  }
  this->pde_entry.clear();
}

// void PDE::printPDE() {
//   std::cout << "PDE: 0x" << this->phy_addr << "  type:" << this->type
//             << " entry:" << this->self_entry << std::endl;
//   for (auto it = this->pde_entry.begin(); it != this->pde_entry.end(); it++) {
//     std::cout << it->first << " "
//               << "A: " << (uint64_t)it->second->A;
//     std::cout << " V: " << (uint64_t)it->second->V << " ";
//     std::cout << "flags: " << std::bitset<8>(it->second->flags) << " ";
//     std::cout << "next_phy_addr: 0x" << std::hex << it->second->addr
//               << std::endl;
//   }
//   std::cout << std::endl;
// }

// bool PDE::constructPDE() {
//   for (int i = 0; i < 4; i++) {
//     uint8_t *entry_Ptr = (uint8_t *)this->entry_addr + i * 1024;
//     for (int j = 8; j < 128; j++) {
//       if (entry_Ptr[j] != 0) {
//         return false;
//       }
//     }

//     std::uint8_t V = entry_Ptr[0] & 0x1;
//     std::uint8_t A = (entry_Ptr[0] >> 1) & 0x3;
//     std::uint8_t flags = entry_Ptr[0] & 0xff;
//     std::uint64_t addr = 0;
//     std::uint64_t entry_bits = 0;
//     for (int j = 0; j < 8; ++j)
//       addr |= entry_Ptr[j] << (j * 8);
//     addr &= 0xFFFFFFFFFFFFFFFF;
//     entry_bits = addr;
//     addr >>= 8;
//     addr <<= 12;
//     if (addr == 0x0)
//       continue;
//     ENTRY *entry = new ENTRY(addr, flags, V, A, i, entry_bits);
//     if (V == 0x00 && A == 0x00)
//       continue;
//     else if (V == 0x00 && A > 0 && A < 4)
//       this->pde_entry[i] = entry;
//     else
//       return false;
//   }
//   if (this->pde_entry.size() != 0)
//     // for (auto it=this->pde_entry.begin(); it != this->pde_entry.end(); it++)
//     //   it->second->constructEntry();
//     return true;
//   return false;
// }
