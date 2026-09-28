#include "stats.h"
#include "util.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <vector>
#include <fstream>
#include <iostream>


namespace stats {

int64_t Timer::GetTimeNs(){
  auto now_chrono = std::chrono::system_clock::now();
  auto epoch_time = now_chrono.time_since_epoch();
  return std::chrono::duration_cast<std::chrono::nanoseconds>(epoch_time).count();
}

Metric::Metric() : Metric(4096) {}

Metric::Metric(int starting_size) {
	data.reserve(starting_size);
}

void Metric::Record(int64_t value) {
  data.push_back(value);
}
	
int64_t Metric::Count() {
  return data.size();
}

int64_t Metric::Percentile(float p) {
  if (p < 0.0f || p > 1.0f || data.empty()) return 0;
  std::sort(data.begin(), data.end()); // Ascending order.
  int index = static_cast<int>(p * (data.size()-1));
  return data[index];
}

void Metric::Reset() {
  data.clear();
}

bool Metric::Save(const std::string& file_name) {
  if (!utilities::CreateDir(file_name)) { return false; }
  std::ofstream outfile(file_name);

  if (outfile.is_open()) {
    for (const auto& value : data) {
      outfile << value << std::endl;
    }
    outfile.close();
	return true;
  } else {
    std::cerr << "Error opening file!" << std::endl;
	return false;
  }
}

void Metric::PrintSummary(float scale) {
  for (float percentile: {0.0f,0.5f,0.9f, 0.99f}){
    std::cout << Percentile(percentile) / scale << ",";
  }
}

} // namespace stats.