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

            //printf("Vectorizer constructor running with num_envs = %d\n", num_envs_);
            // Create environments
            envs_.resize(num_envs_);
            for (int i = 0; i < num_envs_; ++i)
            {
                //printf("Vectorizer creating environment %d\n", i);
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
                //printf("Vectorizer starting worker thread %d\n", i);
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
            //printf("Running reset func in vectorizer. About to enqueue reset commands.\n");
            // Enqueue a reset command for every environment
            for (int i = 0; i < num_envs_; ++i)
            {
                action_queue_.push({i, 0, true});
            }
            //printf("Reset commands enqueued. Waiting for results, about to collect results.\n");
            // Wait for all results and return them
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
            // Wait for all results and return them
            return collect_results();
        }

        const std::vector<uint8_t> getActionSet() const
        {
            //printf("Execution in AsyncVectorizer::getActionSet\n");
            return std::vector<uint8_t>({0}); // Placeholder, should return the actual action set
            // return action_set_cache_;
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

                // 1. Perform the action (step or reset)
                if (work.force_reset)
                {
                    env->reset();
                }
                else
                {
                    env->step(work.action_value);
                }

                // 2. Get the resulting state from the environment
                hcle::vector::Timestep timestep = env->get_timestep();
                timestep.env_id = work.env_id; // Assign the env_id to the result
                timestep.done = env->isDone(); // Get the 'done' status
                // printf("Timestep result for env %d: reward=%.2f, done=%d\n", timestep.env_id, timestep.reward, timestep.done);
                // 3. Push the complete result to the queue
                result_queue_.push(timestep);
            }
        }

        // --- MODIFIED RESULT COLLECTION ---
        std::vector<hcle::vector::Timestep> collect_results()
        {
            //printf("==== COLLECTING RESULTS FROM WORKER THREADS ====\n");
            std::vector<hcle::vector::Timestep> results(num_envs_);
            for (int i = 0; i < num_envs_; ++i)
            {
                //printf("Waiting for result %d of %d\n", i + 1, num_envs_);

                // Pop the Timestep directly from the queue
                hcle::vector::Timestep timestep = result_queue_.pop();
                //printf("popped timestep result from results_queque for env id %d, reward=%d, done=%d\n", timestep.env_id, timestep.reward, timestep.done);
                // Use the env_id from the Timestep struct to sort the results
                if (timestep.env_id >= 0 && timestep.env_id < num_envs_)
                {
                    //printf("Received result for env %d, about to place.\n", timestep.env_id);
                    results[timestep.env_id] = std::move(timestep);
                    //printf("Received and placed result for env %d\n", results[timestep.env_id].env_id);
                }
            }
            // printf("All results collected.\n");
            return results;
        }
    };
}