// C++ Program to demonstrate thread pooling
// Adapted from https://www.geeksforgeeks.org/cpp/thread-pool-in-cpp/

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <atomic>

namespace scheduler {

// Class that represents a simple thread pool
class ThreadPool {
public:
  ThreadPool(size_t num_threads)
  {
    // Creating worker threads
    for (size_t i = 0; i < num_threads; ++i) {
      threads_.emplace_back([this] {
        while (true) {
          std::function<void()> task;
          {
            // Locking the queue so that data
            // can be shared safely
            std::unique_lock<std::mutex> lock(queue_mutex_);
            completion_cv_.notify_one();

            // Waiting until there is a task to
            // execute or the pool is stopped
            cv_.wait(lock, [this] {
              return !tasks_.empty() || stop_.load();
            });

            // exit the thread in case the pool
            // is stopped and there are no tasks
            if (stop_.load() && tasks_.empty()) {
              return;
            }

            // Get the next task from the queue
            task = move(tasks_.front());
            ++in_flight_tasks_;
            tasks_.pop();
          }
          task();
          --in_flight_tasks_;
        }
      });
    }
  }

  // Destructor to stop the thread pool
  ~ThreadPool()
  {
    {
      // Lock the queue to update the stop flag safely
      std::unique_lock<std::mutex> lock(queue_mutex_);
      stop_ = true;
    }

    // Notify all threads
    cv_.notify_all();

    // Joining all worker threads to ensure they have
    // completed their tasks
    for (auto& thread : threads_) {
      thread.join();
    }
  }

  // Enqueue task for execution by the thread pool
  void enqueue(std::function<void()> task)
  {
    {
      std::unique_lock<std::mutex> lock(queue_mutex_);
      tasks_.emplace(move(task));
    }
    cv_.notify_one();
  }
  
  void wait_for_all() {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    completion_cv_.wait(lock, [this] {
      return (tasks_.empty() && in_flight_tasks_ == 0); 
    }); 
  }

private:
  std::vector<std::thread> threads_;
  std::queue<std::function<void()>> tasks_;
  std::mutex queue_mutex_;
  std::condition_variable cv_, completion_cv_;  
  std::atomic<bool> stop_ = false;
  std::atomic<int> in_flight_tasks_ = 0;
};

}  // namespace scheduler

#endif  // THREAD_POOL_H