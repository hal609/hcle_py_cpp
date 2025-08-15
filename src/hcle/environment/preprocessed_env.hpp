#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

#include "hcle/environment/hcle_environment.hpp"

namespace hcle::environment
{
  /**
   * @brief Wraps a single HCLEnvironment to add standard preprocessing steps
   * like frame skipping, max-pooling, resizing, and frame stacking.
   */
  class PreprocessedEnv
  {
  public:
    PreprocessedEnv(
        const std::string &rom_path,
        const std::string &game_name,
        const std::string &render_mode,
        const uint8_t obs_height = 84,
        const uint8_t obs_width = 84,
        const uint8_t frame_skip = 4,
        const bool maxpool = true,
        const bool grayscale = true,
        const uint8_t stack_num = 4);

    /**
     * @brief Resets the environment to an initial state.
     * @param obs_output_buffer A pre-allocated buffer where the final stacked observation will be written.
     */
    void reset(uint8_t *obs_output_buffer);

    /**
     * @brief Executes a single action in the environment for frame_skip steps.
     * @param action_index The index of the action to perform.
     * @param obs_output_buffer A pre-allocated buffer where the final stacked observation will be written.
     */
    void step(uint8_t action_index, uint8_t *obs_output_buffer);

    // --- Getters for state information ---
    bool isDone() const;
    float getReward() const;
    // Return by value to avoid dangling reference issues.
    std::vector<uint8_t> getActionSet() const;
    size_t getObservationSize() const;

  private:
    /**
     * @brief Performs all image processing (max-pool, color conversion, resize)
     * and updates the internal frame stack.
     */
    void process_screen();

    /**
     * @brief Copies the current observation from the internal frame stack to an external buffer.
     * @param obs_output_buffer The destination buffer.
     */
    void write_observation(uint8_t *obs_output_buffer);

    // Environment configuration
    uint8_t obs_height_;
    uint8_t obs_width_;
    uint8_t frame_skip_;
    bool maxpool_;
    bool grayscale_;
    uint8_t stack_num_;

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
