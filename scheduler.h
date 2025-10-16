#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector>

namespace scheduler {

struct SchedulerAttributes {
  int niceness = 0;    // Thread niceness (bigger is slower).
  int min_util = 512;  // Thread utilization min: 0-1024.
  int max_util = 1024; // Thread utilization max: min-1024.
};

bool SetAttributes(SchedulerAttributes& s);
bool SetAffinity(std::vector<int> cores);
void SleepForMs(int milliseconds);
int GetNumProcs();
void ForceNumProcs(int n);

} // namespace scheduler.

#endif // SCHEDULER_H