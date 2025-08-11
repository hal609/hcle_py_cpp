// src/hcle/environment/hcle_environment.hpp
#pragma once

#include "hcle/emucore/nes.hpp"
#include "hcle/environment/game_logic.hpp"
#include <string>
#include <vector>
#include <memory>

namespace hcle
{

    class HCLEnvironment
    {
    public:
        HCLEnvironment();

        void loadROM(const std::string &rom_path, const std::string &game_name);

        // The main step function called from Python
        float act(uint8_t action_index);

        void reset();

        // Getters for Python
        const std::vector<uint8_t> &getScreenRGB();
        const std::vector<uint8_t> &getRAM();
        const std::vector<uint8_t> &getActionSet();
        bool isDone();

    private:
        std::unique_ptr<cynes::NES> nes_;
        std::unique_ptr<hcle::games::GameLogic> game_logic_;

        // A factory to create the correct GameLogic based on game_name
        void createGameLogic(const std::string &game_name);
    };

} // namespace hcle