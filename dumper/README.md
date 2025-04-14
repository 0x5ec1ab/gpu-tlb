This GPU memory dumper works on **Ubuntu 20.04**.

---

First, chose a supported version and set the environment variable accordingly:

- `export NV_DRV_VERSION=515.76`
- `export NV_DRV_VERSION=535.113`
- `export NV_DRV_VERSION=555.58.02`
- `export NV_DRV_VERSION=560.35.03`
- `export NV_DRV_VERSION=570.133.07; export NV_KERNEL_OPEN=1` (The patch for 570.133.07 works on the open-kernel version)

> If the version you want to use is not supported, you can try to apply the closest patch, but this might not work if one of the files to be patched has been modified.
> In that case, you should patch it manually by just adding the functions and the definitions you can find in one of the patches.
> Unless NVIDIA pushes a breaking change in the UVM APIs, this patch should be easibily applicable to all versions.
> If you manage to patch a different version of the driver, **you are welcome to create a pull request** containing the new patch file and the updated list of versions in this readme.

Then, download the driver installer:

```
wget https://us.download.nvidia.com/XFree86/Linux-x86_64/$NV_DRV_VERSION/NVIDIA-Linux-x86_64-$NV_DRV_VERSION.run 
```

This will download the installer named `NVIDIA-Linux-x86_64-$NV_DRV_VERSION.run` to your current directory.

Then, make the installer executable:

```
chmod +x NVIDIA-Linux-x86_64-$NV_DRV_VERSION.run
```

Run the following command to extract files from the executable:

```
./NVIDIA-Linux-x86_64-$NV_DRV_VERSION.run -x
```

There will be a new directory named `NVIDIA-Linux-x86_64-$NV_DRV_VERSION` under your current working directory. 

Go into `NVIDIA-Linux-x86_64-$NV_DRV_VERSION`:

```
cd NVIDIA-Linux-x86_64-$NV_DRV_VERSION/
```

Apply the patch:

```
patch -p1 < path_to_dumper/patch/driver-$NV_DRV_VERSION.patch
```

Install the patched driver:

```
sudo ./nvidia-installer
```

---

After the modified driver is installed, under the mem-dumper directory, compile the dumper:

```bash
export CUDA_PATH=/path/to/cuda # if not already defined, such as /usr/local/cuda
export NVIDIA_DRIVER_PATH=/path/to/the/patched/driver
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
