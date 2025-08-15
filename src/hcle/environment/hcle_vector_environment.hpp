#pragma once

#include <vector>
#include <memory>
#include <string>
#include <functional>

#include "hcle/environment/async_vectorizer.hpp"
#include "hcle/environment/preprocessed_env.hpp"

namespace hcle::environment
{
    /**
     * @brief The user-facing interface for creating and interacting with a
     * collection of parallel game environments.
     */
    class HCLEVectorEnvironment
    {
    public:
        HCLEVectorEnvironment(
            const int num_envs,
            const std::string &rom_path,
            const std::string &game_name,
            const std::string &render_mode = "rgb_array",
            const int obs_height = 84,
            const int obs_width = 84,
            const int frame_skip = 4,
            const bool maxpool = false,
            const bool grayscale = true,
            const int stack_num = 4)
        {
            // Create a factory function that the AsyncVectorizer can use to
            // construct PreprocessedEnv instances with the correct settings.
            auto env_factory = [=](int env_id)
            {
                // Only the first environment should render to a window if requested.
                std::string current_render_mode = (env_id == 0) ? render_mode : "rgb_array";
                return std::make_unique<PreprocessedEnv>(
                    rom_path, game_name, current_render_mode, obs_height, obs_width,
                    frame_skip, maxpool, grayscale, stack_num);
            };

            // Create and own the vectorizer engine.
            vectorizer_ = std::make_unique<AsyncVectorizer>(num_envs, env_factory);
        }

        // --- Public API (Pass-through calls to the vectorizer) ---

        void reset(uint8_t *obs_buffer, float *reward_buffer, uint8_t *done_buffer)
        {
            vectorizer_->reset(obs_buffer, reward_buffer, done_buffer);
        }

        void send(const std::vector<int> &action_ids)
        {
            vectorizer_->send(action_ids);
        }

        void recv(uint8_t *obs_buffer, float *reward_buffer, uint8_t *done_buffer)
        {
            vectorizer_->recv(obs_buffer, reward_buffer, done_buffer);
        }

        const std::vector<uint8_t> &getActionSet() const
        {
            return vectorizer_->getActionSet();
        }

        size_t getObservationSize() const
        {
            return vectorizer_->getObservationSize();
        }

    private:
        std::unique_ptr<AsyncVectorizer> vectorizer_;
    };
}
