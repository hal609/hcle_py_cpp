#pragma once

#include <string>
#include <vector>
#include <memory>

#include "hcle/emucore/nes.hpp"
#include "hcle/environment/game_logic.hpp"
#include "hcle/common/display.hpp"
#include "hcle/common/exceptions.hpp"
#include "hcle/games/smb1.hpp"
#include "hcle/version.hpp"

struct StepResult
{
  float reward;
  bool done;
  std::vector<uint8_t> observation;
};

const size_t RAW_FRAME_SIZE = 240 * 256 * 3;   // RGB format
const size_t GRAYSCALE_FRAME_SIZE = 240 * 256; // Grayscale format

// Ensure the namespace is correct
namespace hcle
{
  namespace environment
  {

    class HCLEnvironment
    {
    public:
      HCLEnvironment();

      static void WelcomeMessage();

      void loadROM(const std::string &rom_path, const std::string &render_mode);
      void setOutputModeGrayscale();
      float act(uint8_t controller_input);

      const std::vector<uint8_t> getActionSet() const;
      void getFrameBufferData(uint8_t *buffer, bool mix_in) const;
      float getReward() const;

      void render();
      bool isDone();
      void reset();

      std::unique_ptr<cynes::NES> emu;
      std::unique_ptr<hcle::games::GameLogic> game_logic;

    private:
      static bool was_welcomed;
      hcle::games::GameLogic *createGameLogic(const std::string &rom_path);

      std::string rom_path_;
      std::string game_name_;
      std::string render_mode_;

      std::unique_ptr<hcle::games::GameLogic> game_logic_;
      std::unique_ptr<hcle::common::Display> display_;

      const uint8_t *frame_ptr_;
      size_t frame_size_;

      uint64_t current_step_;
    };

  } // namespace environment
} // namespace hcle