#include "compute.h"

#include <iostream>
#include <chrono>

int main(int argc, char** argv)
{
  std::cout << "testing GPU computation" << std::endl;

  const uint64_t kCount = 1024;
  // we want to compute with addition and multiplication kernel
  // a*b + a*b = 2*a*b
  float* a = new float[kCount * kCount];
  float* b = new float[kCount * kCount];
  float* c = new float[kCount * kCount];
  float* d = new float[kCount * kCount];
  float* expected = new float[kCount * kCount];

  for (uint64_t i = 0; i < kCount; i++)
  {
    for (uint64_t j = 0; j < kCount; j++)
    {
      a[i * kCount + j] = rand() % 1024;
      b[i * kCount + j] = rand() % 1024;
    }
  }

  // should use google micro benchmark
  std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
  compute_with_acc_wrapper(a, b, c, kCount);
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  std::cout << "Time difference (Pure GPU) = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
  begin = std::chrono::steady_clock::now();
  test_mul_from_external_lib(a, b, d, kCount);
  end = std::chrono::steady_clock::now();
  std::cout << "Time difference (Library) = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
  std::cout << "Time difference (Library) = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[ucro]" << std::endl;

  // OpenMP
  #pragma omp parallel for default(none) shared(a,b,expected)
  for (uint64_t row = 0; row < kCount; row++)
  {
    for (uint64_t col = 0; col < kCount; col++)
    {
      float res{};
      for (uint64_t s = 0; s < kCount; s++)
      {
        res += a[row * kCount + s] * b[s * kCount + col];
      }
      expected[row * kCount + col] = 2 * res;
    }
  }

  for (uint64_t i = 0; i < kCount; ++i)
  {
    for (uint64_t j = 0; j < kCount; ++j)
    {
      if (c[i * kCount + j] != expected[i * kCount + j]) {
          std::cout << "there is an error in c" << std::endl;
          exit(1);
      }

      if (d[i * kCount + j] != expected[i * kCount + j]) {
          std::cout << "there is an error in d" << std::endl;
          exit(1);
      }
    }
  }

  delete [] a;
  delete [] b;
  delete [] c;
  delete [] d;
  delete [] expected;

  return 0;
}