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

    bool isDone() const { return m_done; }
    double getReward() const { return m_reward; }
    std::vector<uint8_t> getActionSet() const { return m_action_set; }
    size_t getObservationSize() const { return m_stacked_obs_size; }
    const uint8_t *getFramePointer() const { return m_env->frame_ptr; }

    void saveToState(int state_num);
    void loadFromState(int state_num);

    void createWindow(uint8_t fps_limit = 0);
    void updateWindow();

  private:
    void processScreen();

    void writeObservation(uint8_t *obs_output_buffer);

    int m_obs_height;
    int m_obs_width;
    int m_frame_skip;
    bool m_maxpool;
    bool m_grayscale;
    int m_stack_num;

    bool m_requires_resize;

    std::unique_ptr<HCLEnvironment> m_env;
    std::vector<uint8_t> m_action_set;

    double m_reward;
    bool m_done;

    // Buffers and dimensions
    const int m_raw_frame_height = 240;
    const int m_raw_frame_width = 256;
    int m_channels_per_frame;
    size_t m_raw_size; // Size of a single raw RGB frame from the emulator
    size_t m_obs_size; // Size of a single processed (resized, grayscale) observation frame
    size_t m_stacked_obs_size;

    std::vector<uint8_t> m_prev_frame;  // Previous frame for max-pooling
    std::vector<uint8_t> m_frame_stack; // Circular buffer for stacked processed frames
    int m_frame_stack_idx;
  };
}
