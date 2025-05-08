#include "page-dir.h"
#include "common.h"

/*******************************************************************************
 *
 ******************************************************************************/
PageDir::PageDir(MemDump &dump, std::uint64_t addr, TransType type) : 
    Trans(dump, addr, type)
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
    std::uint64_t offsetLarge = mPhyAddr + i * 16;
    std::uint64_t offsetSmall = offsetLarge + 8;
    
    std::uint64_t addrLarge = 0;
    std::uint64_t addrSmall = 0;
    for (int j = 0; j < 5; ++j) {
      std::uint8_t byteValLarge = mMemDump.getByte(offsetLarge + j);
      addrLarge |= byteValLarge << (j * 8);
      std::uint8_t byteValSmall = mMemDump.getByte(offsetSmall + j);
      addrSmall |= byteValSmall << (j * 8);
    }
    addrLarge &= 0x0000000FFFFFFFFF;
    addrSmall &= 0x0000000FFFFFFFFF;
    addrLarge >>= 4;
    addrSmall >>= 8;
    
    std::uint8_t flagLarge = mMemDump.getByte(offsetLarge);
    flagLarge &= 0x07;
    std::uint8_t flagSmall = mMemDump.getByte(offsetSmall);
    flagSmall &= 0x07;
    
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
      nPhyAddr += vgpuOffset;
      
      std::uint8_t flagHuge = mMemDump.getByte(offsetLarge);
      Trans *next = new Page(mMemDump, nPhyAddr, HUGE, flagHuge);
      mPageDirEnts[i] = std::make_pair(nullptr, next);
      continue;
    }
    
    Trans *nextSmall = nullptr;
    Trans *nextLarge = nullptr;
    
    // construct PT for SMALL pages 
    if (flagSmall == 0x02) {      
      std::uint64_t nPhyAddr = addrSmall << 12;      
      nPhyAddr += vgpuOffset;
      nextSmall = new PageTab(mMemDump, nPhyAddr, PT, SMALL);
      bool ok = nextSmall->constructTrans();
      if (!ok) {
        delete nextSmall;
        nextSmall = nullptr;
      }
    }
    
    // construct PT for LARGE pages 
    if (flagLarge == 0x02) {
      std::uint64_t nPhyAddr = addrLarge << 8;      
      nPhyAddr += vgpuOffset;
      nextLarge = new PageTab(mMemDump, nPhyAddr, PT, LARGE);
      bool ok = nextLarge->constructTrans();
      if (!ok) {
        delete nextLarge;
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

 
