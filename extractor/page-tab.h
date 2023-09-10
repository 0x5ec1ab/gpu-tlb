#ifndef _PAGE_TAB_H_
#define _PAGE_TAB_H_

#include <cstdint>
#include <map>

#include "trans.h"
#include "page.h"

/*******************************************************************************
 *
 ******************************************************************************/
class PageTab : public Trans {
  TransType               mPageType;
  std::map<int, Trans *>  mPageTabEnts;
  
public:
  PageTab(const std::uint8_t *, std::uint64_t, TransType, TransType);
  
  ~PageTab();
  
  bool 
  constructTrans();
  
  void 
  printTrans(std::uint64_t);
};

#endif

