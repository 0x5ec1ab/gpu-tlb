#ifndef _TRANS_H_
#define _TRANS_H_

#include <cstdint>

#include "mem-dump.h"

/*******************************************************************************
 *
 ******************************************************************************/
enum TransType {
  PD3,
  PD2,
  PD1,
  PD0,
  PT,
  HUGE,
  LARGE,
  SMALL
};

enum PTEAperture {
  APERTURE_VIDEO_LOCAL = 0b00,
  APERTURE_VIDEO_PEER = 0b01,
  APERTURE_SYSTEM_MEMORY_COHERENT = 0b10,
  APERTURE_SYSTEM_MEMORY_NONCOHERENT = 0b11
};

/*******************************************************************************
 *
 ******************************************************************************/
class Trans {
protected:
  MemDump &     mMemDump;
  std::uint64_t mPhyAddr;
  TransType     mTransType;
  std::uint8_t  mFlag;
  
public:
  Trans(MemDump &dump, std::uint64_t addr, TransType type, std::uint8_t flag = 0) : 
      mMemDump(dump), mPhyAddr(addr), mTransType(type), mFlag(flag) { }
  
  virtual 
  ~Trans() { }
  
  virtual bool 
  constructTrans() = 0;
  
  virtual void 
  printTrans(std::uint64_t) = 0;
};

#endif

