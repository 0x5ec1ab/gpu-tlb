#include "mem-dump.h"

/*******************************************************************************
 *
 ******************************************************************************/
MemDump::MemDump(const char *fileName, std::uint64_t physOffset)
: mPhysOffset(physOffset)
{
  mFd = open(fileName, O_RDONLY);
  if (mFd < 0)
    throw std::runtime_error("cannot open the dump file");
  
  struct stat st;
  fstat(mFd, &st);
  mLen = st.st_size;
  
  mBasePtr = mmap(NULL, mLen, PROT_READ, MAP_PRIVATE, mFd, 0);
  if (mBasePtr == MAP_FAILED)
    throw std::runtime_error("cannot mmap the dump file");
}

/*******************************************************************************
 *
 ******************************************************************************/
MemDump::~MemDump()
{
  munmap(mBasePtr, mLen);
  close(mFd);
}

/*******************************************************************************
 *
 ******************************************************************************/
std::uint64_t  
MemDump::getChunkNum()
{
  return mLen / CHUNK_SIZE;
}

/*******************************************************************************
 *
 ******************************************************************************/
std::uint8_t
MemDump::getByte(std::uint64_t physAddress)
{
  if (physAddress < mPhysOffset)
    return 0;
  std::uint64_t fileOff = physAddress - mPhysOffset;
  if (fileOff < mLen)
    return *((std::uint8_t *)mBasePtr + fileOff);
  return 0;
}


