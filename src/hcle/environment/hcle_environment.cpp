#include "hcle/environment/hcle_environment.hpp"
#include "hcle/common/exceptions.hpp"
#include "hcle/games/smb1.hpp"
#include <stdexcept>
#include <iostream>
#include "hcle/version.hpp"

namespace hcle
{
    namespace environment
    {

        bool HCLEnvironment::was_welcomed = false;

        HCLEnvironment::HCLEnvironment()
        {
            this->WelcomeMessage();
        }

        void HCLEnvironment::WelcomeMessage()
        {
            if (!was_welcomed)
                return;
            std::cout << "H.C.L.E: Home Console Learning Environment (version " << HCLE_VERSION << ")" << std::endl;
            std::cout << "[Powered by cynes]" << std::endl;
            was_welcomed = true;
        }

        void HCLEnvironment::loadROM(const std::string &rom_path, const std::string &render_mode)
        {
            rom_path_ = rom_path;
            render_mode_ = render_mode;

            emu.reset(new cynes::NES(rom_path.c_str()));

            if (render_mode_ == "human")
                display_ = std::make_unique<hcle::common::Display>("HCLEnvironment", 256, 240, 3);

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

            static const hcle::games::GameLogic *roms[] = {
                new hcle::games::SMB1Logic(),
            };
            return roms[0]->clone();
        }

        const std::vector<uint8_t> &HCLEnvironment::getActionSet() const
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
            if (!game_logic->onReset())
            {
                emu->reset();
            }
            this->current_step_ = 0;
            game_logic->updateRAM();
            emu->step(NES_INPUT_NONE, 1);
        }

        float HCLEnvironment::act(uint8_t controller_input)
        {
            if (!emu || !game_logic)
            {
                throw std::runtime_error("Environment must be loaded with a ROM before calling step.");
            }

            emu->step(controller_input, 1);
            this->current_step_++;
            game_logic->updateRAM();
            game_logic->onStep();

            if (this->render_mode_ == "human")
                this->render();

            return game_logic->getReward();
        }

        void HCLEnvironment::render()
        {
            if (this->display_)
            {
                this->display_->update(emu->get_frame_buffer());

                if (this->display_->processEvents())
                {
                    throw hcle::common::WindowClosedException();
                }
            }
        }

        bool HCLEnvironment::isDone()
        {
            if (!game_logic)
                return true;
            return game_logic->isDone();
        }

        void HCLEnvironment::getScreenRGB(uint8_t *buffer) const
        {
            if (!emu)
            {
                throw std::runtime_error("Cannot get screen; no game loaded.");
            }
            const uint8_t *frame_ptr = emu->get_frame_buffer();
            const size_t obs_size = 240 * 256 * 3;
            // Copy the data directly into the provided buffer
            std::copy(frame_ptr, frame_ptr + obs_size, buffer);
        }

        std::vector<uint8_t> HCLEnvironment::getRAM()
        {
            if (!emu)
            {
                throw std::runtime_error("Cannot get RAM; no game loaded.");
            }
            // This is a temporary vector to hold the RAM data
            std::vector<uint8_t> ram_buffer(2048);
            const uint8_t *ram_ptr = emu->get_ram_pointer();
            // Copy the data from the raw pointer to our vector
            std::copy(ram_ptr, ram_ptr + ram_buffer.size(), ram_buffer.begin());
            return ram_buffer;
        }

    } // namespace environment
} // namespace hcle