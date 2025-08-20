#include <stdexcept>
#include <iostream>
#include <chrono>

#include "hcle/environment/hcle_environment.hpp"

namespace hcle
{
    namespace environment
    {

        bool HCLEnvironment::was_welcomed = false;

        HCLEnvironment::HCLEnvironment()
        {
            // this->WelcomeMessage();
        }

        void HCLEnvironment::WelcomeMessage()
        {
            if (!was_welcomed)
                return;
            std::cout << "H.C.L.E: Home Console Learning Environment (version " << HCLE_VERSION << ")" << std::endl;
            std::cout << "[Powered by cynes]" << std::endl;
            was_welcomed = true;
        }

        void HCLEnvironment::setOutputModeGrayscale()
        {
            if (!emu)
            {
                throw std::runtime_error("Environment must be loaded with a ROM before setting output mode.");
            }
            emu->setOutputModeGrayscale();
            frame_size_ = GRAYSCALE_FRAME_SIZE;
        }

        void HCLEnvironment::loadROM(const std::string &rom_path)
        {
            rom_path_ = rom_path;
            frame_size_ = RAW_FRAME_SIZE;

            emu.reset(new cynes::NES(rom_path.c_str()));

            frame_ptr = emu->get_frame_buffer();

            hcle::games::GameLogic *wrapper = createGameLogic(rom_path);
            game_logic.reset(wrapper);
            game_logic->initialize(emu.get());

            this->reset();
        }

        hcle::games::GameLogic *
        HCLEnvironment::createGameLogic(const std::string &rom_path)
        {
            // Temporarily just assume the game is SMB1. Later we will
            // probably move this logic elsewhere and check the md5 of the rom
            // to grab the correct game logic.
            if (rom_path.find("smb1") != std::string::npos)
            {
                return new hcle::games::SMB1Logic();
            }
            else if (rom_path.find("kungfu") != std::string::npos)
            {
                return new hcle::games::KungFuLogic();
            }
            // static const hcle::games::GameLogic *roms[] = {
            //     new hcle::games::SMB1Logic(),
            //     new hcle::games::KungFuLogic(),
            // };
            // return roms[1]->clone();
        }

        const std::vector<uint8_t> HCLEnvironment::getActionSet() const
        {
            if (!this->game_logic)
            {
                throw std::runtime_error("Cannot get action set; no game loaded.");
            }
            return game_logic->getActionSet();
        }

        void HCLEnvironment::reset()
        {
            if (!emu || !game_logic)
            {
                throw std::runtime_error("Environment must be loaded with a ROM before reset.");
            }
            game_logic->reset();
            this->current_step_ = 0;
            game_logic->updateRAM();
        }

        float HCLEnvironment::act(uint8_t controller_input, unsigned int frames)
        {
            if (!emu || !game_logic)
            {
                throw std::runtime_error("Environment must be loaded with a ROM before calling step.");
            }
            game_logic->updateRAM();
            emu->step(controller_input, frames);
            this->current_step_++;
            game_logic->onStep();

            return game_logic->getReward();
        }

        void HCLEnvironment::saveToState(int state_num)
        {
            if (!game_logic)
            {
                throw std::runtime_error("Environment must be loaded with a ROM before saving state.");
            }
            game_logic->saveToState(state_num);
        }

        void HCLEnvironment::loadFromState(int state_num)
        {
            game_logic->loadFromState(state_num);
        }

        float HCLEnvironment::getReward() const
        {
            if (!game_logic)
                throw std::runtime_error("Environment must be loaded with a ROM before getting reward.");
            return game_logic->getReward();
        }

        bool HCLEnvironment::isDone()
        {
            return game_logic->isDone();
        }

    } // namespace environment
} // namespace hcle