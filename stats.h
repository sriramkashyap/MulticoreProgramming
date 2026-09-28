#ifndef STATS_H
#define STATS_H

#include <cstdint>
#include <vector>
#include <string>

namespace stats {

class Timer {
  private:
  int64_t start = 0;
  
  int64_t GetTimeNs();
  public:
  void Start() {
    start = GetTimeNs();
  }
  
  int64_t GetElapsedNs() {
    return (GetTimeNs() - start);
  }

  int64_t ResetAndGetElapsedNs() {
    int64_t now = GetTimeNs();
    int64_t ret = now - start;
    start = now;
    return ret;
  }
};

class Metric {
  private:
    std::vector<int64_t> data;
  public:
    Metric();
    explicit Metric(int starting_size);
    void Record(int64_t value);
    int64_t Count();
    int64_t Percentile(float p);
    void Reset();
    bool Save(const std::string& file_name);
    void PrintSummary(float scale);
};

} // namespace stats.

#endif // STATS_H