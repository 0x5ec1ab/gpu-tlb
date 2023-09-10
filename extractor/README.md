Compile the extractor:

```
make
```

Now, you can use this tool to extract the GPU page tables from a GPU memory dump named "xyz":

```
./extractor xyz
```

Here's an example of its output:

```
PD3@0x0000201000
    0-->PD2@0x0000202000
        0-->PD1@0x0000203000
            16-->PD0@0x0000204000
                     0-->2MB-Page@0x0000400000     VA: 0x200000000
                     2-->2MB-Page@0x0000c00000     VA: 0x200400000
                    27-->2MB-Page@0x027b400000     VA: 0x203600000
                    28-->2MB-Page@0x027b600000     VA: 0x203800000
                    29-->2MB-Page@0x027b800000     VA: 0x203a00000
                    30-->2MB-Page@0x027ba00000     VA: 0x203c00000
        
        ...
        
        448-->PD1@0x0000237000
             0-->PD0@0x0000238000
                16-->PT@0x0000224200
                     0-->64KB-Page@0x000d800000    VA: 0x700002000000
                    16-->64KB-Page@0x000d900000    VA: 0x700002100000
                17-->PT@0x00002bfb00
                     0-->64KB-Page@0x0108a00000    VA: 0x700002200000
                    16-->64KB-Page@0x0108b00000    VA: 0x700002300000
        ...
```

In this example, it shows that a PD3 is at physical address 0x0000201000, and its entry 0 gives a PD2 at 0x0000202000. The entry 0 of the PD2 points to a PD1 at 0x0000203000 and the entry 16 of the PD1 points to a PD0 at 0x0000204000. The entry 0 of the PD0 gives the translation for the virtual address 0x200000000, which is mapped to a 2MB page frame whose physical address is 0x0000400000, and the entry 2 of the PD0 translates 0x200400000 to a 2MB page frame at 0x0000c00000, etc. 

The entry 448 of the PD2 gives another PD1 (at 0x0000237000) whose entry 0 points to a PD0 at 0x0000238000. The entry 16 of the PD0 points to a PT at 0x0000224200. The entry 0 of the PT has the virtual address 0x700002000000 mapped to the physical address 0x000d800000 (a 64KB page in this case)...
