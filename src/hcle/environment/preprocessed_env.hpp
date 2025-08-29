#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

#include "hcle/environment/hcle_environment.hpp"

namespace hcle::environment
{
  class PreprocessedEnv
  {
  public:
    PreprocessedEnv(
        const std::string &rom_path,
        const std::string &game_name,
        const int obs_height,
        const int obs_width,
        const int frame_skip,
        const bool maxpool,
        const bool grayscale,
        const int stack_num,
        const bool color_index_grayscale = false);

    void reset(uint8_t *obs_output_buffer);

    void step(uint8_t action_index, uint8_t *obs_output_buffer);

    // --- Getters ---
    bool isDone() const { return done_; }
    double getReward() const { return reward_; }
    std::vector<uint8_t> getActionSet() const { return action_set_; }
    size_t getObservationSize() const { return m_stacked_obs_size; }
    const uint8_t *getFramePointer() const { return env_->frame_ptr; }

    void saveToState(int state_num);
    void loadFromState(int state_num);

    void createWindow(uint8_t fps_limit = 0);
    void updateWindow();

  private:
    void process_screen();

    void write_observation(uint8_t *obs_output_buffer);

    // Environment configuration
    int obs_height_;
    int obs_width_;
    int frame_skip_;
    bool maxpool_;
    bool grayscale_;
    int stack_num_;

    bool requires_resize_;

    // Core environment
    std::unique_ptr<HCLEnvironment> env_;
    std::vector<uint8_t> action_set_;

    // State variables
    double reward_;
    bool done_;

    // Buffers and dimensions
    const int m_raw_frame_height = 240;
    const int m_raw_frame_width = 256;
    int m_channels_per_frame;
    size_t m_raw_size; // Size of a single raw RGB frame from the emulator
    size_t m_obs_size; // Size of a single processed (resized, grayscale) observation frame
    size_t m_stacked_obs_size;

    std::vector<uint8_t> prev_frame_;   // Previous frame for max-pooling
    std::vector<uint8_t> m_frame_stack; // Circular buffer for stacked processed frames
    int m_frame_stack_idx;
  };
}
