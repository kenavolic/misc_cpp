#include <CL/cl2.hpp>
#include <iostream>
#include <fstream>
#include <sstream>


void compute_with_acc_wrapper(float* a, float* b, float* c, size_t count)
{
  std::vector<cl::Platform> platforms;
  cl::Platform::get(&platforms);
  if (platforms.size() == 0)
  {
    std::cout << "Platform size 0\n";
    return;
  }

  // Print number of platforms and list of platforms
  std::cout << "Platform count: " << platforms.size() << std::endl;
  std::string platformVendor;
  for (unsigned int i = 0; i < platforms.size(); ++i)
  {
    platforms[i].getInfo((cl_platform_info)CL_PLATFORM_VENDOR, &platformVendor);
    std::cout << "Platform from: " << platformVendor << std::endl;
  }

  cl_context_properties properties[] =
  {
    CL_CONTEXT_PLATFORM,
    (cl_context_properties)(platforms[0])(),
    0
  };
  cl::Context context(CL_DEVICE_TYPE_ALL, properties);

  std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

  // Print number of devices and list of devices
  std::cout << "Device count: " << devices.size() << std::endl;
  for (unsigned int i = 0; i < devices.size(); ++i)
  {
    std::cout << "Device #" << i << ": " << devices[i].getInfo<CL_DEVICE_NAME>() << std::endl;
  }

  std::ifstream fin("compute.cl", std::ios::binary);
  std::ostringstream ostrm;
  ostrm << fin.rdbuf();
  std::string str{ostrm.str()};
  cl::Program::Sources sources;
  sources.push_back({ str.c_str(), str.length() });

  cl::Program program(context, sources);
  program.build(devices);

  cl_int err = CL_SUCCESS;
  // should add try catch for opencl extension
  cl::Kernel kernel(program, "add", &err);
  cl::compatibility::make_kernel<cl::Buffer, cl::Buffer, unsigned>addKernel(program, "add");

  cl::Buffer A_d(context, CL_MEM_READ_WRITE, sizeof(float) * count * count);
  cl::Buffer B_d(context, CL_MEM_READ_ONLY, sizeof(float) * count * count);
  cl::Buffer C_d(context, CL_MEM_READ_WRITE, sizeof(float) * count * count);

  cl::CommandQueue queue(context, devices[0]);
  cl::NDRange global(count * count / 64);
  cl::NDRange local(64);
  addKernel(cl::EnqueueArgs(queue, global, local), A_d, B_d, count * count);

  queue.enqueueReadBuffer(C_d, CL_TRUE, 0,  sizeof(float) * count * count, c);
}

void test_mul_from_external_lib(float* a, float* b, float* c, size_t count)
{
  std::cout << "not implemented" << std::endl;
}