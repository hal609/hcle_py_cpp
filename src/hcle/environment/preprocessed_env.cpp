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
        const int stack_num)
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
        env_->loadROM(rom_path);

        if (grayscale_)
            env_->setOutputModeGrayscale();

        action_set_ = env_->getActionSet();

        // The final observation size depends on the preprocessing options.
        m_channels_per_frame = grayscale_ ? 1 : 3;
        m_raw_size = m_raw_frame_height * m_raw_frame_width * m_channels_per_frame;
        m_obs_size = obs_height_ * obs_width_ * m_channels_per_frame;
        m_stacked_obs_size = stack_num_ * m_obs_size;

        // Allocate buffers with the correct sizes.
        prev_frame_.resize(m_raw_size, 0);
        m_frame_stack.resize(m_stacked_obs_size, 0);
        m_frame_stack_idx = 0;

        requires_resize_ = (obs_height_ != m_raw_frame_height) || (obs_width_ != m_raw_frame_width);
    }

    void PreprocessedEnv::reset(uint8_t *obs_output_buffer)
    {
        env_->reset();
        reward_ = 0.0f;
        done_ = false;

        m_frame_stack_idx = 0;
        process_screen();

        for (int i = 1; i < stack_num_; ++i)
        {
            std::memcpy(m_frame_stack.data() + (i * m_obs_size),
                        m_frame_stack.data(),
                        m_obs_size);
        }
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

        if (maxpool_)
        {
            accumulated_reward += env_->act(controller_input, frame_skip_ - 1);
            std::memcpy(prev_frame_.data(), env_->frame_ptr, m_raw_size);
            accumulated_reward += env_->act(controller_input, 1);
        }
        else
        {
            accumulated_reward += env_->act(controller_input, frame_skip_);
        }
        done_ = env_->isDone();
        reward_ = accumulated_reward;

        process_screen();

        write_observation(obs_output_buffer);
    }

    void PreprocessedEnv::process_screen()
    {
        auto cv2_format = grayscale_ ? CV_8UC1 : CV_8UC3;
        uint8_t *frame_pointer = const_cast<uint8_t *>(env_->frame_ptr);

        if (maxpool_)
        {
            frame_pointer = std::max(frame_pointer, prev_frame_.data());
        }
        cv::Mat source_mat = cv::Mat(m_raw_frame_height, m_raw_frame_width, cv2_format, frame_pointer);

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

    void PreprocessedEnv::saveToState(int state_num)
    {
        env_->saveToState(state_num);
    }

    void PreprocessedEnv::loadFromState(int state_num)
    {
        env_->loadFromState(state_num);
    }

    // --- Getters ---

} // namespace environment
