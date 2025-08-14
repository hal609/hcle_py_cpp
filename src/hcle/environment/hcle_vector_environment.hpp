#pragma once

#include <vector>
#include <memory>
#include <string>

#include "hcle/environment/async_vectorizer.hpp"

namespace hcle::environment
{
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
            const uint8_t stack_num = 4) : num_envs_(num_envs),
                                           rom_path_(rom_path),
                                           game_name_(game_name),
                                           render_mode_(render_mode),
                                           obs_height_(obs_height),
                                           obs_width_(obs_width),
                                           frame_skip_(frame_skip),
                                           maxpool_(maxpool),
                                           grayscale_(grayscale),
                                           stack_num_(stack_num)
        {
            auto env_factory = [=](int env_id)
            {
                std::string current_render_mode = (env_id == 0) ? render_mode : "rgb_array";
                return std::make_unique<PreprocessedEnv>(
                    rom_path, game_name, current_render_mode, obs_height, obs_width,
                    frame_skip, maxpool, grayscale, stack_num);
            };

            vectorizer_ = std::make_unique<AsyncVectorizer>(num_envs, env_factory);
        }

        int getNumEnvs() const { return num_envs_; }
        void send(const std::vector<int> &action_ids) { vectorizer_->send(action_ids); }
        std::vector<hcle::vector::Timestep> recv() { return vectorizer_->recv(); }
        std::vector<uint8_t> getActionSet() const { return vectorizer_->getActionSet(); }

        std::vector<hcle::vector::Timestep> reset()
        {
            return vectorizer_->reset();
        }

        const std::tuple<int, int, int, int> get_observation_shape() const
        {
            if (grayscale_)
            {
                return std::make_tuple(stack_num_, obs_height_, obs_width_, 1);
            }
            else
            {
                return std::make_tuple(stack_num_, obs_height_, obs_width_, 3);
            }
        }

    private:
        std::unique_ptr<AsyncVectorizer> vectorizer_;
        std::vector<uint8_t> action_set_;

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
    };
}