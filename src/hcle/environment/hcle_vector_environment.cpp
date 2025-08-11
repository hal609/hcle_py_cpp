#include "hcle/environment/hcle_vector_environment.hpp"
#include <future> // For std::async
#include <stdexcept>

namespace hcle
{
    namespace environment
    {

        HCLEVectorEnvironment::HCLEVectorEnvironment(
            int num_envs,
            const std::string &rom_path,
            const std::string &game_name) : num_envs_(num_envs), thread_pool_(num_envs)
        {

            if (num_envs <= 0)
            {
                throw std::invalid_argument("Number of environments must be positive.");
            }
            // Create 'num_envs' separate environment instances
            for (int i = 0; i < num_envs_; ++i)
            {
                auto env = std::make_unique<HCLEnvironment>();
                // Load each one with the same ROM. Render mode is always "rgb_array" for vector envs.
                env->loadROM(rom_path, game_name, "rgb_array");
                envs_.push_back(std::move(env));
            }
        }

        void HCLEVectorEnvironment::reset(uint8_t *obs_buffer)
        {
            const size_t obs_size = 240 * 256 * 3;

            for (int i = 0; i < num_envs_; ++i)
            {
                thread_pool_.enqueue([this, i, obs_buffer, obs_size]()
                                     {
            envs_[i]->reset();
            envs_[i]->getScreenRGB(obs_buffer + (i * obs_size)); });
            }
            thread_pool_.wait_for_completion(num_envs_);
        }

        int HCLEVectorEnvironment::getActionSpaceSize() const
        {
            if (envs_.empty())
            {
                return 0;
            }
            // Return the size of the action set from the first managed environment
            return envs_[0]->getActionSet().size();
        }

        void HCLEVectorEnvironment::step(
            const std::vector<uint8_t> &actions,
            uint8_t *obs_buffer,
            float *reward_buffer,
            bool *done_buffer)
        {

            const size_t obs_size = 240 * 256 * 3;

            for (int i = 0; i < num_envs_; ++i)
            {
                thread_pool_.enqueue([this, i, &actions, obs_buffer, reward_buffer, done_buffer, obs_size]()
                                     {
            reward_buffer[i] = envs_[i]->act(actions[i]);
            done_buffer[i] = envs_[i]->isDone();
            envs_[i]->getScreenRGB(obs_buffer + (i * obs_size)); });
            }
            thread_pool_.wait_for_completion(num_envs_);
        }
    } // namespace environment
} // namespace hcle