#include "compute.h"

#include <cstdint>
#include <iostream>

#ifdef USE_PARALLEL
  #define ACC_TYPE parallel
#else
  #define ACC_TYPE kernels
#endif

void add(uint64_t size, float* arr1, float* arr2)
{
  int n = size * size;
  for (uint64_t i = 0; i < n; i++)
  {
    arr1[i] = 2 * arr1[i];
  }
}

void mul(uint64_t size, float* __restrict__ arr1, float* __restrict__  arr2, float* __restrict__ out)
{
  float res = 0.0;
  // if not independant loop, needs to be transformed in independant
  // loop to be run in parallel
#pragma acc data copyin(arr1[0:size*size], arr2[0:size*size]) copy(out[0:size*size])
  {
#pragma acc kernels
    {
#pragma acc loop independent
      for (uint64_t row = 0; row < size; row++)
      {
#pragma acc loop independent
        for (uint64_t col = 0; col < size; col++)
        {
          res = 0.0f;
          // reduction -> updating the same variable at each loop
#pragma acc loop reduction(+:res)
          for (uint64_t s = 0; s < size; s++)
          {
            res += arr1[row * size + s] * arr2[s * size + col];
          }
          out[row * size + col] = res;
        }
      }
    }
  }
}

void compute_with_acc_wrapper(float* a, float* b, float* c, size_t count)
{
  mul(count, a, b, c);
  add(count, c, c);
}

void test_mul_from_external_lib(float*a, float*b, float*c, size_t count) {
  std::cout << "not implemented" << std::endl;
}