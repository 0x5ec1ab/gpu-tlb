#include "page-map.h"

/*******************************************************************************
 *
 ******************************************************************************/
PageMap::PageMap(MemDump &dump, std::uint64_t addr, TransType type) : 
    Trans(dump, addr, type)
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
    std::uint64_t offset = mPhyAddr + i * entSize;
    
    // a valid entry has zeros for most bytes
    for (int j = 5; j < entSize; ++j) {
      std::uint8_t byteVal = mMemDump.getByte(offset + j);
      if (byteVal != 0)
        continue;
    }
    
    std::uint64_t addr = 0;
    for (int j = 0; j < 5; ++j) {
      std::uint8_t byteVal = mMemDump.getByte(offset + j);
      addr |= byteVal << (j * 8);
    }
    addr &= 0x0000000FFFFFFFFF;
    addr >>= 8;
    
    std::uint8_t flag = mMemDump.getByte(offset);
    flag &= 0x07;
    
    if (flag == 0x00 && addr == 0)
      continue;
    else if (flag != 0x02)
      continue;
    
    // construct the next-level trans
    TransType nTransType = mTransType == PD3 ? PD2 : 
                           mTransType == PD2 ? PD1 : PD0;
    std::uint64_t nPhyAddr = addr << 12;
    
    Trans *next = nullptr;
    if (nTransType == PD0)
      next = new PageDir(mMemDump, nPhyAddr, nTransType);
    else
      next = new PageMap(mMemDump, nPhyAddr, nTransType);
    bool ok = next->constructTrans();
    if (!ok) {
      delete next;
      continue;
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


