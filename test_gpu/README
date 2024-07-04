# GPU computing exploration

## Desc

Test CUDA usage for simple arithmetic operation.
Other framework are also tested (OpenACC, OpenCL).

## Cuda

### Documentation

- <https://docs.nvidia.com/cuda/cuda-c-best-practices-guide/index.html>
- <https://docs.nvidia.com/cuda/cuda-c-programming-guide/index.html>
- <https://docs.nvidia.com/cuda/cuda-compiler-driver-nvcc/index.html>
- <https://docs.nvidia.com/cuda/cuda-runtime-api/>
- <https://ulhpc-tutorials.readthedocs.io/en/latest/gpu/openacc/basics/>
- <https://github.com/jslee02/awesome-gpgpu>
- <https://github.com/phrb/intro-cuda/tree/master/src/cuda-samples/7_CUDALibraries>
- <https://developer.nvidia.com/blog/introduction-cuda-dynamic-parallelism/>

### Installation

```shell
$ lspci | egrep -i "vga|display|3d"

$ sudo lshw -c video

$ sudo apt update
$ sudo apt install mesa-utils
$ glxinfo -B

# For cuda
#<https://docs.nvidia.com/cuda/cuda-installation-guide-linux/index.html#recommended-post%5B/url%5D>

$ sudo apt-get --purge remove cuda-*nvidia-cuda-*
$ export PATH=${PATH}:/usr/local/cuda-12.4/bin
$ nvcc --version
```

### Example usage

```shell
$ mkdir build && cd build
$ cmake .. -DCMAKE_CUDA_COMPILER=/usr/local/cuda-12.4/bin/nvcc -DCMAKE_CUDA_ARCHITECTURES=61
$ make
$ ./main
$ sudo nvprof --unified-memory-profiling off ./main

# a compilation example without cmake:
$ nvcc main.cu -o main_cuda

# a compilation example with mixed cpp and cuda:
$ nvcc -c -o compute.o compute.cu 
$ g++ -o main compute.o main.cpp -I. -lcudart
```

### Debugging

## Debugging

```shell
$ export CUDA_ENABLE_COREDUMP_ON_EXCEPTION=1
$ export CUDA_ENABLE_CPU_COREDUMP_ON_EXCEPTION=0
$ export CUDA_COREDUMP_SHOW_PROGRESS=1
```

## OpenCL

### Installation

```shell
$ sudo apt install opencl-headers ocl-icd-opencl-dev -y
```

### Example usage

```shell
$ mkdir build && cd build
$ cmake .. -DBUILD_WITH_OPENCL=ON
$ make
$ ./main
```

## OpenAcc

NOTE THAT THE EXAMPLE SEEMS ONLY TO WORK WITH NVIDIA COMPILER.
WITH GCC, IT IS VERY SLOW.

### Installation

```shell
$ sudo apt install gcc-offload-nvptx
```

### Example usage

```shell
# With cmake and OpenACC, you need to specify the nvptx compiler (x86_64-linux-gnu-accel-nvptx-none-gcc)
$ mkdir build && cd build
$ cmake .. -DBUILD_WITH_OPENACC=ON
$ make
$ ./main

# compilation example without cmake
$ /usr/bin/g++ -o main -g -O -fopenacc -foffload=nvptx-none -fcf-protection=none -fno-stack-protector ../compute_acc.cpp ../main.cpp -I..

# openacc nvidia compiler
$ /opt/nvidia/hpc_sdk/Linux_x86_64/24.3/compilers/bin/pgc++ -acc -Minfo=all ../compute_acc.cpp ../main.cpp -I.. -o main
```
