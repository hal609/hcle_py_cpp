#pragma once

#include <string>
#include <vector>
#include <memory>

#include "hcle/emucore/nes.hpp"
#include "hcle/games/game_logic.hpp"
#include "hcle/common/exceptions.hpp"
#include "hcle/common/display.hpp"
#include "hcle/games/smb1.hpp"
#include "hcle/games/kungfu.hpp"
#include "hcle/version.hpp"

struct StepResult
{
  float reward;
  bool done;
  std::vector<uint8_t> observation;
};

const size_t RAW_FRAME_SIZE = 240 * 256 * 3;     // RGB format
const size_t SINGLE_CHAN_FRAME_SIZE = 240 * 256; // Grayscale format

// Ensure the namespace is correct
namespace hcle
{
  namespace environment
  {

    using tp = std::chrono::steady_clock::time_point;
    using namespace std::chrono;

    class HCLEnvironment
    {
    public:
      HCLEnvironment();

      void createWindow(uint8_t fps_limit = 0);
      void updateWindow();

      static void WelcomeMessage();

      void loadROM(const std::string &game_name);
      void setOutputModeGrayscale();
      void setOutputMode(std::string mode);
      double act(uint8_t controller_input, unsigned int frames);

      const std::vector<uint8_t> getActionSet() const;
      double getReward() const;

      bool isDone();
      void reset();

      void saveToState(int state_num);
      void loadFromState(int state_num);

      const uint8_t *frame_ptr;

      std::unique_ptr<cynes::NES> emu;
      std::unique_ptr<hcle::games::GameLogic> game_logic;

    private:
      static bool was_welcomed;

      std::string rom_path_;
      std::string game_name_;

      std::unique_ptr<hcle::games::GameLogic> game_logic_;

      size_t frame_size_;

      uint64_t current_step_;

      std::unique_ptr<hcle::common::Display> display_;
      bool single_channel_ = false;
      bool running_window_ = false;
      uint8_t fps_limit_ = 0;
      milliseconds fps_sleep_ms_;
      tp last_update;
    };

  } // namespace environment
} // namespace hcle