#ifndef _PDE1_H_
#define _PDE1_H_

#include "pde.h"
#include "pde0.h"
#include "pte.h"
class PDE1
{
public:
    uint64_t phy_addr;
    void *base_addr;
    void *entry_addr;
    PDEType type = PD1;

    ENTRY self_entry;
    std::map<int, ENTRY *> pde_entry;
    std::map<int, PDE0 *> PDE0s;

    std::map<int, ENTRY *> pte_entry;
    //   std::map<int, PTE_PAGE *> PAGEs;

public:
    PDE1(uint64_t phy_addr, void *base_addr, PDEType type, ENTRY self_entry)
        : phy_addr(phy_addr), base_addr(base_addr), self_entry(self_entry)
    {
        this->entry_addr = (uint8_t *)this->base_addr + this->phy_addr;
    }

    ~PDE1()
    {
        for (auto it = this->pde_entry.begin(); it != this->pde_entry.end(); it++)
        {
            delete it->second;
        }
        for (auto it = this->PDE0s.begin(); it != this->PDE0s.end(); it++)
        {
            delete it->second;
        }
        // for (auto it = this->PAGEs.begin(); it != this->PAGEs.end(); it++) {
        //   delete it->second;
        // }
        for (auto it = this->pte_entry.begin(); it != this->pte_entry.end(); it++)
        {
            delete it->second;
        }
        this->pde_entry.clear();
        this->PDE0s.clear();
        // this->PAGEs.clear();
        this->pte_entry.clear();
    }

    void print(uint64_t addr)
    {
        for (int i = 0; i < this->type; i++)
            std::cout << "\t";
        std::cout << "PDE1: 0x" << this->phy_addr << "  type:" << this->type
                  << "  entry:" << std::hex << this->self_entry.entry_bits
                  << std::endl;
        for (auto it = this->pde_entry.begin(); it != this->pde_entry.end(); it++)
        {
            for (int i = 0; i < this->type; i++)
                std::cout << "\t";
            std::cout << "\t";
            std::cout << std::dec << it->first << " "
                      << "A: " << (uint64_t)it->second->A;
            std::cout << " V: " << (uint64_t)it->second->V << " ";
            std::cout << "flags: 0b" << std::bitset<8>(it->second->flags) << " ";
            std::cout << "next_phy_addr: 0x" << std::hex << it->second->addr
                      << std::endl;
            PDE0s[it->first]->print(addr | ((uint64_t)it->first << 29));
        }
        for (auto it = this->pte_entry.begin(); it != this->pte_entry.end(); it++)
        {
            for (int i = 0; i < this->type; i++)
                std::cout << "\t";
            std::cout << "\t PDE1 is PTE:";
            std::cout << std::dec << it->first << " "
                      << "A: " << (uint64_t)it->second->A;
            std::cout << " V: " << (uint64_t)it->second->V << " ";
            std::cout << "flags: 0b" << std::bitset<8>(it->second->flags) << " ";
            std::cout << "Page_addr: 0x" << std::hex << it->second->addr << " ";
            std::cout << "Page_size: "
                      << "PAGE_512M"
                      << " virt_addr: 0x" << std::hex << (addr | ((uint64_t)it->first << 29))
                      << " " << std::endl;
            it->second->virt_addr = (addr | ((uint64_t)it->first << 29));
            it->second->small = PAGE_512M;
            //   PAGEs[it->first]->print();
        }
        std::cout << std::endl;
    }
    bool construct()
    {
        bool ok;
        ok = construct_PDE1_entry();
        if (!ok)
            return false;
        ok = construct_PDE0();
        // ok |= construct_PTE();
        if (!ok)
        {
            return false;
        }
        return true;
    }
    bool construct_PDE1_entry()
    {
        for (int i = 0; i < 512; i++)
        {
            uint8_t *entry_Ptr = (uint8_t *)this->entry_addr + i * 8;

            std::uint8_t V = entry_Ptr[0] & 0x1;
            std::uint8_t A = (entry_Ptr[0] >> 1) & 0x3;
            std::uint8_t flags = entry_Ptr[0] & 0xff;
            std::uint64_t addr = 0;
            std::uint64_t entry_bits = 0;
            if (V == 0)
            {
                if (A <= 1)
                {
                    for (int j = 5; j < 8; j++)
                    {
                        if (entry_Ptr[j] != 0)
                        {
                            return false;
                        }
                    }
                }
                else if (A > 1 && A < 4)
                {
                    for (int j = 7; j < 8; j++)
                    {
                        if (entry_Ptr[j] != 0)
                        {
                            return false;
                        }
                    }
                }
                else
                {
                    return false;
                }
            }
            for (int j = 0; j < 8; ++j)
            {
                addr |= entry_Ptr[j] << (j * 8);
            }
            entry_bits = addr;
            if (A > 1)
                addr &= 0x00FFFFFFFFFFFFFF;
            else
                addr &= 0x00000000FFFFFFFF;
            addr >>= 8;
            addr <<= 12;
            if (addr == 0x0)
                continue;

            if (V == 0x00 && A == 0x00)
                continue;
            else if (V == 0x00 && A > 0 && A < 4)
            {
                ENTRY *entry_pde = new ENTRY(addr, flags, V, A, i, entry_bits);
                this->pde_entry[i] = entry_pde;
            }
            else if (V == 0x01 && A < 4)
            {
                ENTRY *entry_pte = new ENTRY(addr, flags, V, A, i, entry_bits);
                this->pte_entry[i] = entry_pte;
            }
            else
                return false;
        }
        if (this->pde_entry.size() != 0 || this->pte_entry.size() != 0)
            return true;
        return false;
    }

