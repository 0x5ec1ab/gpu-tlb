#ifndef _MEM_DUMP_H_
#define _MEM_DUMP_H_

#define CHUNK_SIZE 4096

#include <stdexcept>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdint>

/*******************************************************************************
 *
 ******************************************************************************/
class MemDump {
  int           mFd;
  void *        mBasePtr;
  std::uint64_t mLen;
  std::uint64_t mPhysOffset;
  
public:
  MemDump(const char *, std::uint64_t physOffset = 0);
  
  ~MemDump();
  
  std::uint64_t  
  getChunkNum();
  
  std::uint8_t 
  getByte(std::uint64_t);
  
};

#endif

