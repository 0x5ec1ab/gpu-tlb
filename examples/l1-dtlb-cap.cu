#include <cuda.h>
#include <stdint.h>
#include <stdio.h>

#define CHUNK0_SIZE (64L * 1024L * 1024L * 1024L * 1024L + 0x55554000000L)
#define CHUNK1_SIZE (41L * 1024L * 1024L * 1024L * 1024L + 0x0ffc8000000L)
#define STRIDE_SIZE (1L * 1024L * 1024L)

#define BASE_ADDR   0x700000000000
#define DUMMY_ADDR  0x7F0000000000

#define PAGE0_NUM   17
#define PAGE1_NUM   4000
#define WAIT_TIME   10000000000L // about 5 seconds on RTX3080

#define BLK_NUM     100
#define SHARED_MEM  (96 * 1024)
#define SMID0       0
#define SMID1       12 // IMPORTANT: SM0 and SM12 are in the same GPC on RTX3080

__global__ void 
loop(volatile uint64_t *page0, volatile uint64_t *page1, uint64_t x)
{
  uint64_t y = x;
  volatile uint64_t *ptr = NULL;
  uint64_t clk0 = 0;
  uint64_t clk1 = 0;
  uint32_t smid;
  
  asm("mov.u32 %0, %%smid;" : "=r" (smid));
  if (smid != SMID0 && smid != SMID1)
    return;
  
  if (smid == SMID0) {
    while (y == x) {
      for (ptr = (uint64_t *)page0[0]; ptr != page0; ptr = (uint64_t *)ptr[0])
        ++ptr[2];
      
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
  uint64_t *dummy = NULL;
  
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
  dummy = (uint64_t *)DUMMY_ADDR;
  
  for (int i = 0; i < PAGE0_NUM; ++i)
    put<<<1, 1>>>(list0[i], (uint64_t)list0[(i + 1) % PAGE0_NUM], 0xdeadbeef);
  for (int i = 0; i < PAGE1_NUM; ++i)
    put<<<1, 1>>>(list1[i], (uint64_t)list1[(i + 1) % PAGE1_NUM], 0xdeadbeef);
  put<<<1, 1>>>(dummy, 0, 0);
  cudaDeviceSynchronize();
  
  loop<<<BLK_NUM, 1, SHARED_MEM>>>(list0[0], list1[0], 0xdeadbeef);
  cudaDeviceSynchronize();
  
  cudaFree(chunk0);
  cudaFree(chunk1);
}


