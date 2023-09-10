#include <cuda.h>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

#define CHUNK0_SIZE (64L * 1024L * 1024L * 1024L * 1024L + 0x55554000000L)
#define CHUNK1_SIZE (41L * 1024L * 1024L * 1024L * 1024L + 0x0ffc8000000L)
#define STRIDE_SIZE (1L * 1024L * 1024L)

#define BASE_ADDR   0x700000000000
#define DUMMY_ADDR  0x7F0000000000

#define FILL_NUM    3000
#define WAIT_TIME   10000000000L // about 5 seconds on RTX3080

std::vector<int> l1_idx_vec = {
  0, 129, 258, 387,
  1, 128, 259, 386, 
  2, 131, 256, 385, 
  3, 130, 257, 384, 
};

// these numbers form an L2-uTLB eviction set for target 32 
std::vector<int> l2_idx_vec = {
  131628, 131757, 131886, 132015, 
  132128, 132257, 132386, 132515,
};

// these numbers form an L3-uTLB eviction set for target 32 (derived using l3-utlb-set)
std::vector<int> l3_idx_vec = {
  32607, 33185, 65246, 66338, 
  97373, 98979, 130524, 132644,
};

/*******************************************************************************
 * derive the L3-uTLB set selection hash function
 ******************************************************************************/
__global__ void 
loop(volatile uint64_t *chain, volatile uint64_t *evict, volatile uint64_t *fill, uint64_t x)
{
  uint64_t y = x;
  volatile uint64_t *ptr;
  volatile uint64_t *evt;
  uint64_t clk0 = 0;
  uint64_t clk1 = 0;
  
  for (ptr = (uint64_t *)fill[0]; ptr != fill; ptr = (uint64_t *)ptr[0])
    ++ptr[2];
  
  while (y == x) {
    ptr = chain;
    do {
      ++ptr[2];
      ptr = (uint64_t *)ptr[0];
      for (evt = (uint64_t *)evict[0]; evt != evict; evt = (uint64_t *)evt[0])
        ++evt[2];
    } while (ptr != chain);
    
    clk0 = clock64();
    clk1 = 0;
    while (clk1 < WAIT_TIME)
      clk1 = clock64() - clk0;
    
    y = chain[1];
  }
}

__global__ void
put(uint64_t *page, uint64_t x1, uint64_t x2)
{
  page[0] = x1;
  page[1] = x2;
}

int 
main(int argc, char *argv[])
{
  cudaDeviceReset();
  
  // hoard a large address space
  uint8_t *chunk0 = NULL;
  uint8_t *chunk1 = NULL;
  cudaMallocManaged(&chunk0, CHUNK0_SIZE);
  cudaMallocManaged(&chunk1, CHUNK1_SIZE);
  
  std::vector<uint64_t *> chain;
  int target = std::stoi(argv[1]);
  chain.push_back((uint64_t *)(BASE_ADDR + target * STRIDE_SIZE));
  for (auto i : l3_idx_vec)
    chain.push_back((uint64_t *)(BASE_ADDR + i * STRIDE_SIZE));
  std::vector<uint64_t *> evict;
  for (auto i : l1_idx_vec)
    evict.push_back((uint64_t *)(BASE_ADDR + i * STRIDE_SIZE));
  for (auto i : l2_idx_vec)
    evict.push_back((uint64_t *)(BASE_ADDR + i * STRIDE_SIZE));
  std::vector<uint64_t *> fill;
  for (int i = 1; i <= FILL_NUM; ++i)
    fill.push_back((uint64_t *)(DUMMY_ADDR + i * STRIDE_SIZE));
  uint64_t *dummy = (uint64_t *)DUMMY_ADDR;
  
  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // manipulate the last virtual address in chain
  int bit0 = argc >= 3 ? std::stoi(argv[2]) : -1;
  uint64_t mask0 = bit0 >= 0 ? 0x0000000000000001UL << bit0 : 0;
  int bit1 = argc == 4 ? std::stoi(argv[3]) : -1;
  uint64_t mask1 = bit1 >= 0 ? 0x0000000000000001UL << bit1 : 0;
  uint64_t addr = (uint64_t)chain.back();
  addr ^= mask0;
  addr ^= mask1;
  
  if (((uint8_t *)addr < chunk0 || (uint8_t *)addr >= chunk0 + CHUNK0_SIZE) && 
      ((uint8_t *)addr < chunk1 || (uint8_t *)addr >= chunk1 + CHUNK1_SIZE)) {
    std::cout << "out of scope: try another setting (e.g., BASE_ADDR or target)\n";
    return -1;
  }
  
  // an address in the set is generated
  for (size_t i = 0; i < chain.size() - 1; ++i) {
    if (addr == (uint64_t)chain[i])
      return 0;
  }
  
  for (auto temp : evict) {
    if (addr == (uint64_t)temp)
      while (1)
        ;
  }
  
  chain.back() = (uint64_t *)addr;
  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  
  
  // do dummy first to ensure its physical address unchanged when changing inputs 
  put<<<1, 1>>>(dummy, 0, 0);
  for (size_t i = 0; i < chain.size(); ++i)
    put<<<1, 1>>>(chain[i], (uint64_t)chain[(i + 1) % chain.size()], 0xdeadbeef);
  for (size_t i = 0; i < evict.size(); ++i)
    put<<<1, 1>>>(evict[i], (uint64_t)evict[(i + 1) % evict.size()], 0);
  for (size_t i = 0; i < fill.size(); ++i)
    put<<<1, 1>>>(fill[i], (uint64_t)fill[(i + 1) % fill.size()], 0);
  cudaDeviceSynchronize();
  
  loop<<<1, 1>>>(chain[0], evict[0], fill[0], 0xdeadbeef);
  cudaDeviceSynchronize();
  
  cudaFree(chunk0);
  cudaFree(chunk1);
}


