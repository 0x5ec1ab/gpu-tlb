This GPU memory dumper works on **Ubuntu 20.04**.

---

First, download the driver installer (version 515.76):

```
wget https://us.download.nvidia.com/XFree86/Linux-x86_64/515.76/NVIDIA-Linux-x86_64-515.76.run 
```

This will download the installer named `NVIDIA-Linux-x86_64-515.76.run` to your current directory.

Then, make the installer executable:

```
chmod +x NVIDIA-Linux-x86_64-515.76.run
```

Run the following command to extract files from the executable:

```
./NVIDIA-Linux-x86_64-515.76.run -x
```

There will be a new directory named `NVIDIA-Linux-x86_64-515.76` under your current working directory. 

Go into `NVIDIA-Linux-x86_64-515.76`:

```
cd NVIDIA-Linux-x86_64-515.76/
```

Apply the patch:

```
patch -p1 < path_to_dumper/patch/driver-515.76.patch
```

Install the patched driver:

```
sudo ./nvidia-installer
```

---

After the modified driver is installed, under the mem-dumper directory, compile the dumper:

```
make
```

*(If nvml.h cannot be found, please also install CUDA 11/12.)*

Now, you can use our GPU memory dumper to dump GPU memory. For example, the following dumps the first 1GB memory of GPU device 0 to a file named xyz:

```
./dumper -d 0 -s 0 -b 0xf0000000 -o xyz
```

- -d specifies which GPU device. Normally, 0 is the first GPU installed on your machine, 1 is the second GPU (if there is one), and so forth. If not specified, the default is 0.

- -s specifies the GPU memory address from which the dumping starts. Note that this address is a physical address. If not specified, the default is 0.

- -b specifies the number of bytes to dump. 

- -o specifies the file to which the GPU memory is dumped. 
