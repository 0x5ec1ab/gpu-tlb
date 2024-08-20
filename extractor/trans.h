#ifndef _TRANS_H_
#define _TRANS_H_

#include <cstdint>

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
  const std::uint8_t *  mDumpPtr;
  std::uint64_t         mPhyAddr;
  TransType             mTransType;
  std::uint8_t          mFlags;
    
public:
  Trans(const std::uint8_t *ptr, std::uint64_t addr, TransType type, std::uint8_t flags = 0) : 
      mDumpPtr(ptr), mPhyAddr(addr), mTransType(type), mFlags(flags) { }
  
  virtual 
  ~Trans() { }
  
  virtual bool 
  constructTrans() = 0;
  
  virtual void 
  printTrans(std::uint64_t) = 0;
};

#endif

