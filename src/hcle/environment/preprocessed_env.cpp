#include <stdexcept>
#include <opencv2/opencv.hpp>
#include "hcle/environment/preprocessed_env.hpp"

namespace hcle::environment
{
    PreprocessedEnv::PreprocessedEnv(
        const std::string &rom_path,
        const std::string &game_name,
        const int obs_height,
        const int obs_width,
        const int frame_skip,
        const bool maxpool,
        const bool grayscale,
        const int stack_num,
        const int max_episode_steps,
        const bool color_index_grayscale)
        : m_obs_height(obs_height),
          m_obs_width(obs_width),
          m_frame_skip(frame_skip),
          m_maxpool((m_frame_skip > 1) && maxpool),
          m_grayscale(grayscale),
          m_stack_num(stack_num),
          m_max_episode_steps(max_episode_steps),
          m_reward(0.0f),
          m_done(false)
    {
        m_env = std::make_unique<HCLEnvironment>();
        m_env->loadROM(game_name);

        if (m_grayscale)
            m_env->setOutputMode((color_index_grayscale) ? "index" : "grayscale");

        m_action_set = m_env->getActionSet();

        // The final observation size depends on the preprocessing options.
        m_channels_per_frame = m_grayscale ? 1 : 3;
        m_raw_size = m_raw_frame_height * m_raw_frame_width * m_channels_per_frame;
        m_obs_size = m_obs_height * m_obs_width * m_channels_per_frame;
        m_stacked_obs_size = m_stack_num * m_obs_size;

        // Allocate buffers with the correct sizes.
        m_prev_frame.resize(m_raw_size, 0);
        m_frame_stack.resize(m_stacked_obs_size, 0);
        m_frame_stack_idx = 0;

        m_requires_resize = (m_obs_height != m_raw_frame_height) || (m_obs_width != m_raw_frame_width);
    }

    void PreprocessedEnv::reset(uint8_t *obs_output_buffer)
    {
        m_env->reset();
        m_reward = 0.0f;
        m_done = false;
        m_step_count = 0;

        m_frame_stack_idx = 0;
        processScreen();

        for (int i = 1; i < m_stack_num; ++i)
        {
            std::memcpy(m_frame_stack.data() + (i * m_obs_size),
                        m_frame_stack.data(),
                        m_obs_size);
        }
        writeObservation(obs_output_buffer);
    }

    void PreprocessedEnv::step(uint8_t action_index, uint8_t *obs_output_buffer)
    {
        if (action_index >= m_action_set.size())
        {
            throw std::out_of_range("Action index out of range.");
        }
        m_step_count++;

        uint8_t controller_input = m_action_set[action_index];
        double accumulated_reward = 0.0f;

        if (m_maxpool)
        {
            accumulated_reward += m_env->act(controller_input, m_frame_skip - 1);
            std::memcpy(m_prev_frame.data(), m_env->frame_ptr, m_raw_size);
            accumulated_reward += m_env->act(controller_input, 1);
        }
        else
        {
            accumulated_reward += m_env->act(controller_input, m_frame_skip);
        }
        m_done = m_env->isDone();
        if (m_max_episode_steps > 0 && m_step_count > m_max_episode_steps)
        {
            m_done = true;
        }
        m_reward = accumulated_reward;

        processScreen();

        writeObservation(obs_output_buffer);
    }

    void PreprocessedEnv::processScreen()
    {
        auto cv2_format = m_grayscale ? CV_8UC1 : CV_8UC3;
        uint8_t *frame_pointer = const_cast<uint8_t *>(m_env->frame_ptr);

        if (m_maxpool)
        {
            frame_pointer = std::max(frame_pointer, m_prev_frame.data());
        }
        cv::Mat source_mat = cv::Mat(m_raw_frame_height, m_raw_frame_width, cv2_format, frame_pointer);

        // Get pointer to current position in circular buffer
        uint8_t *dest_ptr = m_frame_stack.data() + (m_frame_stack_idx * m_obs_size);

        if (m_requires_resize)
        {
            cv::Mat dest_mat(m_obs_height, m_obs_width, cv2_format, dest_ptr);
            cv::resize(source_mat, dest_mat, dest_mat.size(), 0, 0, cv::INTER_AREA);
        }
        else
        {
            std::memcpy(dest_ptr, source_mat.data, m_obs_size);
        }

        // Move to next position in circular buffer
        m_frame_stack_idx = (m_frame_stack_idx + 1) % m_stack_num;
    }

    void PreprocessedEnv::writeObservation(uint8_t *obs_output_buffer)
    {
        if (m_frame_stack_idx == 0)
        {
            std::memcpy(obs_output_buffer, m_frame_stack.data(), m_stacked_obs_size);
        }
        else
        {
            size_t older_part_size = (m_stack_num - m_frame_stack_idx) * m_obs_size;
            std::memcpy(obs_output_buffer,
                        m_frame_stack.data() + (m_frame_stack_idx * m_obs_size),
                        older_part_size);

            size_t newer_part_size = m_frame_stack_idx * m_obs_size;
            std::memcpy(obs_output_buffer + older_part_size,
                        m_frame_stack.data(),
                        newer_part_size);
        }
    }

    void PreprocessedEnv::saveToState(int state_num)
    {
        m_env->saveToState(state_num);
    }

    void PreprocessedEnv::loadFromState(int state_num)
    {
        m_env->loadFromState(state_num);
    }

    void PreprocessedEnv::createWindow(uint8_t fps_limit)
    {
        m_env->createWindow(fps_limit);
    }

    void PreprocessedEnv::updateWindow()
    {
        m_env->updateWindow();
    }

    // --- Getters ---

} // namespace environment
