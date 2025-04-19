#ifndef _PDE2_H_
#define _PDE2_H_

#include "pde.h"
#include "pde1.h"
#include "pte.h"
class PDE2
{
public:
    uint64_t phy_addr;
    void *base_addr;
    void *entry_addr;
    PDEType type = PD2;

    ENTRY self_entry;
    std::map<int, ENTRY *> pde_entry;
    std::map<int, PDE1 *> PDE1s;

    //   std::map<int, ENTRY *> pte_entry;
    //   std::map<int, PTE_PAGE *> PAGEs;

public:
    PDE2(uint64_t phy_addr, void *base_addr, PDEType type, ENTRY self_entry)
        : phy_addr(phy_addr), base_addr(base_addr), self_entry(self_entry)
    {
        this->entry_addr = (uint8_t *)this->base_addr + this->phy_addr;
    }
    ~PDE2()
    {
        for (auto it = this->pde_entry.begin(); it != this->pde_entry.end(); it++)
        {
            delete it->second;
        }
        for (auto it = this->PDE1s.begin(); it != this->PDE1s.end(); it++)
        {
            delete it->second;
        }
        // for (auto it = this->PAGEs.begin(); it != this->PAGEs.end(); it++) {
        //   delete it->second;
        // }
        // for (auto it = this->pte_entry.begin(); it != this->pte_entry.end(); it++) {
        //   delete it->second;
        // }
        this->pde_entry.clear();
        this->PDE1s.clear();
        // this->PAGEs.clear();
        // this->pte_entry.clear();
    }

    void print(uint64_t addr)
    {
        for (int i = 0; i < this->type; i++)
            std::cout << "\t";
        std::cout << "PDE2: 0x" << this->phy_addr << "  type:" << this->type
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
            PDE1s[it->first]->print(addr | ((uint64_t)it->first << 38));
        }
        // for (auto it = this->pte_entry.begin(); it != this->pte_entry.end(); it++) {
        //   for (int i = 0; i < this->type; i++)
        //     std::cout << "\t";
        //   std::cout << "\t";
        //   std::cout << std::dec << it->first << " "
        //             << "A: " << (uint64_t)it->second->A;
        //   std::cout << " V: " << (uint64_t)it->second->V << " ";
        //   std::cout << "flags: 0b" << std::bitset<8>(it->second->flags) << " ";
        //   std::cout << "next_phy_addr: 0x" << std::hex << it->second->addr
        //             << std::endl;
        //   PAGEs[it->first]->print();
        // }
        std::cout << std::endl;
    }

    bool construct()
    {
        bool ok;
        ok = construct_PDE2_entry();
        if (!ok)
            return false;
        ok = construct_PDE1();
        // ok |= construct_PTE();
        if (!ok)
        {
            return false;
        }
        return true;
    }
    bool construct_PDE2_entry()
    {
        for (int i = 0; i < 512; i++)
        {
            uint8_t *entry_Ptr = (uint8_t *)this->entry_addr + i * 8;

            std::uint8_t V = entry_Ptr[0] & 0x1;
            std::uint8_t A = (entry_Ptr[0] >> 1) & 0x3;
            std::uint8_t flags = entry_Ptr[0] & 0xff;
            std::uint64_t addr = 0;
            std::uint64_t entry_bits = 0;

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
            ENTRY *entry_pde = new ENTRY(addr, flags, V, A, i, entry_bits);
            // ENTRY *entry_pte = new ENTRY(addr, flags, V, A, i, entry_bits);
            if (V == 0x00 && A == 0x00)
                continue;
            else if (V == 0x00 && A > 0 && A < 4) // PDE2
            {
                this->pde_entry[i] = entry_pde;
                // this->pte_entry[i] = entry_pte;
            }
            //   else if(V==0x01 && A < 4 ) // PTE
            //   {
            //     this->pte_entry[i] = entry_pde;
            //   }
            else
                return false;
        }
        if (this->pde_entry.size() != 0)
            return true;
        return false;
    }

    bool construct_PDE1()
    {
        if (this->pde_entry.size() == 0)
        {
            return false;
        }
        for (auto it = this->pde_entry.begin(); it != this->pde_entry.end();)
        {
            if (it->second->addr > 0x5f3c00000)
            {
                return false;
            }
            ENTRY *tmpentry = it->second;
            if (tmpentry->A > 0x01)
            {
                return false;
            }
            PDE1 *ptPtr =
                new PDE1(it->second->addr, this->base_addr, PD1, *(it->second));
            bool ok = ptPtr->construct();
            if (!ok)
            {
                delete ptPtr;
                it = this->pde_entry.erase(it);
            }
            else
            {
                PDE1s[it->first] = ptPtr;
                it++;
            }
        }

        if (this->pde_entry.size() != 0)
        {
            return true;
        }
        return false;
    }

    //   bool construct_PTE() {
    //     if (this->pte_entry.size() == 0) {
    //       return false;
    //     }
    //     for (auto it = this->pte_entry.begin(); it != this->pte_entry.end();) {
    //       PTE_PAGE *ptPtr =
    //           new PTE_PAGE(it->second->addr, this->base_addr, PAGE512M, *(it->second));
    //       bool ok = ptPtr->construct();
    //       if (!ok) {
    //         delete ptPtr;
    //         it = this->pte_entry.erase(it);
    //       } else {
    //         PAGEs[it->first] = ptPtr;
    //         it++;
    //       }
    //     }

    //     if (this->pte_entry.size() != 0) {
    //       return true;
    //     }
    //     return false;
    //   }
};

#endif