    //   bool construct_PTE_entry() {
    //     for (int i = 0; i < 512; i++) {
    //       uint8_t *entry_Ptr = (uint8_t *)this->entry_addr + i * 8;

    //       std::uint8_t V = entry_Ptr[0] & 0x1;
    //       std::uint8_t A = (entry_Ptr[0] >> 1) & 0x3;
    //       std::uint8_t flags = entry_Ptr[0] & 0xff;
    //       std::uint64_t addr = 0;
    //       std::uint64_t entry_bits = 0;

    //       if (A <= 1) {
    //         for (int j = 5; j < 8; j++) {
    //           if (entry_Ptr[j] != 0) {
    //             return false;
    //           }
    //         }
    //       } else if (A > 1 && A < 4) {
    //         for (int j = 7; j < 8; j++) {
    //           if (entry_Ptr[j] != 0) {
    //             return false;
    //           }
    //         }
    //       } else {
    //         return false;
    //       }

    //       for (int j = 0; j < 8; ++j)
    //         addr |= entry_Ptr[j] << (j * 8);
    //       entry_bits = addr;
    //       addr &= 0xFFFFFFFFFFFFFFFF;
    //       addr >>= 8;
    //       addr <<= 12;
    //       if (addr == 0x0)
    //         continue;
    //       ENTRY *entry = new ENTRY(addr, flags, V, A, i, entry_bits);
    //       if (V == 0x00 && A == 0x00)
    //         continue;
    //       else if (V == 0x00 && A > 0 && A < 4)
    //         this->pde_entry[i] = entry;
    //       else
    //         return false;
    //     }
    //     if (this->pde_entry.size() != 0)
    //       return true;
    //     return false;
    //   }

    bool construct_PDE0()
    {
        if (this->pde_entry.size() == 0)
        {
            return false;
        }
        for (auto it = this->pde_entry.begin(); it != this->pde_entry.end();)
        {
            PDE0 *ptPtr =
                new PDE0(it->second->addr, this->base_addr, PD0, *(it->second));
            bool ok = ptPtr->construct();
            if (!ok)
            {
                delete ptPtr;
                it = this->pde_entry.erase(it);
            }
            else
            {
                PDE0s[it->first] = ptPtr;
                it++;
            }
        }

        if (this->pde_entry.size() != 0)
        {
            return true;
        }
        return false;
    }

    // bool construct_PTE() {
    // if (this->pte_entry.size() == 0) {
    //   return false;
    // }
    // for (auto it = this->pte_entry.begin(); it != this->pte_entry.end();) {
    //   PTE_PAGE *ptPtr =
    //       new PTE_PAGE(it->second->addr, this->base_addr, PAGE2M,
    //       *(it->second), 4096, 16);
    //   bool ok = ptPtr->construct();
    //   if (!ok) {
    //     delete ptPtr;
    //     it = this->pte_entry.erase(it);
    //   } else {
    //     PAGEs[it->first] = ptPtr;
    //     it++;
    //   }
    // }

    //     if (this->pte_entry.size() != 0) {
    //       return true;
    //     }
    //     return false;
    //   }
};
#endif