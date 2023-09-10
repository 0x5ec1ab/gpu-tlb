#ifndef _PAGE_H_
#define _PAGE_H_

#include <cstdint>
#include <iostream>
#include <iomanip>
#include <string>

#include "trans.h"

/*******************************************************************************
 *
 ******************************************************************************/
class Page : public Trans {    
public:
  Page(const std::uint8_t *ptr, std::uint64_t addr, TransType type) : 
      Trans(ptr, addr, type) { }
  
  ~Page() { }
  
  bool 
  constructTrans() 
  {
    return true;
  }
  
  void 
  printTrans(std::uint64_t virtAddr)
  {
    std::string sizeStr = "2MB";
    std::uint64_t mask = 0x0FF;
    int shBits = 21;
    if (mTransType == LARGE) {
      sizeStr = "64KB";
      mask = 0x01F;
      shBits = 16;
    } else if (mTransType == SMALL) {
      sizeStr = "4KB";
      mask = 0x1FF;
      shBits = 12;
    }
    
    std::cout << "\t\t\t\t\t" << std::dec << std::setw(3) << std::setfill(' ')
              << (virtAddr >> shBits & mask) << "-->" + sizeStr + "-Page@0x";
    std::cout << std::hex << std::setw(10) << std::setfill('0');
    std::cout << mPhyAddr << "\tVA: 0x" << virtAddr << std::endl;
  }
};

#endif

