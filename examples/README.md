The examples given here are specifically tuned for running on the RTX 3080 GPU. If you are using other GPUs from the Turing or Ampere series, you may need to adjust the settings accordingly. Please don't hesitate to reach out if you encounter any issues. 

---

First of all, turn off the ASLR to make the process repeatable.

```
echo 0 | sudo tee /proc/sys/kernel/randomize_va_space
```

If you also want to permanently turn if off, write to a file under /etc/sysctl.d/.

```
echo "kernel.randomize_va_space=0" | sudo tee /etc/sysctl.d/01-aslr.conf
```

---

Compile the examples:

```
make
```

- Example 1: fill.cu tests if referencing a number of 64KB pages can flush the TLB hierarchy. The number of pages is specified by
  
  #define PAGE_NUM    4000
  To run the example:
  
  ```
  ./run_fill.py
  ```

- Example 2: l1-dtlb-cap.cu finds how many entries in the L1-dTLB. The number of pages is specified by: 
  
  #define PAGE0_NUM   17
  
  To run the example:
  
  ```
  ./run_l1-dtlb-cap.py
  ```
  
  Accessing 17 (or more) data pages can achieve eviction, since it has 16 entries.

- Example 3: l1-itlb-cap.cu finds how many entries in the L1-iTLB. The number of pages is specified by:
  
  #define PAGE0_NUM   16
  
  To run the example:
  
  ```
  ./run_l1-itlb-cap.py
  ```
  
  L1-iTLB also has 16 entries. Since the code page containing the loop kernel needs to be accounted for as well, accessing 16 fabricated code pages can achieve eviction in this case.

- Example 4: l2-utlb-set.cu derives an L2-uTLB eviction set for a targeted translation.
  
  To run the example:
  
  ```
  ./run_l2-utlb-set.py
  ```

- Example 5: l2-utlb-hash.cu derives the L2-uTLB set selection hash function.
  
  To run the example:
  
  ```
  ./run_l2-utlb-hash.py
  ```

- Example 6: l3-utlb-set.cu derives an L3-uTLB eviction set for a targeted translation.
  
  To run the example:
  
  ```
  ./run_l3-utlb-set.py
  ```

- Example 7: l3-utlb-hash.cu derives the L3-uTLB set selection hash function.
  
  To run the example:
  
  ```
  ./run_l2-utlb-hash.py
  ```
