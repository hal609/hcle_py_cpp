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
            const int stack_num = 4,
            const bool color_index_grayscale = false)
            : m_render_mode(render_mode),
              m_grayscale(grayscale)
        {

            auto env_factory = [=]([[maybe_unused]] int env_id)
            {
                return std::make_unique<PreprocessedEnv>(
                    rom_path, game_name, obs_height, obs_width,
                    frame_skip, maxpool, grayscale, stack_num, color_index_grayscale);
            };

            // Create and own the vectorizer engine.
            m_vectorizer = std::make_unique<AsyncVectorizer>(num_envs, env_factory);

            // Only create a display window if in "human" mode.
            if (m_render_mode == "human")
            {
                m_display = std::make_unique<hcle::common::Display>("HCLEnvironment", 256, 240, 3);
                m_frame_ptr = m_vectorizer->getRawFramePointer(0);
            }
        }

        void reset(uint8_t *obs_buffer, double *reward_buffer, uint8_t *done_buffer)
        {
            m_vectorizer->reset(obs_buffer, reward_buffer, done_buffer);
        }

        void send(const std::vector<int> &action_ids)
        {
            if (m_render_mode == "human" && m_display && m_frame_ptr)
            {
                hcle::common::Display::update_window(m_display, m_frame_ptr, m_grayscale);
            }
            m_vectorizer->send(action_ids);
        }

        void recv(uint8_t *obs_buffer, double *reward_buffer, uint8_t *done_buffer)
        {
            m_vectorizer->recv(obs_buffer, reward_buffer, done_buffer);
        }

        const std::vector<uint8_t> &getActionSet() const
        {
            return m_vectorizer->getActionSet();
        }

        size_t getObservationSize() const
        {
            return m_vectorizer->getObservationSize();
        }

        int getNumEnvs() const { return m_vectorizer->getNumEnvs(); }

        void loadFromState(int state_num)
        {
            m_vectorizer->loadFromState(state_num);
        }

    private:
        std::unique_ptr<AsyncVectorizer> m_vectorizer;
        std::unique_ptr<hcle::common::Display> m_display;
        std::string m_render_mode;
        const uint8_t *m_frame_ptr = nullptr;

        bool m_grayscale;
    };
}
