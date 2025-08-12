#include "hcle/environment/hcle_environment.hpp"
#include "hcle/games/smb1.hpp"
#include <stdexcept>

namespace hcle
{
    namespace environment
    {

        HCLEnvironment::HCLEnvironment(const std::string &rom_path, const std::string &game_name, const std::string &render_mode)
            : rom_path_(rom_path), game_name_(game_name), render_mode_(render_mode)
        {
            nes_ = std::make_unique<cynes::NES>(rom_path.c_str(), render_mode_);
            createGameLogic(game_name);
            game_logic_->initialize(nes_.get());
            this->reset();
            this->current_step_ = 0;
        }

        void HCLEnvironment::createGameLogic(const std::string &game_name)
        {
            if (game_name == "SuperMarioBros" || game_name == "smb1")
            {
                game_logic_ = std::make_unique<hcle::games::SMB1Logic>();
            }
            else
            {
                throw std::runtime_error("Unsupported game: " + game_name);
            }
        }

        void HCLEnvironment::reset()
        {
            if (!nes_ || !game_logic_)
            {
                throw std::runtime_error("Environment must be loaded with a ROM before reset.");
            }
            this->current_step_ = 0;
            nes_->reset();
            game_logic_->onReset();
            game_logic_->updateRAM();
            nes_->step(NES_INPUT_NONE, 1);
        }

        float HCLEnvironment::act(uint8_t action_index)
        {
            if (!nes_ || !game_logic_)
            {
                throw std::runtime_error("Environment must be loaded with a ROM before calling act.");
            }

            const auto &action_set = game_logic_->getActionSet();
            if (action_index >= action_set.size())
            {
                throw std::out_of_range("Action index out of range.");
            }

            if (typeid(action_index) != typeid(uint8_t))
            {
                throw std::invalid_argument("Action index must be of type uint8_t.");
            }

            uint8_t controller_input = action_set[action_index];
            nes_->step(controller_input, 1);
            this->current_step_++;
            game_logic_->updateRAM();
            game_logic_->onStep();
            return game_logic_->getReward();
        }

        bool HCLEnvironment::isDone()
        {
            if (!game_logic_)
                return true;
            return game_logic_->isDone();
        }

        const std::vector<uint8_t> &HCLEnvironment::getActionSet()
        {
            if (!game_logic_)
            {
                throw std::runtime_error("Cannot get action set; no game loaded.");
            }
            return game_logic_->getActionSet();
        }

        void HCLEnvironment::getScreenRGB(uint8_t *buffer) const
        {
            if (!nes_)
            {
                throw std::runtime_error("Cannot get screen; no game loaded.");
            }
            const uint8_t *frame_ptr = nes_->get_frame_buffer();
            const size_t obs_size = 240 * 256 * 3;
            // Copy the data directly into the provided buffer
            std::copy(frame_ptr, frame_ptr + obs_size, buffer);
        }

        std::vector<uint8_t> HCLEnvironment::getRAM()
        {
            if (!nes_)
            {
                throw std::runtime_error("Cannot get RAM; no game loaded.");
            }
            // This is a temporary vector to hold the RAM data
            std::vector<uint8_t> ram_buffer(2048);
            const uint8_t *ram_ptr = nes_->get_ram_pointer();
            // Copy the data from the raw pointer to our vector
            std::copy(ram_ptr, ram_ptr + ram_buffer.size(), ram_buffer.begin());
            return ram_buffer;
        }

    } // namespace environment
} // namespace hcle