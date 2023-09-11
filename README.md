# gpu-tlb 
This repo contains the tools described in our paper "**TunneLs for Bootlegging: Fully Reverse-Engineering GPU TLBs for Challenging Isolation Guarantees of NVIDIA MIG**". 
- dumper: This utility tool can dump the memory of GPUs from the Ampere series and earlier. 
- extractor: This utility tool can extract the GPU page tables from a given GPU memory dump. 
- modifier: This tool is used to update the PTE values in GPU memory. 
- examples: We provide several examples illustrating how to use our tools to explore GPU TLB properties.

### bibtex entry
```
@inproceedings{Zhang:2023:CCS,
  author    = {Zhenkai Zhang and Tyler Allen and Fan Yao and Xing Gao and Rong Ge},
  title     = {{TunneLs for Bootlegging: Fully Reverse-Engineering GPU TLBs for Challenging Isolation Guarantees of NVIDIA MIG}},
  year      = {2023},
  booktitle = {Proceedings of the 2023 ACM SIGSAC Conference on Computer and Communications Security},
  series    = {CCS '23},
}
```
