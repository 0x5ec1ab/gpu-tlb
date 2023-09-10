#include "page-dir.h"

/*******************************************************************************
 *
 ******************************************************************************/
PageDir::PageDir(const std::uint8_t *ptr, std::uint64_t addr, TransType type) : 
    Trans(ptr, addr, type)
{ }

/*******************************************************************************
 *
 ******************************************************************************/
PageDir::~PageDir()
{ 
  for (const auto &ent : mPageDirEnts) {
    delete ent.second.first;
    delete ent.second.second;
  }
}

/*******************************************************************************
 *
 ******************************************************************************/
bool 
PageDir::constructTrans()
{
  for (int i = 0; i < 256; ++i) {
    const std::uint8_t *entLargePtr = mDumpPtr + i * 16;
    const std::uint8_t *entSmallPtr = entLargePtr + 8;
    
    std::uint64_t addrLarge = 0;
    std::uint64_t addrSmall = 0;
    for (int j = 0; j < 5; ++j) {
      addrLarge |= entLargePtr[j] << (j * 8);
      addrSmall |= entSmallPtr[j] << (j * 8);
    }
    addrLarge &= 0x0000000FFFFFFFFF;
    addrSmall &= 0x0000000FFFFFFFFF;
    addrLarge >>= 4;
    addrSmall >>= 8;
    
    uint8_t flagLarge = *entLargePtr & 0x07;
    uint8_t flagSmall = *entSmallPtr & 0x07;
    
    /*
    if (flagSmall == 0x00 && flagLarge == 0x00) 
      continue;
    else if (flagSmall == 0x00 && flagLarge != 0x02)
      return false;
    else if (flagSmall != 0x02 && flagLarge == 0x00)
      return false;
    */
    
    // construct HUGE pages
    if (flagSmall == 0x00 && flagLarge == 0x01) {
      std::uint64_t addrHuge = addrLarge >> 4;
      std::uint64_t nPhyAddr = addrHuge << 12;
      std::int64_t offset = nPhyAddr - mPhyAddr;
      const std::uint8_t *nDumpPtr = mDumpPtr + offset;
      
      Trans *next = new Page(nDumpPtr, nPhyAddr, HUGE);
      mPageDirEnts[i] = std::make_pair(nullptr, next);
      continue;
    }
    
    Trans *nextSmall = nullptr;
    Trans *nextLarge = nullptr;
    
    // construct PT for SMALL pages 
    if (flagSmall == 0x02) {      
      std::uint64_t nPhyAddr = addrSmall << 12;
      std::int64_t offset = nPhyAddr - mPhyAddr;
      const std::uint8_t *nDumpPtr = mDumpPtr + offset;
      
      nextSmall = new PageTab(nDumpPtr, nPhyAddr, PT, SMALL);
      bool ok = nextSmall->constructTrans();
      if (!ok) {
        delete nextSmall;
        // a valid PD0 may have an invalid PT so no false returned
        nextSmall = nullptr;
      }
    } 
    
    // construct PT for LARGE pages 
    if (flagLarge == 0x02) {
      std::uint64_t nPhyAddr = addrLarge << 8;
      std::int64_t offset = nPhyAddr - mPhyAddr;
      const std::uint8_t *nDumpPtr = mDumpPtr + offset;
      
      nextLarge = new PageTab(nDumpPtr, nPhyAddr, PT, LARGE);
      bool ok = nextLarge->constructTrans();
      if (!ok) {
        delete nextLarge;
        // a valid PD0 may have an invalid PT so no false returned
        nextLarge = nullptr;
      }
    }
    
    if (nextSmall == nullptr && nextLarge == nullptr)
      continue;
    else
      mPageDirEnts[i] = std::make_pair(nextSmall, nextLarge);
  }
  
  if (mPageDirEnts.empty())
    return false;
  else
    return true;
}

/*******************************************************************************
 *
 ******************************************************************************/
void 
PageDir::printTrans(std::uint64_t virtAddr)
{
  std::cout << "\t\t\t" << std::dec << std::setw(3) << std::setfill(' ')
            << (virtAddr >> 29 & 0x1FF) << "-->PD0@0x";
  std::cout << std::hex << std::setw(10) << std::setfill('0');
  std::cout << mPhyAddr << std::endl;
  
  for (const auto &ent : mPageDirEnts) {
    std::uint64_t virtAddrNew = virtAddr | (std::uint64_t)ent.first << 21;
    if (ent.second.first != nullptr)
      ent.second.first->printTrans(virtAddrNew);
    if (ent.second.second != nullptr)
      ent.second.second->printTrans(virtAddrNew);
  }
}

 
