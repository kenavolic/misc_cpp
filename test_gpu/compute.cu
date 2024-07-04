#include <cuda_profiler_api.h>
#include <thrust/host_vector.h>
#include <thrust/device_vector.h>
#include <thrust/sequence.h>
#include <cublas_v2.h>

#include <iostream>
#include <cmath>
#include <algorithm>

#if __CUDA_ARCH__ == 500

#warning "__CUDA_ARCH__ 500"

#endif

// - https://docs.nvidia.com/cuda/cuda-c-best-practices-guide/index.html
// - https://docs.nvidia.com/cuda/cuda-c-programming-guide/index.html
// - https://docs.nvidia.com/cuda/cuda-compiler-driver-nvcc/index.html
// - https://docs.nvidia.com/cuda/cuda-runtime-api/ 


// __global__ Define a CUDA kernel function must return void
// called from host code and executed on device
// __device_ Called and executed from device
//
// The kernel function is executed by N threads in parallel
//
__global__
void add(uint64_t size, float* arr1, float* arr2) { 
  int row = threadIdx.y + blockIdx.y * blockDim.y;
  int col = threadIdx.x + blockIdx.x * blockDim.x;

  if (row < size && col < size) {
    arr1[row*size + col] = arr1[row*size + col] + arr2[row*size + col];
  }
}

__global__
void mul(uint64_t size, float* arr1, float* arr2, float* out) {
  int row = threadIdx.y + blockIdx.y * blockDim.y;
  int col = threadIdx.x + blockIdx.x * blockDim.x;

  if (row < size && col < size) {
    float res{};
    for (uint64_t s = 0; s < size; s++) {
      res += arr1[row*size + s] * arr2[s*size + col];
    }
    out[row*size+col] = res;
  }
}

__global__
void mul_tile(uint64_t size, float* arr1, float* arr2, float* out) {
  int realRow = threadIdx.y + blockIdx.y * blockDim.y; 
  int realCol = threadIdx.x + blockIdx.x * blockDim.x;

  if (realRow > size || realCol > size) {
    return;
  }

  int row = threadIdx.y;
  int col = threadIdx.x;

  float Cvalue = 0;
  // for each sublocks, load Asubi and Bsubi to shared memory
  // compute multiplication
  for (uint64_t sub = 0; sub < size / 16U; sub++) {
    // load Asubi et bsubi
    __shared__ float Asubi[16][16];
    __shared__ float Bsubi[16][16];
    Asubi[row][col] = arr1[realRow*size+col+sub*16];
    Bsubi[row][col] = arr2[(row+sub*16)*size+realCol];
    __syncthreads();
    for (uint64_t e = 0; e < 16; e++) {
      Cvalue += Asubi[row][e] * Bsubi[e][col];
    }
    __syncthreads();
  }
  out[realRow*size+realCol] = Cvalue;
}

void mul_blas(const int size, const float *A, const float *B, float *C) {
     int lda=size,ldb=size,ldc=size;
     const float alf = 1;
     const float bet = 0;
     const float *alpha = &alf;
     const float *beta = &bet;
 
     // Create a handle for CUBLAS
     cublasHandle_t handle;
     auto res = cublasCreate(&handle);

     if (res != CUBLAS_STATUS_SUCCESS) {
      std::cout << "cublas handle error " << res << std::endl;
     }
 
     // Do the actual multiplication
     // https://stackoverflow.com/questions/56043539/cublassgemm-row-major-multiplication
     // for the row major operation
     cublasSgemm(handle, CUBLAS_OP_N, CUBLAS_OP_N, size, size, size, alpha, B, lda, A, ldb, beta, C, ldc);
 
     // Destroy the handle
     cublasDestroy(handle);
}

void compute(float*a, float*b, float*c, size_t count, bool useLib = false) {
  const uint64_t kCount = count;

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////// Initialization
  /////////////////////////////////////////////////////////////////////////////

  // check config before
  // cudaGetDeviceCount
  // SM Version
  int devicesCount{-666};
  auto cudaStatus = cudaGetDeviceCount(&devicesCount);
  
  if (cudaStatus != cudaSuccess) {
      std::cerr << "Failed to get device count with error " << static_cast<int>(cudaStatus) << std::endl;
      return;
  }

  std::cout << "device count: " << devicesCount << std::endl;

  cudaDeviceProp prop;
  cudaGetDeviceProperties(&prop, 0);

  // 6.1 on my quadro = Pascal
  std::cout << "compute cap: " << prop.major << "." << prop.minor << std::endl;
  //std::cout << "the cuda api version: " << CUDA_VERSION << std::endl;

  // check the concurrent kernel prop
  std::cout << "concurrent kernel: " << prop.concurrentKernels << std::endl;

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////// Create device container
  /////////////////////////////////////////////////////////////////////////////

  std::cout << "start computation" << std::endl;

  float*arr1{nullptr};
  float*arr2{nullptr};
  float*mulResult{nullptr};
  cudaStatus = cudaMallocManaged(&arr1, kCount*kCount*sizeof(float));

  if (cudaStatus != cudaSuccess) {
      std::cerr << "Failed to allocated first memory with error " << static_cast<int>(cudaStatus) << std::endl;
      return;
  }

  cudaStatus = cudaMallocManaged(&arr2, kCount*kCount*sizeof(float));

  if (cudaStatus != cudaSuccess) {
      std::cerr << "Failed to allocated second memory with error " << static_cast<int>(cudaStatus) << std::endl;
      return;
  }

  cudaStatus = cudaMallocManaged(&mulResult, kCount*kCount*sizeof(float));

  if (cudaStatus != cudaSuccess) {
      std::cerr << "Failed to allocated third memory with error " << static_cast<int>(cudaStatus) << std::endl;
      return;
  }

  std::cout << "memory allocated" << std::endl;

  cudaMemcpy(arr1, a, sizeof(float) * kCount * kCount, cudaMemcpyHostToDevice);
  cudaMemcpy(arr2, b, sizeof(float) * kCount * kCount, cudaMemcpyHostToDevice);

  std::cout << "array initialized" << std::endl;

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////// Perform arithmetic operation
  /////////////////////////////////////////////////////////////////////////////

  dim3 threadsPerBlock(16, 16);
  dim3 numBlocks(kCount / threadsPerBlock.x, kCount / threadsPerBlock.y);

  if (!useLib) {
    std::cout << "using raw cuda" << std::endl;
    mul_tile<<<numBlocks, threadsPerBlock>>>(kCount, arr1, arr2, mulResult);
  } else {
    std::cout << "using cublas lib" << std::endl;
    mul_blas(kCount, arr1, arr2, mulResult);
  }
  
  cudaDeviceSynchronize();
  add<<<numBlocks, threadsPerBlock>>>(kCount, mulResult, mulResult);

  // Wait for GPU to finish before accessing on host
  cudaDeviceSynchronize();

  // transfer the result in c
  cudaMemcpy(c, mulResult, sizeof(float) * kCount * kCount, cudaMemcpyDeviceToHost);

  cudaFree(arr1);
  cudaFree(arr2);
  cudaFree(mulResult);

  cudaProfilerStop();
}

void compute_with_acc_wrapper(float*a, float*b, float*c, size_t count) {
  compute(a,b,c,count);
}

void test_mul_from_external_lib(float*a, float*b, float*c, size_t count) {
  compute(a,b,c,count, true);
}