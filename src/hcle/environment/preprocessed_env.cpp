#include <stdexcept>

#include <opencv2/opencv.hpp>

#include "hcle/environment/preprocessed_env.hpp"

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
              stack_num_(stack_num),
              reward_(0.0f)
        {
            rgb_buffer.resize(m_raw_frame_height * m_raw_frame_width * 3, 0);
            grayscale_buffer.resize(m_raw_frame_height * m_raw_frame_width * 3, 0);

            env_ = std::make_unique<HCLEnvironment>();
            env_->loadROM(rom_path, render_mode);
            action_set_ = env_->getActionSet();

            // --- INITIALIZE NEW MEMBERS ---
            m_channels_per_frame = grayscale_ ? 1 : 3;

            // Get raw screen dimensions
            m_raw_size = m_raw_frame_height * m_raw_frame_width * 3;
            m_obs_size = obs_height_ * obs_width_ * m_channels_per_frame;

            // Allocate buffers
            m_raw_frames.resize(2, std::vector<uint8_t>(m_raw_size));
            m_frame_stack.resize(stack_num_ * m_obs_size, 0);
            m_frame_stack_idx = 0;
        }

        void PreprocessedEnv::reset()
        {
            env_->reset();
            // After reset, we need to populate the first frame of the stack
            std::fill(m_frame_stack.begin(), m_frame_stack.end(), 0);
            m_frame_stack_idx = 0;

            // Get the initial screen
            get_screen_data(m_raw_frames[0].data());
            std::fill(m_raw_frames[1].begin(), m_raw_frames[1].end(), 0); // Clear the second buffer

            process_screen(); // Process it into the frame stack
        }

        void PreprocessedEnv::step(uint8_t action_index)
        {
            if (action_index >= action_set_.size())
            {
                throw std::out_of_range("Action index out of range.");
            }

            uint8_t controller_input = action_set_[action_index];
            float accumulated_reward = 0.0f;
            bool done = false;

            // --- FRAME SKIPPING LOOP ---
            for (int skip = 0; skip < frame_skip_; ++skip)
            {
                accumulated_reward += env_->act(controller_input);
                done = env_->isDone();

                if (done)
                    break;

                // Capture the last two frames for max-pooling
                if (maxpool_ && skip >= frame_skip_ - 2)
                {
                    // The index will be 0 for the second-to-last frame, 1 for the last
                    int buffer_idx = (frame_skip_ - 1) - skip;
                    env_->getScreenRGB(m_raw_frames[buffer_idx].data());
                }
            }
            this->reward_ = accumulated_reward;

            // If not max-pooling, just get the final frame
            if (!maxpool_)
            {
                env_->getScreenRGB(m_raw_frames[0].data());
            }

            process_screen();
        }

        hcle::vector::Timestep PreprocessedEnv::get_timestep() const
        {
            hcle::vector::Timestep timestep;
            timestep.reward = this->reward_;
            timestep.done = env_->isDone();

            // The observation is a stack of frames. We need to assemble it correctly.
            timestep.observation.resize(stack_num_ * m_obs_size);

            for (int i = 0; i < stack_num_; ++i)
            {
                int src_idx = (m_frame_stack_idx + i) % stack_num_;
                std::memcpy(
                    timestep.observation.data() + i * m_obs_size,
                    m_frame_stack.data() + src_idx * m_obs_size,
                    m_obs_size);
            }

            timestep.final_observation = nullptr;

            return timestep;
        }

        void PreprocessedEnv::process_screen()
        {
            // 1. Max-pooling (if enabled)
            if (maxpool_)
            {
                // This assumes m_raw_frames are RGB (3 channels)
                const size_t raw_rgb_size = m_raw_frame_height * m_raw_frame_width * 3;
                for (size_t i = 0; i < raw_rgb_size; ++i)
                {
                    m_raw_frames[0][i] = std::max(m_raw_frames[0][i], m_raw_frames[1][i]);
                }
            }

            // 2. Create a cv::Mat wrapper for the source image (always start with RGB)
            cv::Mat source_mat(m_raw_frame_height, m_raw_frame_width, CV_8UC3, m_raw_frames[0].data());
            cv::Mat processed_mat; // This will hold our intermediate results

            // 3. Color Conversion (if enabled)
            if (grayscale_)
            {
                cv::cvtColor(source_mat, processed_mat, cv::COLOR_RGB2GRAY);
            }
            else
            {
                // If not converting to grayscale, our processed_mat is just the source_mat
                processed_mat = source_mat;
            }

            // 4. Get a pointer to the current position in the circular frame stack
            uint8_t *dest_ptr = m_frame_stack.data() + (m_frame_stack_idx * m_obs_size);

            // 5. Resizing
            bool requires_resize = (obs_height_ != m_raw_frame_height) || (obs_width_ != m_raw_frame_width);
            if (requires_resize)
            {
                // Create a destination Mat wrapper around our final buffer
                auto cv2_format = grayscale_ ? CV_8UC1 : CV_8UC3;
                cv::Mat dest_mat(obs_height_, obs_width_, cv2_format, dest_ptr);

                // Resize the processed image into the destination buffer
                cv::resize(processed_mat, dest_mat, dest_mat.size(), 0, 0, cv::INTER_AREA);
            }
            else
            {
                // No resize needed, just copy the processed data directly
                std::memcpy(dest_ptr, processed_mat.data, m_obs_size);
            }

            // 6. Move to the next position in the circular buffer
            m_frame_stack_idx = (m_frame_stack_idx + 1) % stack_num_;
        }

        void PreprocessedEnv::get_screen_data(uint8_t *buffer)
        {
            if (grayscale_)
            {
                env_->getScreenRGB(this->rgb_buffer.data());

                cv::Mat rgb_mat(m_raw_frame_height, m_raw_frame_width, CV_8UC3, this->rgb_buffer.data());
                cv::Mat gray_mat(m_raw_frame_height, m_raw_frame_width, CV_8UC1, buffer); // Wrap the destination buffer
                cv::cvtColor(rgb_mat, gray_mat, cv::COLOR_RGB2GRAY);
            }
            else
            {
                env_->getScreenRGB(buffer);
            }
        }

        bool PreprocessedEnv::isDone()
        {
            return env_->isDone();
        }

        const std::vector<uint8_t> PreprocessedEnv::getActionSet()
        {
            return env_->getActionSet();
        }

    } // namespace environment
} // namespace hcle