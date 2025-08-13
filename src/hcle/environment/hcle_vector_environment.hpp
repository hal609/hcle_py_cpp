#pragma once

#include <vector>
#include <memory>
#include <string>
#include <atomic>

#include "hcle/environment/hcle_environment.hpp"
#include "hcle/environment/preprocessed_env.hpp"
#include "hcle/common/thread_safe_queue.hpp"

namespace hcle
{
    namespace environment
    {
        // Struct to send work to worker threads
        struct Action
        {
            int env_id;
            uint8_t action_value;
            bool force_reset = false; // Flag to signal a reset
        };

        // Struct to receive results from worker threads
        struct Result
        {
            int env_id;
            float reward;
            bool done;
            // The observation is moved into the buffer directly, so not needed here
        };

        struct StepResult
        {
            std::vector<uint8_t> observation;
            float reward;
            bool done;
        };

        class HCLEVectorEnvironment
        {
        public:
            HCLEVectorEnvironment(
                const int num_envs,
                const std::string &rom_path,
                const std::string &game_name,
                const std::string &render_mode,
                const uint8_t obs_height = 84,
                const uint8_t obs_width = 84,
                const uint8_t frame_skip = 4,
                const bool maxpool = true,
                const bool grayscale = true,
                const uint8_t stack_num = 4);

            ~HCLEVectorEnvironment();

            // Resets all environments and returns the initial observations
            void reset(uint8_t *obs_buffer);

            size_t getActionSpaceSize() const;
            const std::tuple<int, int, int, int> get_observation_shape() const;

            // Asynchronously sends actions to the environments
            void step_async(const std::vector<uint8_t> &actions);

            // Synchronously waits for and receives results from the environments
            void step_wait(uint8_t *obs_buffer, float *reward_buffer, uint8_t *done_buffer);

            // Steps all environments in parallel
            void step(
                const std::vector<uint8_t> &actions,
                uint8_t *obs_buffer,
                float *reward_buffer,
                uint8_t *done_buffer);

            int getNumEnvs() const { return num_envs_; }

        private:
            const int num_envs_;
            const std::string &rom_path_;
            const std::string &game_name_;
            const std::string &render_mode_;
            const uint8_t obs_height_;
            const uint8_t obs_width_;
            const uint8_t frame_skip_;
            const bool maxpool_;
            const bool grayscale_;
            const uint8_t stack_num_;

            std::vector<std::unique_ptr<PreprocessedEnv>> envs_;

            std::atomic<bool> m_step_in_flight;

            // Private worker function for threads
            void worker_function();

            // --- NEW MEMBERS FOR PIPELINE ---
            common::ThreadSafeQueue<Action> action_queue_;
            common::ThreadSafeQueue<Result> result_queue_;
            std::vector<std::thread> workers_;
            std::atomic<bool> stop_;
        };

    } // namespace environment
} // namespace hcle