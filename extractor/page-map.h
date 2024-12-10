#ifndef _PAGE_MAP_H_
#define _PAGE_MAP_H_

#include <cstdint>
#include <map>

#include "mem-dump.h"
#include "trans.h"
#include "page-dir.h"

/*******************************************************************************
 *
 ******************************************************************************/
class PageMap : public Trans {
  std::map<int, Trans *> mPageMapEnts;
  
public:
  PageMap(MemDump &, std::uint64_t, TransType);
  
  ~PageMap();
  
  bool 
  constructTrans();
  
  void 
  printTrans(std::uint64_t);
};

#endif

