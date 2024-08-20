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
  Page(const std::uint8_t *ptr, std::uint64_t addr, TransType type, std::uint8_t flags);
  
  ~Page();
  
  bool
  constructTrans();
  
  void 
  printTrans(std::uint64_t virtAddr);
};

#endif

