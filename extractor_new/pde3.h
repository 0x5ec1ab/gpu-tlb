#ifndef _PDE3_H_
#define _PDE3_H_

#include "pde.h"
#include "pde2.h"

class PDE3
{
public:
    uint64_t phy_addr;
    void *base_addr;
    void *entry_addr;
    PDEType type = PD3;

    ENTRY self_entry;
    std::map<int, ENTRY *> pde_entry;
    std::map<int, PDE2 *> PDE2s;

public:
    PDE3(uint64_t phy_addr, void *base_addr, PDEType type, ENTRY self_entry)
        : phy_addr(phy_addr), base_addr(base_addr), self_entry(self_entry)
    {
        this->entry_addr = (uint8_t *)this->base_addr + this->phy_addr;
    }

    ~PDE3()
    {
        for (auto it = this->pde_entry.begin(); it != this->pde_entry.end(); it++)
        {
            delete it->second;
        }
        for (auto it = this->PDE2s.begin(); it != this->PDE2s.end(); it++)
        {
            delete it->second;
        }
        this->pde_entry.clear();
        this->PDE2s.clear();
    }

    void print(uint64_t addr)
    {
        for (int i = 0; i < this->type; i++)
            std::cout << "\t";
        std::cout << "PDE3: 0x" << this->phy_addr << "  type:" << this->type
                  << std::endl;
        for (auto it = this->pde_entry.begin(); it != this->pde_entry.end(); it++)
        {
            std::cout << "\t";
            std::cout << it->first << " "
                      << "A: " << (uint64_t)it->second->A;
            std::cout << " V: " << (uint64_t)it->second->V << " ";
            std::cout << "flags: 0b" << std::bitset<8>(it->second->flags) << " ";
            std::cout << "next_phy_addr: 0x" << std::hex << it->second->addr
                      << std::endl;
            PDE2s[it->first]->print(addr | ((uint64_t)it->first << 47));
        }
        std::cout << std::endl;
    }

    bool construct()
    {
        bool ok;
        ok = construct_PDE3_entry();
        if (!ok)
            return false;
        ok = construct_PDE2();
        if (!ok)
            return false;

        return true;
    }

    bool construct_PDE3_entry()
    {
        for (int i = 4; i < 512; i++)
        {
            uint8_t *entry_Ptr = (uint8_t *)this->entry_addr + i * 8;
            for (int j = 0; j < 8; j++)
            {
                if (entry_Ptr[j] != 0)
                {
                    return false;
                }
            }
        }
        for (int i = 0; i < 4; i++)
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
            ENTRY *entry = new ENTRY(addr, flags, V, A, i, entry_bits);
            if (V == 0x00 && A == 0x00)
                continue;
            else if (V == 0x00 && A > 0 && A < 4)
                this->pde_entry[i] = entry;
            else
                return false;
        }
        if (this->pde_entry.size() != 0)
            return true;
        return false;
    }

    bool construct_PDE2()
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
            PDE2 *pde2Ptr =
                new PDE2(it->second->addr, this->base_addr, PD2, *(it->second));
            bool ok = pde2Ptr->construct();
            if (!ok)
            {
                delete pde2Ptr;
                it = this->pde_entry.erase(it);
            }
            else
            {
                PDE2s[it->first] = pde2Ptr;
                it++;
            }
        }
        if (this->pde_entry.size() != 0)
        {
            return true;
        }
        return false;
    }
};

#endif