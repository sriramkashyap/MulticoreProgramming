#include "scheduler.h"

#include <sched.h>
#include <sys/syscall.h>
#include <sys/sysinfo.h>
#include <linux/sched/types.h>
#include <cstring>
#include <unistd.h>
#include <vector>
#include <chrono>
#include <thread>

namespace scheduler {

// debug feature to benchmark CPU scaling.
int force_num_procs = 0;

bool SetAffinity(std::vector<int> cores) {
  cpu_set_t     set;
  CPU_ZERO(&set);
  for (int core: cores) {
    CPU_SET(core, &set);
  }
  return (sched_setaffinity(0, cores.size(), &set) >= 0);
}

bool SetAttributes(SchedulerAttributes& s) {
  struct sched_attr attr;
  size_t size = sizeof(struct sched_attr);
  memset(&attr, 0, size);
  attr.size = size;
  attr.sched_policy = SCHED_OTHER; // Or SCHED_FIFO, SCHED_RR
  attr.sched_nice = s.niceness;
  attr.sched_util_min = s.min_util;
  attr.sched_util_max = s.max_util;
  
  if (syscall(SYS_sched_setattr,0, &attr, 0) == -1) {
  	return false;
  }
  return true;
}

void SleepForMs(int milliseconds) {
  if (milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
  }
}

int GetNumProcs() {
  return force_num_procs == 0 ? get_nprocs() : force_num_procs;
}

void ForceNumProcs(int n) {
  force_num_procs = n;
}

} // namespace scheduler.