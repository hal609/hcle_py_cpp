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
                int num_envs,
                const std::string &rom_path,
                const std::string &game_name);

            // Resets all environments and returns the initial observations
            void reset(uint8_t *obs_buffer);

            int getActionSpaceSize() const;

            // Steps all environments in parallel
            void step(
                const std::vector<uint8_t> &actions,
                uint8_t *obs_buffer,
                float *reward_buffer,
                bool *done_buffer);

            int getNumEnvs() const { return num_envs_; }

        private:
            int num_envs_;
            std::vector<std::unique_ptr<HCLEnvironment>> envs_;
            hcle::common::ThreadPool thread_pool_;
        };

    } // namespace environment
} // namespace hcle