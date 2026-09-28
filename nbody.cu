#include <iostream>
#include <math.h>
#include "nbody.h"
#include "stats.h"

namespace nbody {

static constexpr int kNumThreadsPerBlock = 256;

__global__
void CudaCalculatePoint(float timestep, PointMass* data, int count) {
  // Calculate the unique global index
  int index = blockIdx.x * blockDim.x + threadIdx.x;

  // Boundary check to prevent accessing memory out of bounds
  if (index < count) {
    PointMass& pi = data[index];
    Point2D net_force; // calculate force on pi.
    for (int j = 0; j < count; ++j) {
      Point2D vec = data[j].position.Sub(pi.position);
      float r_square = vec.SquareLength();
      net_force = net_force.Add(vec.Scale(Gravity(pi.mass, data[j].mass, r_square)));
    }
    pi.velocity = pi.velocity.Add(net_force.Scale(timestep / pi.mass));
  }
}

__global__
void CudaCalculatePointShared(float timestep, PointMass* data, int count) {
  // Define a shared memory tile (adjust block size / tile size as needed)
  __shared__ char shared_data_raw[kNumThreadsPerBlock * sizeof(PointMass)];
  PointMass* shared_data = reinterpret_cast<PointMass*>(shared_data_raw);

  int index = blockIdx.x * blockDim.x + threadIdx.x;
  int tid = threadIdx.x;

  Point2D net_force;
  PointMass pi = (index < count) ? data[index] : PointMass{};

  // Loop over tiles of particles
  for (int i = 0; i < count; i += blockDim.x) {
    // 1. Collaboratively load a tile into shared memory
    int load_index = i + tid;
    if (load_index < count) {
      shared_data[tid] = data[load_index];
    }
    __syncthreads();

    // 2. Compute force contributions from the current shared tile
    if (index < count) {
      int limit = min((int)blockDim.x, count - i);
      for (int j = 0; j < limit; ++j) {
        Point2D vec = shared_data[j].position.Sub(pi.position);
        float r_square = vec.SquareLength();
        // Avoid self-interaction if index == i + j
        if (r_square > 0.0f) {
          net_force = net_force.Add(vec.Scale(Gravity(pi.mass, shared_data[j].mass, r_square)));
        }
      }
    }
    __syncthreads(); // Synchronize before loading the next tile
  }

  // Write final velocity update
  if (index < count) {
    pi.velocity = pi.velocity.Add(net_force.Scale(timestep / pi.mass));
    data[index].velocity = pi.velocity; // or update back
  }
}

__global__
void CudaUpdatePoint(float timestep, PointMass* data, int count) {
  // Calculate the unique global index
  int index = blockIdx.x * blockDim.x + threadIdx.x;

  // Boundary check to prevent accessing memory out of bounds
  if (index < count) {
    PointMass& pi = data[index];
    pi.position = pi.position.Add(pi.velocity.Scale(timestep));
  }
}

__host__
void ParticleSystemCuda::Simulate(float timestep) {
  cudaMemcpy(device_data_, data.data(), s_bytes_, cudaMemcpyHostToDevice);
  
  int dataSize = static_cast<int>(data.size());
  int xdim = kNumThreadsPerBlock;
  int ydim = (dataSize + xdim) / xdim;
  CudaCalculatePoint<<<xdim,ydim>>>(timestep, device_data_, dataSize);
  CudaUpdatePoint<<<xdim,ydim>>>(timestep, device_data_, dataSize);
  
  // Wait for GPU to finish before accessing on host
  cudaDeviceSynchronize();
  cudaMemcpy(data.data(), device_data_, s_bytes_, cudaMemcpyDeviceToHost);
}

__host__
void ParticleSystemCuda::Initialize(int count, Point2D min, Point2D max, unsigned int seed) {
  ParticleSystem::Initialize(count, min, max, seed);
  s_bytes_ = data.size()*sizeof(PointMass);
  cudaMalloc(&device_data_, s_bytes_);
}

__host__
ParticleSystemCuda::~ParticleSystemCuda() {
  if (device_data_ != nullptr) {
    cudaFree(device_data_);
  }
}

__host__
void ParticleSystemCudaShared::Simulate(float timestep) {
  cudaMemcpy(device_data_, data.data(), s_bytes_, cudaMemcpyHostToDevice);
  
  int dataSize = static_cast<int>(data.size());
  int xdim = kNumThreadsPerBlock;
  int ydim = (dataSize + xdim) / xdim;
  CudaCalculatePointShared<<<xdim,ydim>>>(timestep, device_data_, dataSize);
  CudaUpdatePoint<<<xdim,ydim>>>(timestep, device_data_, dataSize);
  
  // Wait for GPU to finish before accessing on host
  cudaDeviceSynchronize();
  cudaMemcpy(data.data(), device_data_, s_bytes_, cudaMemcpyDeviceToHost);
}

} // namespace nbody