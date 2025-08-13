#pragma once

#include <string>
#include <vector>
#include <memory>

#include "hcle/emucore/nes.hpp"
#include "hcle/environment/hcle_environment.hpp"
#include "hcle/environment/game_logic.hpp"

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
          const uint8_t obs_height = 84,
          const uint8_t obs_width = 84,
          const uint8_t frame_skip = 4,
          const bool maxpool = true,
          const bool grayscale = true,
          const uint8_t stack_num = 4);

      float step(uint8_t action_index);
      void getScreenRGB(uint8_t *buffer) const;
      void getObservation(uint8_t *buffer);

      void reset();
      void render();
      bool isDone();

      std::vector<uint8_t> getRAM();
      const std::vector<uint8_t> &getActionSet();

    private:
      void process_screen();

      std::unique_ptr<HCLEnvironment> env_;

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

      bool should_close_;
    };

  } // namespace environment
} // namespace hcle