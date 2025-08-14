#include "hcle/common/thread_safe_queue.hpp"
#include "hcle/environment/preprocessed_env.hpp"

namespace hcle::environment
{
    class AsyncVectorizer
    {
    public:
        AsyncVectorizer(
            const int num_envs,
            const std::function<std::unique_ptr<PreprocessedEnv>(int)> &env_factory) : num_envs_(num_envs)
        {

            if (num_envs <= 0)
                throw std::invalid_argument("Number of environments must be positive.");

            //  Create environments
            envs_.resize(num_envs_);
            for (int i = 0; i < num_envs_; ++i)
            {
                envs_[i] = env_factory(i);
            }

            // Cache the action set from the first environment
            if (!envs_.empty())
                action_set_cache_ = envs_[0]->getActionSet();

            // Setup worker threads
            const std::size_t processor_count = std::thread::hardware_concurrency();
            num_threads_ = std::min<int>(num_envs_, static_cast<int>(processor_count));

            // Start worker threads
            for (int i = 0; i < num_threads_; ++i)
            {
                workers_.emplace_back([this]
                                      { worker_function(); });
            }
        }

        ~AsyncVectorizer()
        {
            stop_ = true;
            // Push dummy actions to wake up any waiting workers
            for (int i = 0; i < num_envs_; ++i)
            {
                action_queue_.push({-1, 0});
            }
            // Wait for all worker threads to terminate
            for (auto &worker : workers_)
            {
                if (worker.joinable())
                {
                    worker.join();
                }
            }
        }

        std::vector<hcle::vector::Timestep> reset()
        {
            for (int i = 0; i < num_envs_; ++i)
            {
                action_queue_.push({i, 0, true});
            }
            return collect_results();
        }

        void send(const std::vector<int> &action_ids)
        {
            if (action_ids.size() != num_envs_)
            {
                throw std::runtime_error("Number of actions must equal number of environments.");
            }
            // Enqueue a step command for every environment
            for (int i = 0; i < num_envs_; ++i)
            {
                action_queue_.push({i, static_cast<uint8_t>(action_ids[i]), false});
            }
        }

        std::vector<hcle::vector::Timestep> recv()
        {
            return collect_results();
        }

        const std::vector<uint8_t> getActionSet() const
        {
            return action_set_cache_;
        }

        // --- PRIVATE HELPER ---
    private:
        std::vector<uint8_t> action_set_cache_;
        int num_envs_;
        int num_threads_;
        common::ThreadSafeQueue<hcle::vector::Action> action_queue_;
        common::ThreadSafeQueue<hcle::vector::Timestep> result_queue_;
        std::vector<std::thread> workers_;
        std::atomic<bool> stop_;
        std::vector<std::unique_ptr<PreprocessedEnv>> envs_; // Environment instances

        mutable std::vector<std::vector<uint8_t>> final_obs_storage_; // For same-step autoreset

        void worker_function()
        {
            while (!stop_)
            {
                hcle::vector::Action work = action_queue_.pop();
                if (stop_ || work.env_id < 0)
                {
                    break;
                }

                auto &env = envs_[work.env_id];

                if (work.force_reset)
                {
                    env->reset();
                }
                else
                {
                    env->step(work.action_value);
                }

                hcle::vector::Timestep timestep = env->get_timestep();
                timestep.env_id = work.env_id;
                timestep.done = env->isDone();

                result_queue_.push(timestep);
            }
        }

        std::vector<hcle::vector::Timestep> collect_results()
        {
            std::vector<hcle::vector::Timestep> results(num_envs_);
            for (int i = 0; i < num_envs_; ++i)
            {
                hcle::vector::Timestep timestep = result_queue_.pop();

                if (timestep.env_id >= 0 && timestep.env_id < num_envs_)
                {
                    results[timestep.env_id] = std::move(timestep);
                }
            }
            return results;
        }
    };
}