
__kernel void add(__global float* arr1, __global const float* arr2, const unsigned int size)
{
  int gid = get_global_id(0);
  if (gid < size)
    arr1[gid] = arr1[gid] + arr2[gid];
}

/*void mul(uint64_t size, float* __restrict__ arr1, float* __restrict__  arr2, float* __restrict__ out)
{
  float res = 0.0;
  {
    {
      for (uint64_t row = 0; row < size; row++)
      {
        for (uint64_t col = 0; col < size; col++)
        {
          res = 0.0f;
          // reduction -> updating the same variable at each loop
          for (uint64_t s = 0; s < size; s++)
          {
            res += arr1[row * size + s] * arr2[s * size + col];
          }
          out[row * size + col] = res;
        }
      }
    }
  }
}*/
