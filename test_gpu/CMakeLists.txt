cmake_minimum_required(VERSION 3.24)

project(compute_gpu LANGUAGES CXX)

# Options (default is CUDA)
option(BUILD_WITH_OPENACC "Build with openacc" OFF)
option(BUILD_WITH_OPENCL "Build with opencl" OFF)

# Cuda config
if(NOT BUILD_WITH_OPENACC AND NOT BUILD_WITH_OPENCL)
  message("Enabling cuda")
  include(CheckLanguage)
  # compute capabilities SM version 
  # features supported by the GPU hardware and is used by applications at runtime to 
  # determine which hardware features and/or instructions are available on the present GPU.
  # https://developer.nvidia.com/cuda-gpus
  # control the -arch and -code compiler options or the -gencode compiler
  check_language(CUDA)
  if(NOT DEFINED CMAKE_CUDA_STANDARD)
      set(CMAKE_CUDA_STANDARD 11)
      set(CMAKE_CUDA_STANDARD_REQUIRED ON)
  endif()
  enable_language(CUDA)
endif()

# Target config

set(src_file)
set(target_libs)

if (BUILD_WITH_OPENACC)
  find_package(OpenACC REQUIRED)

  set(src_file
    ${src_file}
    compute_acc.cpp)

  # -fcf-protection=none 
  # gpu=managed
  set(OpenACC_CXX_OPTIONS ${OpenACC_CXX_OPTIONS} "-fcf-protection=none" "-foffload=nvptx-none" "-foffload=-misa=sm_35" "-Ofast")
  message("The compiler flags are ${OpenACC_CXX_OPTIONS}")
elseif(BUILD_WITH_OPENCL)
  find_package(OpenCL REQUIRED)
  set(target_libs OpenCL::OpenCL)
  set(src_file
    ${src_file}
    compute_cl.cpp)
  add_definitions(-DCL_HPP_TARGET_OPENCL_VERSION=210)
  configure_file(compute.cl ${CMAKE_BINARY_DIR}/compute.cl COPYONLY)
else()
  set(src_file
      ${src_file}
      compute.cu
      )
  set(target_libs cublas)
endif()

add_library(computeLib
    ${src_file}
    compute.h
)
target_include_directories(computeLib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
target_link_libraries(computeLib ${target_libs})

if (BUILD_WITH_OPENACC)
  target_compile_options(computeLib PUBLIC ${OpenACC_CXX_OPTIONS})
  target_link_options(computeLib PUBLIC ${OpenACC_CXX_OPTIONS})
elseif(BUILD_WITH_OPENCL)
  target_compile_features(computeLib PRIVATE cxx_auto_type) # for opencl
else()
  set_target_properties(computeLib PROPERTIES
                      CUDA_SEPARABLE_COMPILATION ON) # for cuda
endif()

add_executable(main main.cpp)
target_link_libraries(main computeLib)