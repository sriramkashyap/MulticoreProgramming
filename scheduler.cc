#include "scheduler.h"

#include <cstring>
#include <vector>
#include <chrono>
#include <thread>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <sched.h>
#include <sys/syscall.h>
#include <sys/sysinfo.h>
#include <linux/sched/types.h>
#include <unistd.h>
#endif

namespace scheduler {

// debug feature to benchmark CPU scaling.
int force_num_procs = 0;

bool SetAffinity(std::vector<int> cores) {
#if defined(_WIN32) || defined(_WIN64)
  DWORD_PTR mask = 0;
  for (int core : cores) {
    if (core >= 0 && core < 64) {
      mask |= (1ULL << core);
    }
  }
  // Set affinity for the current thread
  return SetThreadAffinityMask(GetCurrentThread(), mask) != 0;
#else
  cpu_set_t set;
  CPU_ZERO(&set);
  for (int core : cores) {
    CPU_SET(core, &set);
  }
  return (sched_setaffinity(0, cores.size(), &set) >= 0);
#endif
}

bool SetAttributes(SchedulerAttributes& s) {
#if defined(_WIN32) || defined(_WIN64)
  // Windows doesn't have direct equivalents for Linux CFS/EEVDF util min/max or exact nice values.
  // We map the niceness attribute to Windows thread priorities as a close approximation.
  int priority = THREAD_PRIORITY_NORMAL;
  if (s.niceness < 0) {
    priority = THREAD_PRIORITY_HIGHEST;
  } else if (s.niceness > 0) {
    priority = THREAD_PRIORITY_LOWEST;
  }
  return SetThreadPriority(GetCurrentThread(), priority) != 0;
#else
  struct sched_attr attr;
  size_t size = sizeof(struct sched_attr);
  memset(&attr, 0, size);
  attr.size = size;
  attr.sched_policy = SCHED_OTHER; // Or SCHED_FIFO, SCHED_RR
  attr.sched_nice = s.niceness;
  attr.sched_util_min = s.min_util;
  attr.sched_util_max = s.max_util;
  
  if (syscall(SYS_sched_setattr, 0, &attr, 0) == -1) {
    return false;
  }
  return true;
#endif
}

void SleepForMs(int milliseconds) {
  if (milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
  }
}

int GetNumProcs() {
  return (force_num_procs > 0) ? force_num_procs : std::thread::hardware_concurrency();
}

void ForceNumProcs(int n) {
  force_num_procs = n;
}

} // namespace scheduler