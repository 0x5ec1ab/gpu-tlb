#include "page-map.h"

/*******************************************************************************
 *
 ******************************************************************************/
PageMap::PageMap(const std::uint8_t *ptr, std::uint64_t addr, TransType type) : 
    Trans(ptr, addr, type)
{ }

/*******************************************************************************
 *
 ******************************************************************************/
PageMap::~PageMap()
{ 
  for (const auto &ent : mPageMapEnts)
    delete ent.second;
}

/*******************************************************************************
 *
 ******************************************************************************/
bool 
PageMap::constructTrans()
{
  // PD3 has 4 entries, but PD2 and PD1 has 512 entries
  int entNum = mTransType == PD3 ? 4 : 512;
  int entSize = 4096 / entNum;
  
  for (int i = 0; i < entNum; ++i) {
    const std::uint8_t *entPtr = mDumpPtr + i * entSize;
    
    // a valid entry has zeros for most bytes
    for (int j = 5; j < entSize; ++j)
      if (entPtr[j] != 0)
        return false;
    
    std::uint64_t addr = 0;
    for (int j = 0; j < 5; ++j)
      addr |= entPtr[j] << (j * 8);
    addr &= 0x0000000FFFFFFFFF;
    addr >>= 8;
    
    std::uint8_t flag = *entPtr & 0x07;
    
    if (flag == 0x00 && addr == 0)
      continue;
    else if (flag != 0x02)
      return false;
    
    // construct the next-level trans
    TransType nTransType = mTransType == PD3 ? PD2 : 
                           mTransType == PD2 ? PD1 : PD0;
    std::uint64_t nPhyAddr = addr << 12;
    std::int64_t offset = nPhyAddr - mPhyAddr;
    const std::uint8_t *nDumpPtr = mDumpPtr + offset;
    
    Trans *next = nullptr;
    if (nTransType == PD0)
      next = new PageDir(nDumpPtr, nPhyAddr, nTransType);
    else
      next = new PageMap(nDumpPtr, nPhyAddr, nTransType);
    bool ok = next->constructTrans();
    if (!ok) {
      delete next;
      return false;
    }
    
    mPageMapEnts[i] = next;
  }
  
  if (mPageMapEnts.empty())
    return false;
  else
    return true;
}

/*******************************************************************************
 *
 ******************************************************************************/
void 
PageMap::printTrans(std::uint64_t virtAddr)
{
  if (mTransType == PD3)
    std::cout << "PD3@0x";
  else if (mTransType == PD2)
    std::cout << "\t" << std::dec << std::setw(3) << std::setfill(' ')
              << (virtAddr >> 47 & 0x003) << "-->PD2@0x";
  else
    std::cout << "\t\t" << std::dec << std::setw(3) << std::setfill(' ')
              << (virtAddr >> 38 & 0x1FF) << "-->PD1@0x";
  std::cout << std::hex << std::setw(10) << std::setfill('0') 
            << mPhyAddr << std::endl;
  
  for (const auto &ent : mPageMapEnts) {
    int shAmt = mTransType == PD3 ? 47 : mTransType == PD2 ? 38 : 29;
    std::uint64_t virtAddrNew = virtAddr | (std::uint64_t)ent.first << shAmt;
    ent.second->printTrans(virtAddrNew);
  }
}


