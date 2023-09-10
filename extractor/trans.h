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

/*******************************************************************************
 *
 ******************************************************************************/
class Trans {
protected:
  const std::uint8_t *  mDumpPtr;
  std::uint64_t         mPhyAddr;
  TransType             mTransType;
    
public:
  Trans(const std::uint8_t *ptr, std::uint64_t addr, TransType type) : 
      mDumpPtr(ptr), mPhyAddr(addr), mTransType(type) { }
  
  virtual 
  ~Trans() { }
  
  virtual bool 
  constructTrans() = 0;
  
  virtual void 
  printTrans(std::uint64_t) = 0;
};

#endif

