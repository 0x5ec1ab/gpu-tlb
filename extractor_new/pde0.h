#ifndef _PDE0_H_
#define _PDE0_H_

#include "pde.h"
#include "pte.h"
class PDE0
{
public:
    uint64_t phy_addr;
    void *base_addr;
    void *entry_addr;
    PDEType type = PD0;

    ENTRY self_entry;

    std::map<int, ENTRY *> pde_entry_big;
    std::map<int, ENTRY *> pde_entry_small;
    std::map<int, PTE *> PTEs_big;
    std::map<int, PTE *> PTEs_small;
    std::map<int, ENTRY *> pte_entry;

public:
    PDE0(uint64_t phy_addr, void *base_addr, PDEType type, ENTRY self_entry)
        : phy_addr(phy_addr), base_addr(base_addr), self_entry(self_entry)
    {
        this->entry_addr = (uint8_t *)this->base_addr + this->phy_addr;
    }
    ~PDE0()
    {
        for (auto it = this->PTEs_big.begin(); it != this->PTEs_big.end(); it++)
        {
            delete it->second;
        }
        for (auto it = this->pde_entry_big.begin(); it != this->pde_entry_big.end();
             it++)
        {
            delete it->second;
        }
        for (auto it = this->pde_entry_small.begin();
             it != this->pde_entry_small.end(); it++)
        {
            delete it->second;
        }
        for (auto it = this->PTEs_small.begin(); it != this->PTEs_small.end();
             it++)
        {
            delete it->second;
        }
        for (auto it = this->pte_entry.begin(); it != this->pte_entry.end(); it++)
        {
            delete it->second;
        }

        this->PTEs_big.clear();
        this->pde_entry_big.clear();
        this->pde_entry_small.clear();
        this->PTEs_small.clear();
        this->pte_entry.clear();
    }

    void print(uint64_t addr)
    {
        for (int i = 0; i < this->type; i++)
            std::cout << "\t";
        std::cout << "PDE0: 0x" << this->phy_addr << "  type:" << this->type
                  << "  entry:" << std::hex << this->self_entry.entry_bits
                  << std::endl;
        for (auto it = this->pde_entry_big.begin(); it != this->pde_entry_big.end();
             it++)
        {
            for (int i = 0; i < this->type; i++)
                std::cout << "\t";
            std::cout << "\t";
            std::cout << std::dec << it->first << " "
                      << "big_A: " << (uint64_t)it->second->A << " ";
            std::cout << " big_V: " << (uint64_t)it->second->V << " ";
            std::cout << "big_flags: 0b" << std::bitset<8>(it->second->flags) << " ";
            std::cout << "big_next_phy_addr: 0x" << std::hex << it->second->addr
                      << std::endl;
            PTEs_big[it->first]->print(addr | ((uint64_t)it->first << 21));
        }
        for (auto it = this->pde_entry_small.begin();
             it != this->pde_entry_small.end(); it++)
        {
            for (int i = 0; i < this->type; i++)
                std::cout << "\t";
            std::cout << "\t";
            std::cout << std::dec << it->first << " "
                      << "small_A: " << (uint64_t)it->second->A << " ";
            std::cout << "small_V: " << (uint64_t)it->second->V << " ";
            std::cout << "small_flags: 0b" << std::bitset<8>(it->second->flags)
                      << " ";
            std::cout << "small_next_phy_addr: 0x" << std::hex << it->second->addr
                      << std::endl;
            PTEs_small[it->first]->print(addr | ((uint64_t)it->first << 21));
        }

        for (auto it = this->pte_entry.begin(); it != this->pte_entry.end(); it++)
        {
            for (int i = 0; i < this->type; i++)
                std::cout << "\t";
            std::cout << "\t PD0 is PTE:";
            std::cout << std::dec << it->first << " "
                      << "A: " << (uint64_t)it->second->A;
            std::cout << " V: " << (uint64_t)it->second->V << " ";
            std::cout << "flags: 0b" << std::bitset<8>(it->second->flags) << " ";
            std::cout << "Page_addr: 0x" << std::hex << it->second->addr << " ";
            std::cout << "Page_size: "
                      << "PAGE_2M"
                      << " virt_addr: 0x" << std::hex << (addr | ((uint64_t)it->first << 21))
                      << " " << std::endl;
            it->second->virt_addr = (addr | ((uint64_t)it->first << 21));
            it->second->small = PAGE_2M;
            //   PAGEs[it->first]->print();
        }
        std::cout << std::endl;
    }

