#pragma once

#include "hcle/environment/hcle_environment.hpp"
#include "hcle/common/thread_pool.hpp"
#include <vector>
#include <memory>
#include <string>

namespace hcle
{
    namespace environment
    {

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

            // Resets all environments and returns the initial observations
            void reset(uint8_t *obs_buffer);

            size_t getActionSpaceSize() const;

            // Steps all environments in parallel
            void step(
                const std::vector<uint8_t> &actions,
                uint8_t *obs_buffer,
                float *reward_buffer,
                bool *done_buffer);

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

            std::vector<std::unique_ptr<HCLEnvironment>> envs_;
            hcle::common::ThreadPool thread_pool_;
        };

    } // namespace environment
} // namespace hcle