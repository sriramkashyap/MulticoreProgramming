#include <iostream>
#include <format>
#include <string>
#include "scheduler.h"
#include "stats.h"
#include "nbody.h"

template <class T>
class NBodyExperiment {
  public:
    NBodyExperiment() : min(0.0f,0.0f), max(10.0f,10.0f) {
	  universe.Initialize(2000, min, max, 0xFFFF);
	}
	void Run(const std::string& name, int iterations, bool save_image) {
	  constexpr float kTimeStep = 0.01f;
	  
	  stats::Metric latency_ns;
	  for (int iteration = 0; iteration < iterations; ++iteration) {
	    stats::Timer t1;
	    t1.Start();
	    universe.Simulate(kTimeStep);
	    latency_ns.Record(t1.GetElapsedNs());
		if (save_image) {
	      universe.SaveToFile(std::format("image_{}_{:03}.bmp", name, iteration), min, max, 512, 512);
		}
	  }
	  std::cout << name << " results:" << std::endl;
	  latency_ns.PrintSummary();
	  latency_ns.Save(std::format("latency_{}.txt", name));
	}
  protected:
    T universe;
	nbody::Point2D min, max; // Range of the points.
};


void RunNBodyTests(int iterations) {
  // Single threaded.
  NBodyExperiment<nbody::ParticleSystem> nbody_st;
  nbody_st.Run("nbody_st", iterations, false);
  
  // Manual threading using std::thread.
  NBodyExperiment<nbody::ParticleSystemStdThread> nbody_mt_std;
  nbody_mt_std.Run("nbody_mt_std", iterations, false);
    
  // Automatic threading using OpenMP.
  NBodyExperiment<nbody::ParticleSystemOpenMP> nbody_mt_openmp;
  nbody_mt_openmp.Run("nbody_mt_openmp", iterations, false);
  
  // Automatic threading using OpenMP Dynamic scheduling.
  NBodyExperiment<nbody::ParticleSystemOpenMPDynamic> nbody_mt_openmp_dynamic;
  nbody_mt_openmp_dynamic.Run("nbody_mt_openmp_dynamic", iterations, false);
}

void RenderNBody(int iterations) {
  // Saves images of nbody simulation.
  NBodyExperiment<nbody::ParticleSystemStdThread> nbody_mt_std;
  nbody_mt_std.Run("nbody_mt_std", iterations, true);
}

int main(){
  std::cout << "Number of cores: " << scheduler::GetNumProcs() << std::endl;
  RunNBodyTests(10);
  // RenderNBody(100);
  return 0;
}
