#include "hcle/common/thread_pool.hpp"

namespace hcle
{
    namespace common
    {

        ThreadPool::ThreadPool(size_t num_threads) : stop_(false), completed_tasks_(0)
        {
            for (size_t i = 0; i < num_threads; ++i)
            {
                workers_.emplace_back([this]
                                      {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex_);
                    this->condition_.wait(lock, [this] { return this->stop_ || !this->tasks_.empty(); });
                    if (this->stop_ && this->tasks_.empty()) {
                        return;
                    }
                    task = std::move(this->tasks_.front());
                    this->tasks_.pop();
                }
                task();
                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex_);
                    completed_tasks_++;
                }
                completion_condition_.notify_one();
            } });
            }
        }

        ThreadPool::~ThreadPool()
        {
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                stop_ = true;
            }
            condition_.notify_all();
            for (std::thread &worker : workers_)
            {
                worker.join();
            }
        }

        void ThreadPool::enqueue(std::function<void()> task)
        {
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                if (stop_)
                {
                    throw std::runtime_error("enqueue on stopped ThreadPool");
                }
                tasks_.emplace(std::move(task));
            }
            condition_.notify_one();
        }

        void ThreadPool::wait_for_completion(size_t expected_tasks)
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            completion_condition_.wait(lock, [this, expected_tasks]
                                       { return this->completed_tasks_ >= expected_tasks; });
            completed_tasks_ = 0; // Reset for the next batch
        }

    } // namespace common
} // namespace hcle