#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "mem-dump.h"
#include "trans.h"
#include "page-map.h"
#include "page-dir.h"
#include "page-tab.h"
#include "page.h"

/*******************************************************************************
 *
 ******************************************************************************/
int 
main(int argc, char *argv[])
{
  if (argc != 2) {
    std::cout << argv[0] << " <dump>\n";
    return -1;
  }
  
  MemDump dump(argv[1]);
  std::uint64_t chunkNum = dump.getChunkNum();
  
  std::vector<Trans *> pageMaps;
  for (std::uint64_t i = 0; i < chunkNum; ++i) {
    std::uint64_t phyAddr = i * CHUNK_SIZE;
    Trans *topPtr = new PageMap(dump, phyAddr, PD3);
    bool ok = topPtr->constructTrans();
    if (!ok)
      delete topPtr;
    else
      pageMaps.push_back(topPtr);
  }
  
  for (auto topPtr : pageMaps)
    topPtr->printTrans(0);
}

