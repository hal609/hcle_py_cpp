#include <stdexcept>

#include "hcle/environment/preprocessed_env.hpp"
#include "hcle/games/smb1.hpp"

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
              stack_num_(stack_num)
        {
            env_ = std::make_unique<HCLEnvironment>();
            env_->loadROM(rom_path, render_mode);
            action_set_ = env_->getActionSet();

            // --- INITIALIZE NEW MEMBERS ---
            m_channels_per_frame = grayscale_ ? 1 : 3;

            // Get raw screen dimensions from the base environment
            m_raw_frame_height = 240; // Assuming fixed size from your NES emulator
            m_raw_frame_width = 256;

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
        }

        float PreprocessedEnv::step(uint8_t action_index)
        {
            if (action_index >= action_set_.size())
                throw std::out_of_range("Action index out of range.");

            if (typeid(action_index) != typeid(uint8_t))
                throw std::invalid_argument("Action index must be of type uint8_t.");

            uint8_t controller_input = action_set_[action_index];
            if (this->isDone())
            {
                this->reset();
            }
            float accumulated_reward = 0.0f;
            accumulated_reward += env_->act(controller_input);

            // env_->getScreenRGB(m_raw_frames[0].data());

            return accumulated_reward;
        }

        void PreprocessedEnv::getObservation(uint8_t *buffer)
        {
            // Copy the frames from our circular buffer into the output buffer,
            // reordering them from oldest to newest.
            for (int i = 0; i < stack_num_; ++i)
            {
                int src_idx = (m_frame_stack_idx + i) % stack_num_;
                std::memcpy(
                    buffer + i * m_obs_size,
                    m_frame_stack.data() + src_idx * m_obs_size,
                    m_obs_size);
            }
        }

        bool PreprocessedEnv::isDone()
        {
            return env_->isDone();
        }

        const std::vector<uint8_t> &PreprocessedEnv::getActionSet()
        {
            return env_->getActionSet();
        }

        void PreprocessedEnv::getScreenRGB(uint8_t *buffer) const
        {
            env_->getScreenRGB(buffer);
        }

        std::vector<uint8_t> PreprocessedEnv::getRAM()
        {
            return env_->getRAM();
        }

        void PreprocessedEnv::process_screen()
        {
            // 1. Max-pool the last two raw frames
            if (maxpool_)
            {
                for (int i = 0; i < m_raw_size; ++i)
                {
                    m_raw_frames[0][i] = std::max(m_raw_frames[0][i], m_raw_frames[1][i]);
                }
            }

            // Get a pointer to the current position in the circular frame stack buffer
            uint8_t *dest_ptr = m_frame_stack.data() + (m_frame_stack_idx * m_obs_size);

            // 2. Resize and convert to grayscale (if needed) using OpenCV
            auto cv2_format = grayscale_ ? CV_8UC1 : CV_8UC3;

            // Create OpenCV Mat wrappers around our raw memory (no data is copied here)
            cv::Mat src_img(m_raw_frame_height, m_raw_frame_width, cv2_format, m_raw_frames[0].data());
            cv::Mat dst_img(obs_height_, obs_width_, cv2_format, dest_ptr);

            // Perform the resize. INTER_AREA is best for downscaling.
            cv::resize(src_img, dst_img, dst_img.size(), 0, 0, cv::INTER_AREA);

            // 3. Advance the circular buffer index
            m_frame_stack_idx = (m_frame_stack_idx + 1) % stack_num_;
        }

    } // namespace environment
} // namespace hcle