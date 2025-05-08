#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>

#include "mem-dump.h"
#include "trans.h"
#include "page-map.h"
#include "page-dir.h"
#include "page-tab.h"
#include "page.h"
#include "common.h"

/*******************************************************************************
 *
 ******************************************************************************/
int 
main(int argc, char *argv[])
{
  if (argc < 2 || argc > 4) {
    std::cout << "Usage: " << argv[0] << " <dump> [physical‑offset] [vgpu-offset]\n";
    return -1;
  }
  std::uint64_t physOffset = 0;
  if (argc == 3)
    physOffset = std::strtoull(argv[2], nullptr, 0);
  if (argc == 4)
    vgpuOffset = std::strtoull(argv[3], nullptr, 0);

  MemDump dump(argv[1], physOffset);

  std::uint64_t chunkNum = dump.getChunkNum();

  std::vector<Trans *> pageMaps;
  for (std::uint64_t i = 0; i < chunkNum; ++i) {
    std::uint64_t phyAddr = physOffset + i * CHUNK_SIZE;
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

