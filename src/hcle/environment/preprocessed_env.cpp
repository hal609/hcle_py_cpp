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
            m_raw_size = m_raw_frame_height * m_raw_frame_width * m_channels_per_frame;
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
                    get_screen_data(m_raw_frames[buffer_idx].data());
                }
            }
            this->reward_ = accumulated_reward;

            // If not max-pooling, just get the final frame
            if (!maxpool_)
            {
                get_screen_data(m_raw_frames[0].data());
            }

            process_screen();
        }

        hcle::vector::Timestep PreprocessedEnv::get_timestep() const
        {
            hcle::vector::Timestep timestep;
            timestep.reward = this->reward_;
            // printf("Placing reward %.2f onto timestep\n", timestep.reward);
            timestep.done = false; // TODO: Add correct done detection behaviour

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
            // Get pointer to current position in circular buffer
            uint8_t *dest_ptr = m_frame_stack.data() + (m_frame_stack_idx * m_obs_size);

            // Resize directly into the circular buffer or copy if no resize needed
            if (obs_height_ != m_raw_frame_height || obs_width_ != m_raw_frame_width)
            {
                auto cv2_format = (grayscale_) ? CV_8UC1 : CV_8UC3;
                cv::Mat src_img(m_raw_frame_height, m_raw_frame_width, cv2_format, m_raw_frames[0].data());
                cv::Mat dst_img(obs_height_, obs_width_, cv2_format, dest_ptr);
                cv::resize(src_img, dst_img, dst_img.size(), 0, 0, cv::INTER_AREA);
            }
            else
            {
                // No resize needed, copy directly to circular buffer
                std::memcpy(dest_ptr, m_raw_frames[0].data(), m_raw_size);
            }

            // Move to next position in circular buffer
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

        std::vector<uint8_t> PreprocessedEnv::getRAM()
        {
            return env_->getRAM();
        }

    } // namespace environment
} // namespace hcle