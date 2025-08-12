#include <stdexcept>

#include <opencv2/opencv.hpp>

#include "hcle/environment/preprocessed_env.hpp"
#include "hcle/games/smb1.hpp"

namespace hcle
{
    namespace environment
    {

        PreprocessedEnv::PreprocessedEnv(
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
              stack_num_(stack_num)
        {
            env_ = std::make_unique<HCLEnvironment>();
            env_->loadROM(rom_path, render_mode);
            action_set_ = env_->getActionSet();
        }

        void PreprocessedEnv::reset()
        {
            env_->reset();
        }

        float PreprocessedEnv::step(uint8_t action_index)
        {
            if (action_index >= action_set_.size())
                throw std::out_of_range("Action index out of range.");

            if (typeid(action_index) != typeid(uint8_t))
                throw std::invalid_argument("Action index must be of type uint8_t.");

            uint8_t controller_input = action_set_[action_index];
            if (this->isDone())
            {
                this->reset();
            }

            return env_->act(controller_input);
        }

        bool PreprocessedEnv::isDone()
        {
            return env_->isDone();
        }

        const std::vector<uint8_t> &PreprocessedEnv::getActionSet()
        {
            return env_->getActionSet();
        }

        void PreprocessedEnv::getScreenRGB(uint8_t *buffer) const
        {
            env_->getScreenRGB(buffer);
        }

        std::vector<uint8_t> PreprocessedEnv::getRAM()
        {
            return env_->getRAM();
        }

    } // namespace environment
} // namespace hcle