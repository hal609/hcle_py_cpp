#pragma once

#include "hcle/emucore/nes.hpp"
#include "hcle/environment/game_logic.hpp"
#include <string>
#include <vector>
#include <memory>

// Ensure the namespace is correct
namespace hcle
{
  namespace environment
  {

    class HCLEnvironment
    {
    public:
      HCLEnvironment();

      void loadROM(const std::string &rom_path, const std::string &render_mode);
      float act(uint8_t action_index);

      const std::vector<uint8_t> &getActionSet() const;
      std::vector<uint8_t> getRAM();
      void getScreenRGB(uint8_t *buffer) const;

      void render();
      bool isDone();
      void reset();

      std::unique_ptr<cynes::NES> emu;
      std::unique_ptr<hcle::games::GameLogic> game_logic;

    private:
      hcle::games::GameLogic *createGameLogic(const std::string &rom_path);

      std::string rom_path_;
      std::string game_name_;
      std::string render_mode_;

      std::unique_ptr<hcle::games::GameLogic> game_logic_;
      std::unique_ptr<hcle::common::Display> display_;

      uint64_t current_step_;
    };

  } // namespace environment
} // namespace hcle