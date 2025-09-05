#include <stdexcept>
#include <iostream>
#include <chrono>

#include "hcle/environment/hcle_environment.hpp"
#include "hcle/games/roms.hpp"

namespace hcle
{
    namespace environment
    {
        bool HCLEnvironment::was_welcomed = false;

        HCLEnvironment::HCLEnvironment()
        {
            this->WelcomeMessage();
        }

        void HCLEnvironment::createWindow(uint8_t fps_limit)
        {
            m_display = std::make_unique<hcle::common::Display>("HCLEnvironment", 256, 240, 3);
            m_fps_limit = fps_limit;
            m_fps_sleep_ms = static_cast<milliseconds>(static_cast<int>((1.0f / static_cast<float>(m_fps_limit)) * 1000));
            m_last_update = steady_clock::now();
        }

        void HCLEnvironment::updateWindow()
        {
            if (m_fps_limit > 0)
            {
                milliseconds time_dif = duration_cast<milliseconds>(steady_clock::now() - m_last_update);
                m_last_update = steady_clock::now();
                std::this_thread::sleep_for(m_fps_sleep_ms - time_dif);
            }
            hcle::common::Display::update_window(m_display, frame_ptr, m_single_channel);
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
            m_frame_size = SINGLE_CHAN_FRAME_SIZE;
            m_single_channel = true;
        }

        void HCLEnvironment::setOutputMode(std::string mode)
        {
            if (mode == "grayscale")
            {
                emu->setOutputModeGrayscale();
                m_frame_size = SINGLE_CHAN_FRAME_SIZE;
                m_single_channel = true;
            }
            else if (mode == "index")
            {
                emu->setOutputModeColorIndex();
                m_frame_size = SINGLE_CHAN_FRAME_SIZE;
                m_single_channel = true;
            }
        }

        void HCLEnvironment::loadROM(const std::string &game_name)
        {
            m_rom_path = hcle::get_rom_path(game_name);
            m_frame_size = RAW_FRAME_SIZE;

            emu.reset(new cynes::NES(m_rom_path.c_str()));

            frame_ptr = emu->get_frame_buffer();

            games::GameLogic *logic_template = hcle::get_game_logic(game_name);
            if (logic_template)
            {
                game_logic.reset(logic_template->clone());
                game_logic->initialize(emu.get());
            }

            this->reset();
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
            this->m_current_step = 0;
            game_logic->updateRAM();
        }

        double HCLEnvironment::act(uint8_t controller_input, unsigned int frames)
        {
            if (!emu || !game_logic)
            {
                throw std::runtime_error("Environment must be loaded with a ROM before calling step.");
            }
            game_logic->updateRAM();
            if (m_display)
            {
                for (unsigned int k = 0; k < frames; k++)
                {
                    emu->step(controller_input, 1);
                    this->updateWindow();
                }
            }
            else
            {
                emu->step(controller_input, frames);
            }
            this->m_current_step++;
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

        double HCLEnvironment::getReward() const
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