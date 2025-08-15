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
        const std::string &render_mode,
        const int obs_height = 84,
        const int obs_width = 84,
        const int frame_skip = 4,
        const bool maxpool = true,
        const bool grayscale = true,
        const int stack_num = 4);

    void reset(uint8_t *obs_output_buffer);

    void step(uint8_t action_index, uint8_t *obs_output_buffer);

    // --- Getters for state information ---
    bool isDone() const;
    float getReward() const;

    // Return by value to avoid dangling reference issues.
    std::vector<uint8_t> getActionSet() const;
    size_t getObservationSize() const;

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
    float reward_;
    bool done_;

    // Buffers and dimensions
    const int m_raw_frame_height = 240;
    const int m_raw_frame_width = 256;
    int m_channels_per_frame;
    size_t m_raw_size; // Size of a single raw RGB frame from the emulator
    size_t m_obs_size; // Size of a single processed (resized, grayscale) observation frame

    std::vector<std::vector<uint8_t>> m_raw_frames; // Buffers for raw frames for max-pooling
    std::vector<uint8_t> m_frame_stack;             // Circular buffer for stacked processed frames
    int m_frame_stack_idx;
  };
}
