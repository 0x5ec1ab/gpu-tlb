#include <assert.h>
#include <cuda.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CHUNK0_SIZE (64L * 1024L * 1024L * 1024L * 1024L + 0x55554000000L)
#define CHUNK1_SIZE (41L * 1024L * 1024L * 1024L * 1024L + 0x0ffc8000000L)
#define STRIDE_SIZE (1L * 1024L * 1024L)

#define BASE_ADDR   0x700000000000
#define DUMMY_ADDR  0x7F0000000000

#define PAGE0_NUM   1500
#define PAGE1_NUM   4000
#define PAGE2_NUM   16
#define WAIT_TIME   10000000000L // about 5 seconds on RTX3080

#define BLK_NUM     100
#define SHARED_MEM  (96 * 1024)
#define SMID0       0
#define SMID1       3 // IMPORTANT: SM0 and SM3 are in different GPCs on RTX3080

// derived indices that goes into the same L2-uTLB set on RTX3080
int idx[] = {32, 161, 290, 419, 548, 677, 806, 935, 1064};

/*******************************************************************************
 * derive the L2-uTLB set selection function
 ******************************************************************************/
__global__ void 
loop(volatile uint64_t *page0, volatile uint64_t *page1, volatile uint64_t *page2, uint64_t x)
{
  uint64_t y = x;
  volatile uint64_t *ptr;
  volatile uint64_t *evt;
  uint64_t clk0 = 0;
  uint64_t clk1 = 0;
  uint32_t smid;
  
  asm("mov.u32 %0, %%smid;" : "=r" (smid));
  if (smid != SMID0 && smid != SMID1)
    return;
  
  if (smid == SMID0) {
    while (y == x) {
      for (ptr = (uint64_t *)page0[0]; ptr != page0; ptr = (uint64_t *)ptr[0]) {
        // access L1-dTLB eviction set
        for (evt = (uint64_t *)page2[0]; evt != page2; evt = (uint64_t *)evt[0])
          ++evt[2];
        ++ptr[2];
      }
      
      clk0 = clock64();
      clk1 = 0;
      while (clk1 < WAIT_TIME)
        clk1 = clock64() - clk0;
            
      y = ptr[1];
    }
  } else if (smid == SMID1) {
    while (y == x) {
      for (ptr = (uint64_t *)page1[0]; ptr != page1; ptr = (uint64_t *)ptr[0])
        ++ptr[2];
      
      y = ptr[1];
    }
  } 
  
  page0[1] = 0;
  page1[1] = 0;
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
  uint8_t *chunk0 = NULL;
  uint8_t *chunk1 = NULL;
  uint8_t *base = NULL;
  uint64_t *list0[PAGE0_NUM];
  uint64_t *list1[PAGE1_NUM];
  uint64_t *list2[PAGE2_NUM];
  uint64_t *chain[sizeof(idx) / sizeof(int)];
  int num = 0;
  uint64_t *dummy = NULL;
  int bit0 = -1;
  int bit1 = -1;
  uint64_t mask0 = 0;
  uint64_t mask1 = 0;
  uint64_t addr = 0;
  
  cudaDeviceReset();
  cudaFuncSetAttribute(loop, cudaFuncAttributeMaxDynamicSharedMemorySize, SHARED_MEM);
  
  // hoard a large address space
  cudaMallocManaged(&chunk0, CHUNK0_SIZE);
  cudaMallocManaged(&chunk1, CHUNK1_SIZE);
  
  base = (uint8_t *)BASE_ADDR;
  for (int i = 0; i < PAGE0_NUM; ++i)
    list0[i] = (uint64_t *)(base + i * STRIDE_SIZE);
  base += PAGE0_NUM * STRIDE_SIZE;
  for (int i = 0; i < PAGE1_NUM; ++i)
    list1[i] = (uint64_t *)(base + i * STRIDE_SIZE);
  for (int i = 0; i < PAGE2_NUM; ++i)
    list2[i] = list0[idx[0] + i + 1];
  num = sizeof(idx) / sizeof(int);
  for (int i = 0; i < num; ++i)
    chain[i] = list0[idx[i]];
  dummy = (uint64_t *)DUMMY_ADDR;
  
  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // manipulate the last virtual address in chain
  bit0 = argc >= 2 ? atoi(argv[1]) : -1;
  mask0 = bit0 >= 0 ? 0x0000000000000001UL << bit0 : 0;
  bit1 = argc == 3 ? atoi(argv[2]) : -1;
  mask1 = bit1 >= 0 ? 0x0000000000000001UL << bit1 : 0;
  addr = (uint64_t)chain[num - 1];
  addr ^= mask0;
  addr ^= mask1;
  
  if (((uint8_t *)addr < chunk0 || (uint8_t *)addr >= chunk0 + CHUNK0_SIZE) && 
      ((uint8_t *)addr < chunk1 || (uint8_t *)addr >= chunk1 + CHUNK1_SIZE)) {
    printf("out of scope: try another setting (e.g., BASE_ADDR or target)\n");
    return -1;
  }
  
  // an address in the set is generated
  for (int i = 0; i < num - 1; ++i) {
    if (addr == (uint64_t)chain[i])
      return 0;
  }
  
  for (int i = 0; i < PAGE1_NUM; ++i) {
    if (addr == (uint64_t)list1[i]) {
      list1[i] = chain[num - 1];
      break;
    }
  }

  for (int i = 0; i < PAGE2_NUM; ++i) {
    if (addr == (uint64_t)list2[i]) {
      list2[i] = list0[idx[0] + PAGE2_NUM + 1];
      break;
    }
  }
  
  chain[num - 1] = (uint64_t *)addr;
  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  for (int i = 0; i < num; ++i)
    put<<<1, 1>>>(chain[i], (uint64_t)chain[(i + 1) % num], 0xdeadbeef);
  for (int i = 0; i < PAGE1_NUM; ++i)
    put<<<1, 1>>>(list1[i], (uint64_t)list1[(i + 1) % PAGE1_NUM], 0xdeadbeef);
  for (int i = 0; i < PAGE2_NUM; ++i)
    put<<<1, 1>>>(list2[i], (uint64_t)list2[(i + 1) % PAGE2_NUM], 0xdeadbeef);
  put<<<1, 1>>>(dummy, 0, 0);
  cudaDeviceSynchronize();
    
  loop<<<BLK_NUM, 1, SHARED_MEM>>>(chain[0], list1[0], list2[0], 0xdeadbeef);
  cudaDeviceSynchronize();
  
  cudaFree(chunk0);
  cudaFree(chunk1);
}


