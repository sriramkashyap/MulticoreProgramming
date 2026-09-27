#include <iostream>
#include <math.h>
#include "nbody.h"

namespace nbody {

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
  int xdim = 256;
  int ydim = (data.size() + xdim) / xdim;
  CudaCalculatePoint<<<xdim,ydim>>>(timestep, device_data_, data.size());
  CudaUpdatePoint<<<xdim,ydim>>>(timestep, device_data_, data.size());
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

} // namespace nbody