    bool construct()
    {
        bool ok;
        ok = construct_PDE0_entry();
        if (!ok)
            return false;
        ok = construct_PDE();
        // ok |= construct_PTE();
        if (!ok)
        {
            if (this->pte_entry.size() != 0)
            {
                return true;
            }
            return false;
        }
        return true;
    }
    bool construct_PDE0_entry()
    {
        for (int i = 0; i < 256; i++)
        {
            uint8_t *entry_Ptr_big = (uint8_t *)this->entry_addr + i * 16;
            std::uint8_t V = entry_Ptr_big[0] & 0x1;
            std::uint8_t A_big = (entry_Ptr_big[0] >> 1) & 0x3;
            std::uint8_t flags_big = entry_Ptr_big[0] & 0xf;
            std::uint64_t addr_big = 0;
            std::uint64_t entry_bits_big = 0;

            uint8_t *entry_Ptr_small = entry_Ptr_big + 8;
            std::uint8_t A_small = (entry_Ptr_small[0] >> 1) & 0x3;
            std::uint8_t flags_small = entry_Ptr_small[0] & 0xf;
            std::uint64_t addr_small = 0;
            std::uint64_t entry_bits_small = 0;

            std::uint64_t addr_pte = 0;
            std::uint8_t flags_pte = entry_Ptr_big[0] & 0xff;

            if (V == 0)
            {

                if (A_big <= 1)
                {
                    for (int j = 5; j < 8; j++)
                    {
                        if (entry_Ptr_big[j] != 0)
                        {
                            return false;
                        }
                    }
                }
                else if (A_big < 4 && A_big > 1)
                {
                    for (int j = 7; j < 8; j++)
                    {
                        if (entry_Ptr_big[j] != 0)
                        {
                            return false;
                        }
                    }
                }
                else
                {
                    return false;
                }

                if (A_small <= 1)
                {
                    for (int j = 5; j < 8; j++)
                    {
                        if (entry_Ptr_small[j] != 0)
                        {
                            return false;
                        }
                    }
                }
                else if (A_small < 4 && A_small > 1)
                {
                    for (int j = 7; j < 8; j++)
                    {
                        if (entry_Ptr_small[j] != 0)
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
                addr_big |= (uint64_t)entry_Ptr_big[j] << (j * 8);
                addr_small |= (uint64_t)entry_Ptr_small[j] << (j * 8);
            }
            entry_bits_big = addr_big;
            entry_bits_small = addr_small;

            addr_pte = addr_big;
            if (A_big > 1)
                addr_pte &= 0x00FFFFFFFFFFFFFF;
            else
                addr_pte &= 0x00000000FFFFFFFF;

            addr_pte >>= 8;
            addr_pte <<= 12;

            if (A_big > 1)
                addr_big &= 0x00FFFFFFFFFFFFFF;
            else
                addr_big &= 0x00000000FFFFFFFF;
            addr_big >>= 4;
            addr_big <<= 8;

            if (A_small > 1)
                addr_small &= 0x00FFFFFFFFFFFFFF;
            else
                addr_small &= 0x00000000FFFFFFFF;
            addr_small >>= 8;
            addr_small <<= 12;

            //   if (addr_big != 0x0) {
            if (V == 0x00)
            {
                if (addr_big == 0x0 && addr_small == 0x0)
                    continue;
                if (addr_big != 0x0 && A_big >= 1)
                {
                    ENTRY *entry_big = new ENTRY(addr_big, flags_big, V, A_big, i,
                                                 entry_bits_big, PAGE_64K);
                    this->pde_entry_big[i] = entry_big;
                }
                if (addr_small != 0x0 && A_small >= 1)
                {
                    ENTRY *entry_small = new ENTRY(addr_small, flags_small, V, A_small, i,
                                                   entry_bits_small, PAGE_4K);
                    this->pde_entry_small[i] = entry_small;
                }
            }
            else if (V == 0x01)
            {
                ENTRY *entry_pte = new ENTRY(addr_pte, flags_pte, V, A_big, i,
                                             entry_bits_big, PAGE_2M);

                this->pte_entry[i] = entry_pte;
            }
            else
            {
                return false;
            }
        }
        if (this->pde_entry_big.size() != 0 || this->pde_entry_small.size() != 0 ||
            this->pte_entry.size() != 0)
            return true;
        return false;
    }

    bool construct_PDE()
    {
        if (this->pde_entry_big.size() == 0 && this->pde_entry_small.size() == 0)
        {
            return false;
        }
        for (auto it = this->pde_entry_big.begin();
             it != this->pde_entry_big.end();)
        {
            PTE *ptPtr = nullptr;
            if (it->second->small == PAGE_64K)
            {
                ptPtr = new PTE(it->second->addr, this->base_addr, PAGE64K,
                                *(it->second), 256, 8);
            }
            else
            {
                return false;
            }
            if (ptPtr == nullptr)
            {
                return false;
            }

            bool ok = ptPtr->construct();
            if (!ok)
            {
                delete ptPtr;
                it = this->pde_entry_big.erase(it);
            }
            else
            {
                PTEs_big[it->first] = ptPtr;
                it++;
            }
        }

        for (auto it = this->pde_entry_small.begin();
             it != this->pde_entry_small.end();)
        {
            PTE *ptPtr = nullptr;
            if (it->second->small == PAGE_4K && it->second->addr < 0x5f3c00000)
            {
                ptPtr = new PTE(it->second->addr, this->base_addr, PAGE4K,
                                *(it->second), 4096, 8);
            }
            else
            {
                return false;
            }
            if (ptPtr == nullptr)
            {
                return false;
            }

            bool ok = ptPtr->construct();
            if (!ok)
            {
                delete ptPtr;
                it = this->pde_entry_small.erase(it);
            }
            else
            {
                PTEs_small[it->first] = ptPtr;
                it++;
            }
        }

        if (this->pde_entry_small.size() != 0 || this->pde_entry_big.size() != 0)
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
    //       PTE *ptPtr = new PTE(it->second->addr, this->base_addr, PAGE2M,
    //                            *(it->second), 4096, 16);
    //       bool ok = ptPtr->construct();
    //       if (!ok) {
    //         delete ptPtr;
    //         it = this->pte_entry.erase(it);
    //       } else {
    //         PTEs[it->first] = ptPtr;
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