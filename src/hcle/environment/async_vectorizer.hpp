#pragma once

#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include <functional>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <execution>

#include "hcle/common/thread_safe_queue.hpp"
#include "hcle/environment/preprocessed_env.hpp"

namespace hcle::environment
{
    class AsyncVectorizer
    {
    public:
        AsyncVectorizer(
            const int num_envs,
            const std::function<std::unique_ptr<PreprocessedEnv>(int)> &env_factory) : m_num_envs(num_envs), m_stop(false)
        {
            if (num_envs <= 0)
                throw std::invalid_argument("Number of environments must be positive.");

            m_envs.reserve(m_num_envs);
            for (int i = 0; i < m_num_envs; ++i)
            {
                m_envs.push_back(env_factory(i));
            }

            if (m_envs.empty())
                throw std::runtime_error("Environment creation failed.");

            m_action_set_cache = m_envs[0]->getActionSet();

            // Pre-allocate internal buffers to avoid allocations in main loop
            const size_t single_obs_size = getObservationSize();
            m_internal_obs_buffers.resize(m_num_envs);
            for (int i = 0; i < m_num_envs; ++i)
            {
                m_internal_obs_buffers[i].resize(single_obs_size);
            }
            m_internal_reward_buffers.resize(m_num_envs);
            m_internal_done_buffers.resize(m_num_envs);

            const std::size_t processor_count = std::thread::hardware_concurrency();
            m_num_threads = std::min<int>(m_num_envs, static_cast<int>(processor_count));

            // Start worker threads
            m_workers.reserve(m_num_threads);
            for (int i = 0; i < m_num_threads; ++i)
            {
                m_workers.emplace_back([this]
                                       { workerFunction(); });
            }
        }

        ~AsyncVectorizer()
        {
            m_stop = true;
            // Push dummy actions to wake up workers
            for (int i = 0; i < m_num_threads; ++i)
            {
                m_action_queue.push({-1, 0, false});
            }
            // Wait for all worker threads to terminate
            for (auto &worker : m_workers)
            {
                if (worker.joinable())
                {
                    worker.join();
                }
            }
        }

        void reset(uint8_t *obs_buffer, double *reward_buffer, uint8_t *done_buffer)
        {
            for (int i = 0; i < m_num_envs; ++i)
            {
                m_action_queue.push({i, 0, true});
            }
            collectResults(obs_buffer, reward_buffer, done_buffer);
        }

        void send(const std::vector<int> &action_ids)
        {
            if (static_cast<int>(action_ids.size()) != m_num_envs)
            {
                throw std::runtime_error("Number of actions must equal number of environments.");
            }
            // Queue a step command for every environment.
            for (int i = 0; i < m_num_envs; ++i)
            {
                m_action_queue.push({i, static_cast<uint8_t>(action_ids[i]), false});
            }
        }

        const uint8_t *getRawFramePointer(int index) { return m_envs[index]->getFramePointer(); }

        void recv(uint8_t *obs_buffer, double *reward_buffer, uint8_t *done_buffer) { collectResults(obs_buffer, reward_buffer, done_buffer); }

        const std::vector<uint8_t> &getActionSet() const { return m_action_set_cache; }

        size_t getObservationSize() const
        {
            if (m_envs.empty())
                return 0;
            return m_envs[0]->getObservationSize();
        }

        int getNumEnvs() const { return m_num_envs; }

        void loadFromState(int state_num)
        {
            std::for_each(
                std::execution::par,
                m_envs.begin(),
                m_envs.end(),
                [](auto &env)
                {
                    env->loadFromState(0);
                });
        }

    private:
        struct ActionTask
        {
            int env_id;
            uint8_t action_value;
            bool force_reset;
        };

        std::vector<uint8_t> m_action_set_cache;
        int m_num_envs;
        int m_num_threads;
        common::ThreadSafeQueue<ActionTask> m_action_queue;
        common::ThreadSafeQueue<int> m_result_queue; // Queue for "work complete" notifications.
        std::vector<std::thread> m_workers;
        std::atomic<bool> m_stop;
        std::vector<std::unique_ptr<PreprocessedEnv>> m_envs;

        // Internal buffers for thread-safe data transfer.
        std::vector<std::vector<uint8_t>> m_internal_obs_buffers;
        std::vector<double> m_internal_reward_buffers;
        std::vector<bool> m_internal_done_buffers;

        void workerFunction()
        {
            while (!m_stop)
            {
                ActionTask work = m_action_queue.pop();
                if (m_stop || work.env_id < 0)
                {
                    break;
                }

                auto &env = m_envs[work.env_id];

                uint8_t *current_obs_buffer = m_internal_obs_buffers[work.env_id].data();

                if (work.force_reset || env->isDone())
                {
                    env->reset(current_obs_buffer);
                }
                else
                {
                    env->step(work.action_value, current_obs_buffer);
                }

                // Store results in internal buffers
                m_internal_reward_buffers[work.env_id] = env->getReward();
                m_internal_done_buffers[work.env_id] = env->isDone();

                m_result_queue.push(work.env_id);
            }
        }

        void collectResults(uint8_t *obs_buffer, double *reward_buffer, uint8_t *done_buffer)
        {
            const size_t single_obs_size = getObservationSize();

            for (int i = 0; i < m_num_envs; ++i)
            {
                int completed_env_id = m_result_queue.pop();

                if (completed_env_id >= 0 && completed_env_id < m_num_envs)
                {
                    // Copy data from internal buffers to python buffers
                    reward_buffer[completed_env_id] = m_internal_reward_buffers[completed_env_id];
                    done_buffer[completed_env_id] = m_internal_done_buffers[completed_env_id];

                    // Copy observation data from internal buffer to the pthon buffer
                    std::memcpy(obs_buffer + (completed_env_id * single_obs_size),
                                m_internal_obs_buffers[completed_env_id].data(),
                                single_obs_size);
                }
            }
        }
    };
}
