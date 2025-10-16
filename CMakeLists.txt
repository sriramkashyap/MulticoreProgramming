cmake_minimum_required(VERSION 3.5)
project(Test)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-enum-enum-conversion -fopenmp")

find_package(Threads)
find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

add_library(nbody STATIC nbody.cc)
add_library(stats STATIC stats.cc)
add_library(scheduler STATIC scheduler.cc)
add_library(image STATIC image.cc)

add_executable(test test.cc)

target_link_libraries(test nbody stats scheduler image ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(test ${OpenCV_LIBS})

if (ANDROID)
  target_link_libraries(test android log)
endif ()
