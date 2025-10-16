#include "nbody.h"
#include "image.h"
#include "scheduler.h"

#include <thread>
#include <mutex>
#include <atomic>
#include <iostream>
#include <cstdlib>
#include <omp.h>

namespace nbody {
float Gravity(float m1, float m2, float square_distance) {
  constexpr float kGravityConstant = 1.0f;
  return square_distance == 0 ? 0 : kGravityConstant * m1 * m2 / square_distance;
}

void ParticleSystem::Initialize(int count, Point2D min, Point2D max, unsigned int seed) {
  data.resize(count);
  std::srand(seed);
  float x_scale = (max.x - min.x);
  float y_scale = (max.y - min.y);
  for (int i = 0; i < count; ++i) {
	float x = min.x + (rand() * x_scale / RAND_MAX);
	float y = min.y + (rand() * y_scale / RAND_MAX);
    data[i].position = Point2D(x, y);
  }
}

bool ParticleSystem::SaveToFile(const std::string& file_name, Point2D min, Point2D max, int width, int height) {
  image::Image img(width, height);
  image::RGB color(255,255,255);
  Point2D resize = max.Sub(min);
  for (PointMass& p: data) {
	auto& pos= p.position;
	if (!pos.Inside(min, max)) continue;
	Point2D result = pos.Sub(min).Scale(width/resize.x, height/resize.y);
    img.Set(static_cast<int>(result.x), static_cast<int>(result.y), color);
  }
  return img.SaveToFile(file_name);
}

void ParticleSystem::CalculatePoint(int index, float timestep) {
  PointMass& pi = data[index];
  Point2D net_force; // calculate force on pi.
  for (int j = 0; j < data.size(); ++j) {
    Point2D vec = data[j].position.Sub(pi.position);
    float r_square = vec.SquareLength();
	net_force = net_force.Add(vec.Scale(Gravity(pi.mass, data[j].mass, r_square)));
  }
  pi.velocity = pi.velocity.Add(net_force.Scale(timestep / pi.mass));
}

void ParticleSystem::UpdatePoint(int index, float timestep) {
  PointMass& pi = data[index];
  pi.position = pi.position.Add(pi.velocity.Scale(timestep));
}

void ParticleSystem::Simulate(float timestep) {
  for (int i = 0; i < data.size(); ++i) {
    CalculatePoint(i, timestep);
	UpdatePoint(i, timestep);
  }
}

void ParticleSystemOpenMP::Simulate(float timestep) { 
  #pragma omp parallel for
  for (int i = 0; i < data.size(); ++i) {
    CalculatePoint(i, timestep);
  }
  // Update results after all calculations are done. Could be parallel?
  for (int i = 0; i < data.size(); ++i) {
    UpdatePoint(i, timestep);
  }
}

void ParticleSystemOpenMPDynamic::Simulate(float timestep) { 
  #pragma omp parallel for schedule(dynamic)
  for (int i = 0; i < data.size(); ++i) {
    CalculatePoint(i, timestep);
  }
  // Update results after all calculations are done. Could be parallel?
  // #pragma omp parallel for schedule(static,64)
  for (int i = 0; i < data.size(); ++i) {
    UpdatePoint(i, timestep);
  }
}

void WorkerFunction(ParticleSystem* system, std::atomic<int>* jobs_ptr, float timestep) {
  auto& jobs = *jobs_ptr;
  constexpr int kJobSize = 4;
  while(1) {
	int end = jobs.fetch_sub(kJobSize);
	int start = std::max(end - kJobSize, 0);
	if (end < 0) return;
	for (int i = start; i < end; ++i) {
	  system->CalculatePoint(i, timestep);
	}
  }
}

void ParticleSystemStdThread::Simulate(float timestep) {
  int num_threads = scheduler::GetNumProcs();
  std::atomic<int> jobs(data.size());
  
  std::vector<std::thread> threads;
  for (int i = 0; i < num_threads; ++i) {
	threads.emplace_back(WorkerFunction, this, &jobs, timestep); // Create and add threads.
  }
  for (std::thread& t : threads) {
   if (t.joinable()) t.join(); // Wait for each thread to complete.
  }

  // Update results after all calculations are done. Could be parallel?
  for (int i = 0; i < data.size(); ++i) {
	UpdatePoint(i, timestep);
  }
}

} // namespace nbody.