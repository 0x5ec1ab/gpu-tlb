#ifndef _PAGE_DIR_H_
#define _PAGE_DIR_H_

#include <cstdint>
#include <map>

#include "mem-dump.h"
#include "trans.h"
#include "page-tab.h"
#include "page.h"

/*******************************************************************************
 *
 ******************************************************************************/
class PageDir : public Trans {
  std::map<int, std::pair<Trans *, Trans *>> mPageDirEnts;
  
public:
  PageDir(MemDump &, std::uint64_t, TransType);
  
  ~PageDir();
  
  bool 
  constructTrans();
  
  void 
  printTrans(std::uint64_t);
};

#endif

