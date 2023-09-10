#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

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
  
  int fd = open(argv[1], O_RDONLY);
  if (fd < 0) {
    std::cout << "cannot open " << argv[1] << std::endl;
    return -1;
  }
  
  struct stat st;
  fstat(fd, &st);
  
  void *basePtr = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (basePtr == MAP_FAILED) {
    std::cout << "cannot mmap " << argv[1] << std::endl;
    return -1;
  }
  
  std::vector<Trans *> pageMaps;
  for (std::uint64_t offset = 0; offset < st.st_size; offset += 4096) {
    const std::uint8_t *dumpPtr = (std::uint8_t *)basePtr + offset;
    Trans *topPtr = new PageMap(dumpPtr, offset, PD3);
    bool ok = topPtr->constructTrans();
    if (!ok)
      delete topPtr;
    else
      pageMaps.push_back(topPtr);
  }
  
  for (auto topPtr : pageMaps)
    topPtr->printTrans(0);
}

