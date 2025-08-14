#pragma once

#include <string>
#include <vector>
#include <memory>

#include <opencv2/opencv.hpp>

#include "hcle/common/display.hpp"
#include "hcle/emucore/nes.hpp"
#include "hcle/environment/hcle_environment.hpp"
#include "hcle/environment/utils.hpp"
#include "hcle/environment/game_logic.hpp"
#include "hcle/games/smb1.hpp"

// Ensure the namespace is correct
namespace hcle
{
  namespace environment
  {
    class PreprocessedEnv
    {
    public:
      PreprocessedEnv(
          const std::string &rom_path,
          const std::string &game_name,
          const std::string &render_mode,
          const uint8_t obs_height,
          const uint8_t obs_width,
          const uint8_t frame_skip,
          const bool maxpool,
          const bool grayscale,
          const uint8_t stack_num);

      void set_action(uint8_t action_index);

      hcle::vector::Timestep get_timestep() const;
      void step(uint8_t action_index);
      void reset();
      void render();
      bool isDone();

      void getObservation(uint8_t *buffer);

      std::vector<uint8_t> getRAM();
      const std::vector<uint8_t> getActionSet();
      const size_t getObservationSize() const { return m_obs_size; };

    private:
      void process_screen();
      void get_screen_data(uint8_t *buffer); // Helper to get screen data

      std::unique_ptr<HCLEnvironment> env_;

      float reward_;
      uint64_t current_step_;
      std::unique_ptr<hcle::games::GameLogic> game_logic_;

      std::vector<uint8_t> action_set_;

      std::unique_ptr<hcle::common::Display> display_;

      std::string rom_path_;
      std::string game_name_;
      std::string render_mode_;
      uint8_t obs_height_;
      uint8_t obs_width_;
      uint8_t frame_skip_;
      bool maxpool_;
      bool grayscale_;
      uint8_t stack_num_;

      // Frame size variables
      int m_channels_per_frame;
      const int m_raw_frame_height = 240;
      const int m_raw_frame_width = 256;
      std::vector<uint8_t> rgb_buffer;
      std::vector<uint8_t> grayscale_buffer;
      int m_raw_size; // Size of one raw frame (height * width * channels)
      int m_obs_size; // Size of one processed frame (obs_height * obs_width * channels)

      // Frame buffers
      std::vector<std::vector<uint8_t>> m_raw_frames; // Buffers for the last 2 raw frames for max-pooling
      std::vector<uint8_t> m_frame_stack;             // Circular buffer for the final N stacked frames
      int m_frame_stack_idx;                          // Index for the circular buffer

      bool should_close_;
    };

  } // namespace environment
} // namespace hcle