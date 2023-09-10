#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

int 
main(int argc, char *argv[]) {
  // retrieve NVIDIA GPU's BAR0
  uint64_t bar0Addr;
  
  std::ifstream iomemFile("/proc/iomem");
  if (!iomemFile.is_open()) {
    std::cout << "cannot open /proc/iomem" << std::endl;
    return -1;
  }
  
  std::string line;
  while (std::getline(iomemFile, line)) {
    if (line.find("nvidia") == std::string::npos)
      continue;
    std::istringstream iss(line);
    std::string str;
    std::getline(iss, str, '-');
    bar0Addr = std::stoul(str, nullptr, 16);
    break;
  }
  
  iomemFile.close();
  
  // access memory via physical addresses
  int fd = open("/dev/mem", O_RDWR | O_SYNC);
  if (fd == -1) {
    std::cout << "cannot open /dev/mem: " << std::endl;
    return -1;
  }
  
  size_t len = 0x1000000;
  void *phyMem = mmap(0, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, bar0Addr);
  
  uint64_t regAddr = (uint64_t)phyMem + 0x1700;
  uint64_t winAddr = (uint64_t)phyMem + 0x700000;
  
  uint64_t pteAddr = std::stoul(argv[1], nullptr, 16);
  uint64_t pteVal = std::stoul(argv[2], nullptr, 16);
  uint64_t offAmnt = pteAddr & 0x000000000000FFFF;
  
  // move window to the target address
  *(volatile uint32_t *)regAddr = pteAddr >> 16;
  
  // we change 8 bytes of each PTE 
  for (int i = 0; i < 8; ++i) {
    uint8_t byte = (pteVal >> i * 8) & 0x00000000000000FF;
    *(volatile uint8_t *)(winAddr + offAmnt + i) = byte;
  }
  
  munmap(phyMem, len);
  close(fd);
  
  return 0;
}

