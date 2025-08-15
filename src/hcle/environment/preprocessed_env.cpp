#include <stdexcept>
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

        // Allocate buffers with the correct sizes.
        m_raw_frames.resize(2, std::vector<uint8_t>(m_raw_size));
        m_frame_stack.resize(stack_num_ * m_obs_size, 0);
        m_frame_stack_idx = 0;

        requires_resize_ = (obs_height_ != m_raw_frame_height) || (obs_width_ != m_raw_frame_width);

        maxpool_ = (frame_skip_ > 1) && maxpool;
    }

    void PreprocessedEnv::reset(uint8_t *obs_output_buffer)
    {
        env_->reset();
        reward_ = 0.0f;
        done_ = false;

        std::fill(m_frame_stack.begin(), m_frame_stack.end(), 0);
        m_frame_stack_idx = 0;

        // Get the initial screen from the emulator.
        env_->getFrameBufferData(m_raw_frames[0].data(), false);
        // Copy the first frame to the second buffer for the initial max-pool.
        std::memcpy(m_raw_frames[1].data(), m_raw_frames[0].data(), m_raw_size);

        // Process the initial frame and fill the entire stack with it.
        for (int i = 0; i < stack_num_; ++i)
        {
            process_screen();
        }

        // Write the final stacked observation to the user's buffer.
        write_observation(obs_output_buffer);
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
            accumulated_reward += env_->act(controller_input);
            done_ = env_->isDone();

            if (done_)
                break;

            // Capture the last two frames for max-pooling directly from the emulator.
            if (maxpool_)
            {
                if (skip == frame_skip_ - 2)
                {
                    env_->getFrameBufferData(m_raw_frames[0].data(), false);
                }
                if (skip == frame_skip_ - 1)
                {
                    env_->getFrameBufferData(m_raw_frames[0].data(), true);
                }
            }
        }
        reward_ = accumulated_reward;

        // If not max-pooling, just get the final frame.
        if (!maxpool_)
        {
            env_->getFrameBufferData(m_raw_frames[0].data(), false);
        }

        // Perform all image processing and update the frame stack.
        process_screen();

        // Write the final stacked observation to the user's buffer.
        write_observation(obs_output_buffer);
    }

    void PreprocessedEnv::process_screen()
    {
        // // 1. Max-pooling (if enabled).
        // if (maxpool_)
        // {
        //     for (size_t i = 0; i < m_raw_size; ++i)
        //     {
        //         m_raw_frames[0][i] = std::max(m_raw_frames[0][i], m_raw_frames[1][i]);
        //     }
        // }

        // 2. Create a cv::Mat wrapper for the source image (always start with RGB).
        auto cv2_format = grayscale_ ? CV_8UC1 : CV_8UC3;
        cv::Mat source_mat(m_raw_frame_height, m_raw_frame_width, cv2_format, m_raw_frames[0].data());

        // 3. Get a pointer to the current position in the circular frame stack.
        uint8_t *dest_ptr = m_frame_stack.data() + (m_frame_stack_idx * m_obs_size);

        // 4. Resizing.
        bool requires_resize = (obs_height_ != m_raw_frame_height) || (obs_width_ != m_raw_frame_width);
        if (requires_resize)
        {
            cv::Mat dest_mat(obs_height_, obs_width_, cv2_format, dest_ptr);
            cv::resize(source_mat, dest_mat, dest_mat.size(), 0, 0, cv::INTER_AREA);
        }
        else
        {
            // No resize needed, just copy the data directly.
            std::memcpy(dest_ptr, source_mat.data, m_obs_size);
        }

        // 5. Move to the next position in the circular buffer.
        m_frame_stack_idx = (m_frame_stack_idx + 1) % stack_num_;
    }

    void PreprocessedEnv::write_observation(uint8_t *obs_output_buffer)
    {
        // Copy frames from oldest to newest into the single output buffer.
        for (int i = 0; i < stack_num_; ++i)
        {
            int src_idx = (m_frame_stack_idx + i) % stack_num_;
            std::memcpy(
                obs_output_buffer + i * m_obs_size,
                m_frame_stack.data() + src_idx * m_obs_size,
                m_obs_size);
        }
    }

    // --- Getters ---
    bool PreprocessedEnv::isDone() const { return done_; }
    float PreprocessedEnv::getReward() const { return reward_; }
    std::vector<uint8_t> PreprocessedEnv::getActionSet() const { return action_set_; }
    size_t PreprocessedEnv::getObservationSize() const { return stack_num_ * m_obs_size; }

} // namespace environment
