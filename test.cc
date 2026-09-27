#include <iostream>
#include <format>
#include <string>
#include <perfetto.h>
#include "scheduler.h"
#include "stats.h"
#include "nbody.h"

constexpr int kNumPointsNbody = 5000;
constexpr float kTimeStepNBody = 0.001f;

// Define categories used in your application
PERFETTO_DEFINE_CATEGORIES(
  perfetto::Category("multicore").SetDescription("MultiCore Test")
);

// Required static storage macro
PERFETTO_TRACK_EVENT_STATIC_STORAGE();

void InitProfiler() {
  // Initialize Perfetto in-process backend
  perfetto::TracingInitArgs args;
  args.backends |= perfetto::kSystemBackend;
  perfetto::Tracing::Initialize(args);
  perfetto::TrackEvent::Register();
}

template <class T>
class NBodyExperiment {
  public:
    NBodyExperiment() : min(0.0f,0.0f), max(10.0f,10.0f) {
	    universe.Initialize(kNumPointsNbody, min, max, 0xFFFF);
	  }
	  void Run(const std::string& name, int iterations, bool save_image = false, float time_step = kTimeStepNBody) {
      stats::Metric latency_ns;
      for (int iteration = 0; iteration < iterations; ++iteration) {
      // TRACE_EVENT("multicore", perfetto::DynamicString(name));
        stats::Timer t1;
        t1.Start();
        universe.Simulate(time_step);
        latency_ns.Record(t1.GetElapsedNs());
        if (save_image) {
          universe.SaveToFile(std::format("images/{}_{:03}.bmp", name, iteration), min, max, 512, 512);
        }
      }
      std::cout << std::endl << name << ",";
      latency_ns.PrintSummary(1000000.0f);
      latency_ns.Save(std::format("stats/latency_{}.txt", name));
    }
  protected:
    T universe;
    nbody::Point2D min, max; // Range of the points.
};


void RunNBodyTests() {
  constexpr int kSingleThreadIterations = 10;
  constexpr int kMultiThreadIterations = 300;
  
  // Single threaded.
  NBodyExperiment<nbody::ParticleSystem> nbody_st;
  nbody_st.Run("nbody_st", kSingleThreadIterations);

  // Automatic threading using OpenMP.
  NBodyExperiment<nbody::ParticleSystemOpenMP> nbody_mt_openmp;
  nbody_mt_openmp.Run("nbody_mt_openmp", kMultiThreadIterations);

  // Manual threading using std::thread.
  NBodyExperiment<nbody::ParticleSystemStdThread> nbody_mt_std;
  nbody_mt_std.Run("nbody_mt_std", kMultiThreadIterations);
  
  // Automatic threading using OpenMP Dynamic scheduling.
  NBodyExperiment<nbody::ParticleSystemOpenMPDynamic> nbody_mt_openmp_dynamic;
  nbody_mt_openmp_dynamic.Run("nbody_mt_openmp_dynamic", kMultiThreadIterations);

  // Manual threading using an std::thread based pool.
  NBodyExperiment<nbody::ParticleSystemStdThreadPool> nbody_mt_std_pool;
  nbody_mt_std_pool.Run("nbody_mt_stdpool", kMultiThreadIterations);

  // Run with different thread counts.
  /*
  for (int i = 1; i < 30; ++i) {
    scheduler::ForceNumProcs(i);
    NBodyExperiment<nbody::ParticleSystemStdThreadPool> nbody_mt_std_pool2;
    nbody_mt_std_pool2.Run(std::string("nbody_mt_stdpool") + std::to_string(i), kMultiThreadIterations);
  } */

  // Cuda
  NBodyExperiment<nbody::ParticleSystemCuda> nbody_cuda;
  nbody_cuda.Run("nbody_cuda", kMultiThreadIterations);
}

void RenderNBody(int iterations) {
  // Saves images of nbody simulation.
  NBodyExperiment<nbody::ParticleSystemCuda> nbody;
  nbody.Run("image00", iterations, true);
}

int main(){
  InitProfiler();
  std::cout << "Number of cores: " << scheduler::GetNumProcs() << std::endl;
  std::cout << "Number of points: " << kNumPointsNbody << std::endl;
  std::cout << "name,P0 ms,P50 ms,P90 ms,P99 ms";
  RunNBodyTests();
  // RenderNBody(1024);
  return 0;
}
