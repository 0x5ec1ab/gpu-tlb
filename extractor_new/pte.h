#ifndef _PTE_H_
#define _PTE_H_

#include "pde.h"
#include <string>

class PTE
{
public:
    uint64_t phy_addr;
    void *base_addr;
    void *entry_addr;
    ENTRY self_entry;
    std::map<int, ENTRY *> pte_entry;
    PDEType type;

    uint64_t pte_page_size = 4096;
    uint64_t pte_entry_size = 8;

public:
    PTE(uint64_t phy_addr, void *base_addr, PDEType type, ENTRY self_entry)
        : phy_addr(phy_addr), base_addr(base_addr), self_entry(self_entry)
    {
        this->type = type;
        this->entry_addr = (uint8_t *)this->base_addr + this->phy_addr;
    }

    PTE(uint64_t phy_addr, void *base_addr, PDEType type, ENTRY self_entry,
        uint64_t pte_page_size, uint64_t pte_entry_size)
        : phy_addr(phy_addr), base_addr(base_addr), self_entry(self_entry),
          pte_page_size(pte_page_size), pte_entry_size(pte_entry_size)
    {
        this->type = type;
        this->entry_addr = (uint8_t *)this->base_addr + this->phy_addr;
    }
    ~PTE()
    {
        for (auto it = this->pte_entry.begin(); it != this->pte_entry.end(); it++)
        {
            delete it->second;
        }
        this->pte_entry.clear();
    }

    void print(uint64_t addr)
    {
        std::string entry_type;
        if (this->type == PAGE64K)
        {
            entry_type = "PAGE_64K";
        }
        else if (this->type == PAGE4K)
        {
            entry_type = "PAGE_4K";
        }

        for (int i = 0; i < this->type - 3; i++)
            std::cout << "\t";
        std::cout << "PTE: 0x" << this->phy_addr;
        std::cout << "  type:" << entry_type;
        std::cout << "  entry: 0x" << std::hex << this->self_entry.entry_bits
                  << std::endl;
        for (auto it = this->pte_entry.begin(); it != this->pte_entry.end(); it++)
        {
            for (int i = 0; i < this->type; i++)
                std::cout << "\t";
            std::cout << "\t";
            std::cout << std::dec << it->first << " "
                      << "A: " << (uint64_t)it->second->A;
            std::cout << " V: " << (uint64_t)it->second->V << " ";
            std::cout << "flags: 0b" << std::bitset<8>(it->second->flags) << " ";
            std::cout << "Page_addr: 0x" << std::hex << it->second->addr << " ";
            std::cout << "Page_size: " << entry_type << " ";
            std::cout << "entry: 0x" << std::hex << it->second->entry_bits << " ";
            uint64_t addr_tmp = addr;
            if (entry_type == "PAGE_64K")
                addr_tmp = addr | ((uint64_t)it->first << 16);
            else if (entry_type == "PAGE_4K")
                addr_tmp = addr | ((uint64_t)it->first << 12);
            std::cout << "virt_addr: 0x" << std::hex << addr_tmp;
            std::cout << " " << std::endl;
            it->second->virt_addr = addr_tmp;
        }
        std::cout << std::endl;
    }

    bool construct()
    {
        bool ok;
        ok = construct_PTE_entry();
        if (!ok)
            return false;
        else
            return true;
    }

    bool construct_PTE_entry()
    {
        uint64_t entry_nums = this->pte_page_size / this->pte_entry_size;
        for (int i = 0; (uint64_t)i < entry_nums; i++)
        {
            uint8_t *entry_Ptr = (uint8_t *)this->entry_addr + i * this->pte_entry_size;

            std::uint8_t V = entry_Ptr[0] & 0x1;
            std::uint8_t A = (entry_Ptr[0] >> 1) & 0x3;
            std::uint8_t flags = entry_Ptr[0] & 0xff;
            std::uint64_t addr = 0;
            std::uint64_t entry_bits = 0;
            pagetype entry_type;
            if (this->type == PAGE512M)
                entry_type = PAGE_512M;
            else if (this->type == PAGE2M)
                entry_type = PAGE_2M;
            else if (this->type == PAGE64K)
                entry_type = PAGE_64K;
            else if (this->type == PAGE4K)
                entry_type = PAGE_4K;
            //   if (V != 0x1)
            //     return false;

            for (int j = 7; j >= 0; --j)
            {
                addr |= entry_Ptr[j];
                if (j != 0)
                {
                    addr = addr << 8;
                }
            }
            entry_bits = addr;
            addr &= 0x00FFFFFFFFFFFFFF;
            addr >>= 8;
            addr <<= 12;
            if (addr == 0x0 && V == 0x0)
                continue;

            ENTRY *entry = new ENTRY(addr, flags, V, A, i, entry_bits, entry_type);
            if (V == 0x01 && A < 0x04)
                this->pte_entry[i] = entry;
            else
                return false;
        }
        if (this->pte_entry.size() != 0)
            return true;
        return false;
    }
};

#endif