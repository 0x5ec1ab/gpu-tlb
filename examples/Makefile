NVCC = nvcc
FLAGS = -arch=sm_80 -Xptxas -O0

FILES = fill.cu l1-dtlb-cap.cu l1-itlb-cap.cu l2-utlb-set.cu l2-utlb-hash.cu l3-utlb-set.cu l3-utlb-hash.cu 
BINS = $(FILES:.cu=)

all: $(BINS)

%: %.cu
	$(NVCC) $(FLAGS) $< -o $@

clean:
	rm -f $(BINS)

.PHONY: all clean

