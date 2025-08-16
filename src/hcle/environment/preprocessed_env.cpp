#include <stdexcept>
#include <chrono>

#include <opencv2/opencv.hpp>
#include "hcle/environment/preprocessed_env.hpp"

namespace hcle::environment
{
    PreprocessedEnv::PreprocessedEnv(
        const std::string &rom_path,
        const std::string &game_name,
        const std::string &render_mode,
        const int obs_height,
        const int obs_width,
        const int frame_skip,
        const bool maxpool,
        const bool grayscale,
        const int stack_num)
        // Initialize members from parameters
        : obs_height_(obs_height),
          obs_width_(obs_width),
          frame_skip_(frame_skip),
          maxpool_((frame_skip_ > 1) && maxpool),
          grayscale_(grayscale),
          stack_num_(stack_num),
          reward_(0.0f),
          done_(false)
    {
        env_ = std::make_unique<HCLEnvironment>();
        env_->loadROM(rom_path, render_mode);
        if (grayscale_)
            env_->setOutputModeGrayscale();

        action_set_ = env_->getActionSet();

        // The final observation size depends on the preprocessing options.
        m_channels_per_frame = grayscale_ ? 1 : 3;
        m_raw_size = m_raw_frame_height * m_raw_frame_width * m_channels_per_frame;
        m_obs_size = obs_height_ * obs_width_ * m_channels_per_frame;
        m_stacked_obs_size = stack_num_ * m_obs_size;

        // Allocate buffers with the correct sizes.
        m_raw_frame.resize(m_raw_size);
        m_frame_stack.resize(m_stacked_obs_size, 0);
        m_frame_stack_idx = 0;

        requires_resize_ = (obs_height_ != m_raw_frame_height) || (obs_width_ != m_raw_frame_width);
    }

    void PreprocessedEnv::reset(uint8_t *obs_output_buffer)
    {
        env_->reset();
        reward_ = 0.0f;
        done_ = false;

        // Get the initial screen from the emulator.
        env_->getFrameBufferData(m_raw_frame.data(), false);

        m_frame_stack_idx = 0;
        process_screen();

        uint8_t *first_frame_ptr = m_frame_stack.data();

        for (int i = 1; i < stack_num_; ++i)
        {
            std::memcpy(m_frame_stack.data() + (i * m_obs_size),
                        first_frame_ptr,
                        m_obs_size);
        }
        std::memcpy(
            obs_output_buffer,
            m_frame_stack.data(),
            m_stacked_obs_size);
    }

    void PreprocessedEnv::step(uint8_t action_index, uint8_t *obs_output_buffer)
    {
        if (action_index >= action_set_.size())
        {
            throw std::out_of_range("Action index out of range.");
        }

        uint8_t controller_input = action_set_[action_index];
        float accumulated_reward = 0.0f;

        // --- FRAME SKIPPING LOOP ---
        for (int skip = 0; skip < frame_skip_; ++skip)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            accumulated_reward += env_->act(controller_input);
            done_ = env_->isDone();

            if (done_)
                break;

            // Capture last two frames for max-pooling directly from emu
            if (maxpool_)
            {
                if (skip == frame_skip_ - 2)
                {
                    env_->getFrameBufferData(m_raw_frame.data(), false);
                }
                if (skip == frame_skip_ - 1)
                {
                    env_->getFrameBufferData(m_raw_frame.data(), true);
                }
            }
        }
        reward_ = accumulated_reward;

        if (!maxpool_)
            env_->getFrameBufferData(m_raw_frame.data(), false);

        process_screen();
        write_observation(obs_output_buffer);
    }

    void PreprocessedEnv::process_screen()
    {
        auto cv2_format = grayscale_ ? CV_8UC1 : CV_8UC3;
        cv::Mat source_mat(m_raw_frame_height, m_raw_frame_width, cv2_format, m_raw_frame.data());

        // Get pointer to current position in circular frame stack
        uint8_t *dest_ptr = m_frame_stack.data() + (m_frame_stack_idx * m_obs_size);

        if (requires_resize_)
        {
            cv::Mat dest_mat(obs_height_, obs_width_, cv2_format, dest_ptr);
            cv::resize(source_mat, dest_mat, dest_mat.size(), 0, 0, cv::INTER_AREA);
        }
        else
        {
            std::memcpy(dest_ptr, source_mat.data, m_obs_size);
        }

        // 4. Move to the next position in the circular buffer.
        m_frame_stack_idx = (m_frame_stack_idx + 1) % stack_num_;
    }

    void PreprocessedEnv::write_observation(uint8_t *obs_output_buffer)
    {
        if (m_frame_stack_idx == 0)
        {
            std::memcpy(obs_output_buffer, m_frame_stack.data(), m_stacked_obs_size);
        }
        else
        {
            size_t older_part_size = (stack_num_ - m_frame_stack_idx) * m_obs_size;
            std::memcpy(obs_output_buffer,
                        m_frame_stack.data() + (m_frame_stack_idx * m_obs_size),
                        older_part_size);

            size_t newer_part_size = m_frame_stack_idx * m_obs_size;
            std::memcpy(obs_output_buffer + older_part_size,
                        m_frame_stack.data(),
                        newer_part_size);
        }
    }

    // --- Getters ---
    bool PreprocessedEnv::isDone() const { return done_; }
    float PreprocessedEnv::getReward() const { return reward_; }
    std::vector<uint8_t> PreprocessedEnv::getActionSet() const { return action_set_; }
    size_t PreprocessedEnv::getObservationSize() const { return m_stacked_obs_size; }

} // namespace environment
