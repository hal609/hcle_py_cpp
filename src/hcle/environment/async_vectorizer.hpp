#pragma once

#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include <functional>
#include <string>
#include <algorithm> // For std::min
#include <stdexcept> // For std::runtime_error

#include "hcle/common/thread_safe_queue.hpp"
#include "hcle/environment/preprocessed_env.hpp"

namespace hcle::environment
{
    /**
     * @brief Manages a collection of environments that can be stepped in parallel.
     * This class handles the asynchronous distribution of actions to worker threads
     * and the collection of results, using a zero-allocation pattern for performance.
     */
    class AsyncVectorizer
    {
    public:
        /**
         * @brief Constructs the AsyncVectorizer.
         * @param num_envs The number of parallel environments to run.
         * @param env_factory A function that creates a single PreprocessedEnv instance.
         */
        AsyncVectorizer(
            const int num_envs,
            const std::function<std::unique_ptr<PreprocessedEnv>(int)> &env_factory) : num_envs_(num_envs), stop_(false)
        {
            if (num_envs <= 0)
                throw std::invalid_argument("Number of environments must be positive.");

            // Create environments using the provided factory.
            envs_.reserve(num_envs_);
            for (int i = 0; i < num_envs_; ++i)
            {
                envs_.push_back(env_factory(i));
            }

            if (envs_.empty())
                throw std::runtime_error("Environment creation failed.");

            // Cache the action set from the first environment.
            action_set_cache_ = envs_[0]->getActionSet();

            // Pre-allocate internal buffers to avoid allocations in the main loop.
            const size_t single_obs_size = getObservationSize();
            internal_obs_buffers_.resize(num_envs_);
            for (int i = 0; i < num_envs_; ++i)
            {
                internal_obs_buffers_[i].resize(single_obs_size);
            }
            internal_reward_buffers_.resize(num_envs_);
            internal_done_buffers_.resize(num_envs_);

            // Determine the number of worker threads to use.
            const std::size_t processor_count = std::thread::hardware_concurrency();
            num_threads_ = std::min<int>(num_envs_, static_cast<int>(processor_count));

            // Start worker threads.
            workers_.reserve(num_threads_);
            for (int i = 0; i < num_threads_; ++i)
            {
                workers_.emplace_back([this]
                                      { worker_function(); });
            }
        }

        /**
         * @brief Destructor. Signals worker threads to stop and joins them.
         */
        ~AsyncVectorizer()
        {
            stop_ = true;
            // Push dummy actions to wake up any waiting workers.
            for (int i = 0; i < num_threads_; ++i)
            {
                action_queue_.push({-1, 0, false});
            }
            // Wait for all worker threads to terminate.
            for (auto &worker : workers_)
            {
                if (worker.joinable())
                {
                    worker.join();
                }
            }
        }

        /**
         * @brief Resets all environments and populates the initial observation buffers.
         * @param obs_buffer Pointer to a pre-allocated buffer to store observations.
         * @param reward_buffer Pointer to a pre-allocated buffer to store rewards.
         * @param done_buffer Pointer to a pre-allocated buffer to store done flags.
         */
        void reset(uint8_t *obs_buffer, float *reward_buffer, uint8_t *done_buffer)
        {
            for (int i = 0; i < num_envs_; ++i)
            {
                action_queue_.push({i, 0, true});
            }
            collect_results(obs_buffer, reward_buffer, done_buffer);
        }

        /**
         * @brief Sends actions to the worker threads to be executed in the environments.
         * @param action_ids A vector of action IDs, one for each environment.
         */
        void send(const std::vector<int> &action_ids)
        {
            if (action_ids.size() != num_envs_)
            {
                throw std::runtime_error("Number of actions must equal number of environments.");
            }
            // Enqueue a step command for every environment.
            for (int i = 0; i < num_envs_; ++i)
            {
                action_queue_.push({i, static_cast<uint8_t>(action_ids[i]), false});
            }
        }

        /**
         * @brief Waits for and collects results from all environments.
         * @param obs_buffer Pointer to a pre-allocated buffer to store observations.
         * @param reward_buffer Pointer to a pre-allocated buffer to store rewards.
         * @param done_buffer Pointer to a pre-allocated buffer to store done flags.
         */
        void recv(uint8_t *obs_buffer, float *reward_buffer, uint8_t *done_buffer)
        {
            collect_results(obs_buffer, reward_buffer, done_buffer);
        }

        /**
         * @brief Gets the action set from the environment.
         * @return A const reference to the cached action set vector.
         */
        const std::vector<uint8_t> &getActionSet() const
        {
            return action_set_cache_;
        }

        /**
         * @brief Gets the size of a single preprocessed observation.
         * @return The size in bytes.
         */
        const size_t getObservationSize() const
        {
            if (envs_.empty())
                return 0;
            return envs_[0]->getObservationSize();
        }

    private:
        // Struct for passing tasks to worker threads.
        struct ActionTask
        {
            int env_id;
            uint8_t action_value;
            bool force_reset;
        };

        std::vector<uint8_t> action_set_cache_;
        int num_envs_;
        int num_threads_;
        common::ThreadSafeQueue<ActionTask> action_queue_;
        common::ThreadSafeQueue<int> result_queue_; // Queue for "work complete" notifications.
        std::vector<std::thread> workers_;
        std::atomic<bool> stop_;
        std::vector<std::unique_ptr<PreprocessedEnv>> envs_;

        // Internal buffers for thread-safe data transfer.
        std::vector<std::vector<uint8_t>> internal_obs_buffers_;
        std::vector<float> internal_reward_buffers_;
        std::vector<bool> internal_done_buffers_;

        /**
         * @brief The main function executed by each worker thread.
         */
        void worker_function()
        {
            while (!stop_)
            {
                ActionTask work = action_queue_.pop();
                if (stop_ || work.env_id < 0)
                {
                    break;
                }

                auto &env = envs_[work.env_id];

                // Get a pointer to this worker's dedicated internal observation buffer.
                uint8_t *current_obs_buffer = internal_obs_buffers_[work.env_id].data();

                // Perform the action (step or reset), writing the observation
                // directly into the internal buffer.
                if (work.force_reset)
                {
                    env->reset(current_obs_buffer);
                }
                else
                {
                    env->step(work.action_value, current_obs_buffer);
                }

                // Store scalar results in their respective internal buffers.
                internal_reward_buffers_[work.env_id] = env->getReward();
                internal_done_buffers_[work.env_id] = env->isDone();

                // Push ONLY the env_id as a "work complete" notification.
                result_queue_.push(work.env_id);
            }
        }

        /**
         * @brief Collects results from all environments after being notified.
         */
        void collect_results(uint8_t *obs_buffer, float *reward_buffer, uint8_t *done_buffer)
        {
            const size_t single_obs_size = getObservationSize();

            for (int i = 0; i < num_envs_; ++i)
            {
                // Pop the completed env_id notification from the queue.
                int completed_env_id = result_queue_.pop();

                if (completed_env_id >= 0 && completed_env_id < num_envs_)
                {
                    // Copy scalar data from internal buffers to the user-provided buffers.
                    reward_buffer[completed_env_id] = internal_reward_buffers_[completed_env_id];
                    done_buffer[completed_env_id] = internal_done_buffers_[completed_env_id];

                    // Safely copy the observation data from our internal buffer to the user's buffer.
                    std::memcpy(obs_buffer + (completed_env_id * single_obs_size),
                                internal_obs_buffers_[completed_env_id].data(),
                                single_obs_size);
                }
            }
        }
    };
}
