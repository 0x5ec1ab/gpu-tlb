Compile the PTE modifier:

```
make
```

Now, you can use this tool to modify the GPU page table of the running GPU program:

```
./modifier <PTE address> <new PTE value>
```

Refer to pagemap.py under the examples directory to see how to find the PTE address of interest and its format. 
