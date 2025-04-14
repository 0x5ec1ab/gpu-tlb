#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include <nvml.h>
#include <nvtypes.h>
#include <uvm_linux_ioctl.h>

int 
main(int argc, char *argv[])
{
  int i;
  int opt = -1;
  int uvm_dev_fd = -1;
  int dev_num = 0;
  
  int dump_fd = -1;
  unsigned long dump_size = 0;
  unsigned long base_addr = 0;
  int gpu_instance_id = 0;
  char *dump_file = NULL;
  void *dump_ptr = NULL;
  
  nvmlReturn_t nvml_ret;
  nvmlDevice_t nvml_dev;
  nvmlDevice_t nvml_dev_mig;

  char nvml_uuid[NVML_DEVICE_UUID_V2_BUFFER_SIZE];
  const char *uuid_str = NULL;
  
  UVM_INITIALIZE_PARAMS       init_params = {0};
  UVM_REGISTER_GPU_PARAMS     reg_params = {0};
  UVM_DUMP_GPU_MEMORY_PARAMS  dump_params = {0};
  
  // parse arguments
  while ((opt = getopt(argc, argv, "b:d:o:s:g:")) != -1) {
    printf("%s\n", optarg);
    switch (opt) {
      case 'b':
        dump_size = strtoul(optarg, NULL, 0);
        break;
      case 'd':
        dev_num = atoi(optarg);
        break;
      case 'o':
        dump_file = optarg;
        break;
      case 's':
        base_addr = strtoul(optarg, NULL, 0);
        break;
      case 'g':
        gpu_instance_id = atoi(optarg);
        break;
    }
  }  
  
  if (dump_size == 0 || dump_file == NULL) {
    printf("Usage: %s [-d <dev>] [-s <addr>] -b <bytes> -o <file>\n", argv[0]);
    goto cleanup;
  }

  printf("[!] base_addr: 0x%lx, dump_size: 0x%lx\n", base_addr, dump_size);
  
  // get device and its UUID
  nvml_ret = nvmlInit();
  if (nvml_ret != NVML_SUCCESS) {
    printf("cannot initialize NVML: %s\n", nvmlErrorString(nvml_ret));
    goto cleanup;
  }
  
  nvml_ret = nvmlDeviceGetHandleByIndex(dev_num, &nvml_dev);
  if (nvml_ret != NVML_SUCCESS) {
    printf("cannot get device: %s\n", nvmlErrorString(nvml_ret));
    goto cleanup;
  }
  
  // It doesn't matter because the MIG UUID is different from the GPU UUID
  // if (gpu_instance_id != -1) {
  //     nvml_ret = nvmlDeviceGetMigDeviceHandleByIndex(nvml_dev, gpu_instance_id, &nvml_dev_mig);
  //     if (nvml_ret != NVML_SUCCESS) {
  //       printf("cannot get mig device: %s\n", nvmlErrorString(nvml_ret));
  //       goto cleanup;
  //     }
  //     nvml_ret = nvmlDeviceGetUUID(nvml_dev_mig, nvml_uuid, sizeof(nvml_uuid));
  //     if (nvml_ret != NVML_SUCCESS) {
  //       printf("cannot get mig device UUID: %s\n", nvmlErrorString(nvml_ret));
  //       goto cleanup;
  //     }
  // }
  nvml_ret = nvmlDeviceGetUUID(nvml_dev, nvml_uuid, sizeof(nvml_uuid));
  if (nvml_ret != NVML_SUCCESS) {
    printf("cannot get device UUID: %s\n", nvmlErrorString(nvml_ret));
    goto cleanup;
  }
  
  printf("[!] UUID: %s\n", nvml_uuid);

  uuid_str = nvml_uuid + 4;
  for (i = 0; i < 16; ++i) {
    sscanf(uuid_str, "%2hhx", &reg_params.gpu_uuid.uuid[i]);
    uuid_str += 2;
    if (*uuid_str == '-')
      ++uuid_str;
  }

    
  uvm_dev_fd = open("/dev/nvidia-uvm", O_RDWR);
  if (uvm_dev_fd < 0) {
    printf("cannot open /dev/nvidia-uvm file\n");
    printf("check if nvidia_uvm kernel module is loaded\n");
    goto cleanup;
  }
  
  if (ioctl(uvm_dev_fd, UVM_INITIALIZE, &init_params) < 0) {
    printf("cannot initialize uvm\n");
    goto cleanup;
  }
    
  if (ioctl(uvm_dev_fd, UVM_REGISTER_GPU, &reg_params) < 0) {
    printf("cannot register uvm\n");
    goto cleanup;
  }
  
  dump_fd = open(dump_file, O_CREAT | O_RDWR, 0644);
  if (dump_fd < 0) {
    printf("cannot dump to %s\n", dump_file);
    goto cleanup;
  }
  
  if (ftruncate(dump_fd, dump_size)) {
    printf("cannot dump with size %lu\n", dump_size);
    goto cleanup;
  }
  
  dump_ptr = mmap(NULL, dump_size, PROT_WRITE, MAP_SHARED, dump_fd, 0);
  if (dump_ptr == MAP_FAILED)  {
    printf("cannot get memory-mapped file\n");
    goto cleanup;
  }
  
  memcpy(&(dump_params.gpu_uuid), &(reg_params.gpu_uuid), 16);
  dump_params.child_id = gpu_instance_id;
  dump_params.base_addr = base_addr;
  dump_params.dump_size = dump_size;
  dump_params.out_addr = (unsigned long)dump_ptr;
  if (ioctl(uvm_dev_fd, UVM_DUMP_GPU_MEMORY, &dump_params) < 0) {
    printf("fail to dump GPU memory");
    goto cleanup;
  }
  
cleanup:
  munmap(dump_ptr, dump_size);
  
  if (dump_fd >= 0)
    close(dump_fd);
  
  if (uvm_dev_fd >= 0)
    close(uvm_dev_fd);
  
  nvmlShutdown();
}

