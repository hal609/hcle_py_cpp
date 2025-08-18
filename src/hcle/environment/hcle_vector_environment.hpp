#pragma once

#include <vector>
#include <memory>
#include <string>
#include <functional>

#include "hcle/common/display.hpp"
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
            : render_mode_(render_mode) // Store the render mode
        {
            // Only create a display window if in "human" mode.
            if (render_mode_ == "human")
            {
                display_ = std::make_unique<hcle::common::Display>("HCLEnvironment", 256, 240, 3);
            }

            // Create a factory function that the AsyncVectorizer can use to
            // construct PreprocessedEnv instances.
            // CORRECTED: Explicitly capture 'this' to fix the compiler warning.
            auto env_factory = [=]([[maybe_unused]] int env_id)
            {
                // The underlying environments always run in "rgb_array" mode for performance.
                return std::make_unique<PreprocessedEnv>(
                    rom_path, game_name, obs_height, obs_width,
                    frame_skip, maxpool, grayscale, stack_num);
            };

            // Create and own the vectorizer engine.
            vectorizer_ = std::make_unique<AsyncVectorizer>(num_envs, env_factory);
        }

        void render()
        {
            if (render_mode_ == "human" && display_)
            {
                const uint8_t *frame_ptr = vectorizer_->getRawFramePointer(0);
                if (frame_ptr)
                {
                    display_->update(frame_ptr);
                    if (display_->processEvents())
                    {
                        throw hcle::common::WindowClosedException();
                    }
                }
            }
        }

        void reset(uint8_t *obs_buffer, float *reward_buffer, uint8_t *done_buffer)
        {
            vectorizer_->reset(obs_buffer, reward_buffer, done_buffer);
        }

        void send(const std::vector<int> &action_ids)
        {
            render();
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

        int getNumEnvs() const { return vectorizer_->getNumEnvs(); }

    private:
        std::unique_ptr<AsyncVectorizer> vectorizer_;
        std::unique_ptr<hcle::common::Display> display_;
        std::string render_mode_;
    };
}
