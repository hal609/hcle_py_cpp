#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

namespace hcle
{
    namespace common
    {

        class ThreadPool
        {
        public:
            ThreadPool(size_t num_threads);
            ~ThreadPool();

            // Adds a new task to the queue
            void enqueue(std::function<void()> task);

            // Waits until the number of completed tasks matches the expected count
            void wait_for_completion(size_t expected_tasks);

        private:
            std::vector<std::thread> workers_;
            std::queue<std::function<void()>> tasks_;

            std::mutex queue_mutex_;
            std::condition_variable condition_;
            std::condition_variable completion_condition_;
            bool stop_;
            size_t completed_tasks_;
        };

    } // namespace common
} // namespace hcle