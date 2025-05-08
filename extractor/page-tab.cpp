#include "page-tab.h"
#include "common.h"

/*******************************************************************************
 *
 ******************************************************************************/
PageTab::PageTab(MemDump &dump, std::uint64_t addr, TransType type, 
    TransType page) : Trans(dump, addr, type), mPageType(page)
{ }

/*******************************************************************************
 *
 ******************************************************************************/
PageTab::~PageTab()
{ 
  for (const auto &ent : mPageTabEnts)
    delete ent.second;
}

/*******************************************************************************
 *
 ******************************************************************************/
bool 
PageTab::constructTrans()
{
  // page table has 32 entries for 64KB pages and 512 entries for 4KB pages
  int entNum = mPageType == LARGE ? 32 : 512;
  
  for (int i = 0; i < entNum; ++i) {
    std::uint64_t offset = mPhyAddr + i * 8;
    
    std::uint64_t addr = 0;
    for (int j = 0; j < 5; ++j) {
      std::uint8_t byteVal = mMemDump.getByte(offset + j);
      addr |= byteVal << (j * 8);
    }
    addr &= 0x0000000FFFFFFFFF;
    addr >>= 8;
    
    std::uint8_t flagRaw = mMemDump.getByte(offset);
    std::uint8_t flag = flagRaw & 0x07;
    
    if (flag == 0x00 && addr == 0)
      continue;
    else if (flag != 0x01)
      return false;
    
    std::uint64_t nPhyAddr = addr << 12;
    nPhyAddr += vgpuOffset;
    
    Trans *next = new Page(mMemDump, nPhyAddr, mPageType, flagRaw);
    mPageTabEnts[i] = next;
  }
  
  if (mPageTabEnts.empty())
    return false;
  else
    return true;
}

/*******************************************************************************
 *
 ******************************************************************************/
void 
PageTab::printTrans(std::uint64_t virtAddr)
{ 
  std::cout << "\t\t\t\t" << std::dec << std::setw(3) << std::setfill(' ')
            << (virtAddr >> 21 & 0x0FF) << "-->PT@0x";
  std::cout << std::hex << std::setw(10) << std::setfill('0')
            << mPhyAddr << std::endl;
  
  for (const auto &ent : mPageTabEnts) {
    int shAmt = mPageType == LARGE ? 16 : 12;
    std::uint64_t virtAddrNew = virtAddr | (std::uint64_t)ent.first << shAmt;
    ent.second->printTrans(virtAddrNew);
  }
}

