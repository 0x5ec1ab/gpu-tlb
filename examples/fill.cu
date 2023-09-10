#include <cuda.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CHUNK0_SIZE (64L * 1024L * 1024L * 1024L * 1024L + 0x55554000000L)
#define CHUNK1_SIZE (41L * 1024L * 1024L * 1024L * 1024L + 0x0ffc8000000L)
#define STRIDE_SIZE (1L * 1024L * 1024L)

#define BASE_ADDR   0x700000000000
#define DUMMY_ADDR  0x7F0000000000

#define PAGE_NUM    4000
//#define PAGE_NUM    2000
#define WAIT_TIME   10000000000L // about 5 seconds on RTX3080

__global__ void 
loop(volatile uint64_t *page, uint64_t x)
{
  uint64_t y = x;
  volatile uint64_t *ptr;
  uint64_t clk0;
  uint64_t clk1;
  
  while (y == x) {
    for (ptr = (uint64_t *)page[0]; ptr != page; ptr = (uint64_t *)ptr[0])
      ++ptr[2];
    
    clk0 = clock64();
    clk1 = 0;
    while (clk1 < WAIT_TIME)
      clk1 = clock64() - clk0;
    
    y = ptr[1];
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
  uint8_t *chunk0 = NULL;
  uint8_t *chunk1 = NULL;
  uint8_t *base = NULL;
  uint64_t *list[PAGE_NUM];
  uint64_t *dummy = NULL;
  
  cudaDeviceReset();
  
  // hoard a large address space
  cudaMallocManaged(&chunk0, CHUNK0_SIZE);
  cudaMallocManaged(&chunk1, CHUNK1_SIZE);
  
  base = (uint8_t *)BASE_ADDR;
  for (int i = 0; i < PAGE_NUM; ++i)
    list[i] = (uint64_t *)(base + i * STRIDE_SIZE);
  dummy = (uint64_t *)DUMMY_ADDR;
  
  for (int i = 0; i < PAGE_NUM; ++i)
    put<<<1, 1>>>(list[i], (uint64_t)list[(i + 1) % PAGE_NUM], 0xdeadbeef);
  put<<<1, 1>>>(dummy, 0, 0);
  cudaDeviceSynchronize();
  
  loop<<<1, 1>>>(list[0], 0xdeadbeef);
  cudaDeviceSynchronize();
  
  cudaFree(chunk0);
  cudaFree(chunk1);
}


