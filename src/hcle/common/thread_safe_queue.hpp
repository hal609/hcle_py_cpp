#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

namespace hcle
{
    namespace common
    {

        template <typename T>
        class ThreadSafeQueue
        {
        public:
            void push(T item)
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_queue.push(std::move(item));
                m_cond.notify_one();
            }

            T pop()
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_cond.wait(lock, [this]
                            { return !m_queue.empty(); });
                T item = std::move(m_queue.front());
                m_queue.pop();
                return item;
            }

        private:
            std::queue<T> m_queue;
            std::mutex m_mutex;
            std::condition_variable m_cond;
        };

    } // namespace common
} // namespace hcle