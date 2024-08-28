ifndef CUDA_PATH
	CUDA_PATH := /usr/local/cuda
endif

ifndef NVIDIA_DRIVER_PATH
	NVIDIA_DRIVER_PATH := /usr/src/nvidia-515.76
endif

INCLUDES := -I${CUDA_PATH}/include
INCLUDES += -I${NVIDIA_DRIVER_PATH}/kernel/common/inc
INCLUDES += -I${NVIDIA_DRIVER_PATH}/kernel/nvidia
INCLUDES += -I${NVIDIA_DRIVER_PATH}/kernel/nvidia-uvm

all: 
	gcc $(INCLUDES) -o dumper dumper.c -lnvidia-ml

