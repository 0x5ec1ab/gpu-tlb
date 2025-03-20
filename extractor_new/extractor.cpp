#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "pde.h"
#include "pde3.h"
#include "vm_area_struct.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << argv[0] << " <dump>\n";
        return -1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0)
    {
        std::cout << "cannot open " << argv[1] << std::endl;
        return -1;
    }

    struct stat st;
    fstat(fd, &st);
    std::cout << "size: " << std::hex << st.st_size << std::endl;
    void *basePtr = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (basePtr == MAP_FAILED)
    {
        std::cout << "cannot mmap " << argv[1] << std::endl;
        return -1;
    }

    std::vector<PDE3 *> PDE3s;
    for (std::uint64_t offset = 0; offset < (uint64_t)st.st_size;
         offset += 4096)
    {
        // if (offset == 0x5eef36000)
        //   std::cout << "offset: " << std::hex << offset << std::endl;
        uint32_t *value = (uint32_t *)((char *)basePtr + offset);
        if (value[0] == 0xabffee10 && value[1] == 0xabffee11)
            std::cout << "offset: " << std::hex << offset << " value: " << std::hex
                      << value << std::endl;

        PDE3 *topPtr = new PDE3(offset, basePtr, PD3, ENTRY());

        bool ok = topPtr->construct();
        if (!ok)
            delete topPtr;
        else
            PDE3s.push_back(topPtr);
    }

    for (auto topPtr : PDE3s)
    {
        uint64_t pd3_addr = topPtr->phy_addr;
        bool is_pd3 = true;
        for (auto itpd3 = PDE3s.begin(); itpd3 != PDE3s.end(); itpd3++)
        {
            for (auto itpd2 = (*itpd3)->PDE2s.begin(); itpd2 != (*itpd3)->PDE2s.end();
                 itpd2++)
            {
                if (itpd2->second->phy_addr == pd3_addr)
                {
                    is_pd3 = false;
                    break;
                }
            }
        }
        if (is_pd3)
        {
            topPtr->print(0);
            struct vm_area_struct_head *head = visualize_virtual_address_space(topPtr);
            print_area(head);
        }
    }
}