cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CUDA_COMPILER "/usr/local/cuda/bin/nvcc")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CUDA_ARCHITECTURES 75 80 86 87 89 90 100 110 120)
set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -Wno-deprecated-gpu-targets --use_fast_math")
if(ENABLE_CUDA_DEBUG)
    set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -G")        # enable cuda-gdb (may significantly affect performance on some targets)
else()
    set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -lineinfo") # add line information to all builds for debug tools (exclusive to -G option)
endif()

project(Test LANGUAGES C CXX CUDA)

find_package(Threads)
find_package(OpenCV REQUIRED)
find_package(CUDAToolkit REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

add_library(nbody STATIC nbody.cc nbody.cu)
add_library(stats STATIC stats.cc)
add_library(scheduler STATIC scheduler.cc)
add_library(image STATIC image.cc)
add_library(util STATIC util.cc)

if(WIN32)
  set(PERFETTO_PATH "C:\\Code\\perfetto-cpp-sdk-src")
  add_library(perfetto STATIC "${PERFETTO_PATH}\\perfetto.cc")
else()
  set(PERFETTO_PATH "~/perfetto/perfetto/sdk")
  add_library(perfetto STATIC "${PERFETTO_PATH}/perfetto.cc")
endif()
include_directories(${PERFETTO_PATH})

add_executable(test test.cc)

target_compile_options(test PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:--extended-lambda>)
target_compile_features(test PRIVATE cxx_std_17 cuda_std_17)
set_target_properties(test PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
target_include_directories(test PUBLIC ${CUDAToolkit_INCLUDE_DIRS})

target_link_libraries(test nbody stats scheduler image util perfetto ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(test ${OpenCV_LIBS})

if(WIN32)
    # 1. Add compiler flags for MSVC
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /openmp")
    if(MSVC)
        target_compile_options(perfetto PRIVATE /bigobj)
    endif()

    # 2. Add required Windows preprocessor definitions
    target_compile_definitions(perfetto PRIVATE 
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )

    # 3. Link the WinSock2 library required by Perfetto
    target_link_libraries(perfetto PRIVATE ws2_32)
else()
  # Linux
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-enum-enum-conversion -fopenmp -O3 -march=native")
endif()