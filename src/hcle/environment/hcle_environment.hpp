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
      HCLEnvironment(const std::string &rom_path, const std::string &game_name, const std::string &render_mode);

      float act(uint8_t action_index);
      void reset();

      void getScreenRGB(uint8_t *buffer) const;
      std::vector<uint8_t> getRAM();
      void render();
      const std::vector<uint8_t> &getActionSet();
      bool isDone();

    private:
      uint64_t current_step_;
      std::unique_ptr<cynes::NES> nes_;
      std::unique_ptr<hcle::games::GameLogic> game_logic_; // Use full namespace for clarity
      void createGameLogic(const std::string &game_name);

      std::string rom_path_;
      std::string game_name_;
      std::string render_mode_;
    };

  } // namespace environment
} // namespace hcle