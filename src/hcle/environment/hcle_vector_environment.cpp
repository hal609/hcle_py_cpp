#include <future> // For std::async
#include <stdexcept>

#include "hcle/environment/hcle_vector_environment.hpp"

namespace hcle
{
    namespace environment
    {

        HCLEVectorEnvironment::HCLEVectorEnvironment(
            const int num_envs,
            const std::string &rom_path,
            const std::string &game_name,
            const std::string &render_mode,
            const uint8_t obs_height,
            const uint8_t obs_width,
            const uint8_t frame_skip,
            const bool maxpool,
            const bool grayscale,
            const uint8_t stack_num)
            : rom_path_(rom_path),
              game_name_(game_name),
              render_mode_(render_mode),
              obs_height_(obs_height),
              obs_width_(obs_width),
              frame_skip_(frame_skip),
              maxpool_(maxpool),
              grayscale_(grayscale),
              stack_num_(stack_num),
              num_envs_(num_envs)
        {
            if (num_envs <= 0)
            {
                throw std::invalid_argument("Number of environments must be positive.");
            }
            // Create environments in the main thread
            for (int i = 0; i < num_envs_; ++i)
            {
                // Only render the first environment if in human mode
                std::string current_render_mode = (i == 0) ? render_mode_ : "rgb_array";
                auto env = std::make_unique<PreprocessedEnv>(
                    rom_path_,
                    game_name_,
                    current_render_mode,
                    obs_height_,
                    obs_width_,
                    frame_skip_,
                    maxpool_,
                    grayscale_,
                    stack_num_);
                envs_.push_back(std::move(env));
            }

            // Start worker threads
            for (int i = 0; i < num_envs_; ++i)
            {
                workers_.emplace_back(&HCLEVectorEnvironment::worker_function, this);
            }
        }

        HCLEVectorEnvironment::~HCLEVectorEnvironment()
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

        void HCLEVectorEnvironment::worker_function()
        {
            while (!stop_)
            {
                Action work = action_queue_.pop();
                if (stop_ || work.env_id < 0)
                {
                    break; // Exit signal
                }

                Result result;

                // Get the environment for this worker's task
                auto &env = envs_[work.env_id];

                if (work.force_reset)
                {
                    env->reset();
                }
                else
                {
                    result.reward = env->step(work.action_value);
                }

                // Prepare result and push to the result queue
                // result.reward = env->getReward();
                result.env_id = work.env_id;
                // Note: We get reward/done *before* getting the screen for the next state
                result.done = env->isDone();

                // The observation is written directly to the buffer in recv()
                // to avoid an extra copy. We just push the other results.
                result_queue_.push(result);
            }
        }

        void HCLEVectorEnvironment::reset(uint8_t *obs_buffer)
        {
            // Send reset tasks to all workers
            for (int i = 0; i < num_envs_; ++i)
            {
                action_queue_.push({i, 0, true});
            }
            // Wait for results and populate the initial observation buffer
            std::vector<float> dummy_rewards(num_envs_);
            std::vector<uint8_t> dummy_dones(num_envs_);
            step_wait(obs_buffer, dummy_rewards.data(), dummy_dones.data());
        }

        size_t HCLEVectorEnvironment::getActionSpaceSize() const
        {
            if (envs_.empty())
            {
                return 0;
            }
            return envs_[0]->getActionSet().size();
        }

        const std::tuple<int, int, int, int> HCLEVectorEnvironment::get_observation_shape() const
        {
            if (this->grayscale_)
            {
                return std::make_tuple(stack_num_, obs_height_, obs_width_, 0);
            }
            else
            {
                return std::make_tuple(stack_num_, obs_height_, obs_width_, 3);
            }
        }

        void HCLEVectorEnvironment::step_async(const std::vector<uint8_t> &actions)
        {
            if (actions.size() != num_envs_)
            {
                throw std::runtime_error("Number of actions must equal number of environments.");
            }
            for (int i = 0; i < num_envs_; ++i)
            {
                action_queue_.push({i, actions[i], false});
            }
        }

        void HCLEVectorEnvironment::step_wait(
            uint8_t *obs_buffer, float *reward_buffer, uint8_t *done_buffer)
        {
            const size_t obs_size = 240 * 256 * 3;

            for (int i = 0; i < num_envs_; ++i)
            {
                Result result = result_queue_.pop();

                // Get the observation for the new state
                envs_[result.env_id]->getScreenRGB(obs_buffer + (result.env_id * obs_size));

                // Copy the other results to their buffers
                reward_buffer[result.env_id] = result.reward;
                done_buffer[result.env_id] = result.done;
            }
        }

        void HCLEVectorEnvironment::step(
            const std::vector<uint8_t> &actions,
            uint8_t *obs_buffer,
            float *reward_buffer,
            uint8_t *done_buffer)
        {
            this->step_async(actions);
            this->step_wait(obs_buffer, reward_buffer, done_buffer);
        }
    } // namespace environment
} // namespace hcle