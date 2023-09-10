INCLUDES := -I/usr/local/cuda/include
INCLUDES += -I/usr/src/nvidia-515.76/common/inc
INCLUDES += -I/usr/src/nvidia-515.76/nvidia
INCLUDES += -I/usr/src/nvidia-515.76/nvidia-uvm

all: 
	gcc $(INCLUDES) -o dumper dumper.c -lnvidia-ml

