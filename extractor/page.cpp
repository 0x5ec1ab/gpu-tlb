#include "page.h"

/*******************************************************************************
 *
 ******************************************************************************/
Page::Page(const std::uint8_t *ptr, std::uint64_t addr, TransType type, std::uint8_t flags) : Trans(ptr, addr, type, flags) {}

/*******************************************************************************
 *
 ******************************************************************************/
Page::~Page() {}

/*******************************************************************************
 *
 ******************************************************************************/
bool
Page::constructTrans()
{
  return true;
}

/*******************************************************************************
 *
 ******************************************************************************/
void
Page::printTrans(std::uint64_t virtAddr)
{
  std::string sizeStr = "2MB";
  std::uint64_t mask = 0x0FF;
  int shBits = 21;
  if (mTransType == LARGE)
  {
    sizeStr = "64KB";
    mask = 0x01F;
    shBits = 16;
  }
  else if (mTransType == SMALL)
  {
    sizeStr = "4KB";
    mask = 0x1FF;
    shBits = 12;
  }

  bool validFlag = mFlags & 0b01;
  PTEAperture aperture = (PTEAperture)(mFlags >> 1 & 0b11);
  bool volatileFlag = mFlags >> 3 & 0b01;
  bool encryptedFlag = mFlags >> 4 & 0b01;
  bool priviledgedFlag = mFlags >> 5 & 0b01;
  bool readOnlyFlag = mFlags >> 6 & 0b01;
  bool atomicDisableFlag = mFlags >> 7 & 0b01;

  std::cout << "\t\t\t\t\t" << std::dec << std::setw(3) << std::setfill(' ')
            << (virtAddr >> shBits & mask) << "-->" + sizeStr + "-Page@0x";
  std::cout << std::hex << std::setw(10) << std::setfill('0') << mPhyAddr << "\tVA: 0x";
  std::cout << std::hex << std::setw(10) << std::setfill('0') << virtAddr;
  std::cout << "\t|V:" << validFlag
            << "|AP:" << (aperture == APERTURE_VIDEO_LOCAL ? "VL" : aperture == APERTURE_VIDEO_PEER           ? "VP"
                                                                : aperture == APERTURE_SYSTEM_MEMORY_COHERENT ? "SC"
                                                                                                              : "SN")
            << "|VOL:" << volatileFlag
            << "|E:" << encryptedFlag
            << "|P:" << priviledgedFlag
            << "|RO:" << readOnlyFlag
            << "|AD:" << atomicDisableFlag
            << "|";
  std::cout << std::endl;
}
