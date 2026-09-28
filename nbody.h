#ifndef NBODY_H
#define NBODY_H

#include "thread_pool.h"

#include <vector>
#include <string>
#include <cmath>

// Check if the file is being compiled by a CUDA compiler
#ifdef __CUDACC__
    #define CUDA_CALLABLE __host__ __device__
#else
    #define CUDA_CALLABLE 
#endif

namespace nbody {

struct Point2D {
  float x, y;
  CUDA_CALLABLE Point2D(): Point2D(0.0f,0.0f){}
  CUDA_CALLABLE Point2D(float x, float y): x(x), y(y) {}
  CUDA_CALLABLE float SquareLength() {
    return (x*x + y*y);
  }
  CUDA_CALLABLE float Length() {
	return sqrt(SquareLength());
  }
  CUDA_CALLABLE Point2D UnitVector() {
	return Scale(1.0f / Length());
  }
  CUDA_CALLABLE Point2D Add(const Point2D& p2) {
	return Point2D(x+p2.x, y+p2.y);
  }
  CUDA_CALLABLE Point2D Sub(const Point2D& p2) {
	return Point2D(x-p2.x, y-p2.y);
  }
  CUDA_CALLABLE Point2D Scale(float scale) {
	return Point2D(x*scale, y*scale);
  }
  CUDA_CALLABLE Point2D Scale(float xscale, float yscale) {
	return Point2D(x*xscale, y*yscale);
  }  
  CUDA_CALLABLE bool Inside(const Point2D& min, const Point2D& max) {
    return (x >= min.x && x < max.x && y >= min.y && y < max.y);
  }
};

struct PointMass {
  Point2D position;
  Point2D velocity;
  float mass = 1.0f;
  float data[3];  // pad to 32 bytes
};

CUDA_CALLABLE inline float Gravity(float m1, float m2, float square_distance) {
  constexpr float kGravityConstant = 1.0f;
  return square_distance == 0 ? 0 : kGravityConstant * m1 * m2 / square_distance;
}

class ParticleSystem {
  public:
    // Setup the particle system using a random seed.
	virtual void Initialize(int count, Point2D min, Point2D max, unsigned int seed);
	
	// Compute the effect of all other points on point at `index`.
	void CalculatePoint(int index, float timestep);
	
	// Update the position of point at `index` based on its current velocity.
	void UpdatePoint(int index, float timestep);
	
	// Simulate the whole system using a time scaling factor `timestep`.
  virtual void Simulate(float timestep);
	
	int Count() { return static_cast<int>(data.size()); }
	
	// Access to point data.
	PointMass Get(int index) { return data[index]; }
	
	// Save file for visualization.
	bool SaveToFile(const std::string& file_name, Point2D min, Point2D max, int width, int height);
  protected:
    std::vector<PointMass> data;
};

class ParticleSystemOpenMP : public ParticleSystem {
  public:
    void Simulate(float timestep) override;
};

class ParticleSystemOpenMPDynamic : public ParticleSystem {
  public:
    void Simulate(float timestep) override;
};

class ParticleSystemStdThread : public ParticleSystem {
  public:
    void Simulate(float timestep) override;
};

class ParticleSystemStdThreadPool : public ParticleSystem {
  public:
    ParticleSystemStdThreadPool();
    void Simulate(float timestep) override;
    scheduler::ThreadPool pool_;
};

class ParticleSystemCuda : public ParticleSystem {
  public:
	  void Initialize(int count, Point2D min, Point2D max, unsigned int seed) override;
	  void Simulate(float timestep) override;
	  ~ParticleSystemCuda();
  protected:
	  PointMass* device_data_ = nullptr;
	  size_t s_bytes_ = 0;
};

class ParticleSystemCudaShared : public ParticleSystemCuda {
  public:
    void Simulate(float timestep) override;
};

} // namespace nbody.

#endif // NBODY_H