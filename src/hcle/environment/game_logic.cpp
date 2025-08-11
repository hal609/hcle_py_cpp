#include "hcle/environment/game_logic.hpp"
#include "hcle/emucore/nes.hpp" // For the NES class definition
#include <algorithm>            // For std::copy

namespace hcle
{
    namespace games
    {

        void GameLogic::initialize(cynes::NES *nes)
        {
            this->nes_ = nes;
            this->previous_ram_.resize(2048);
            this->current_ram_.resize(2048);
            // Initialize RAM buffers
            updateRAM();
            previous_ram_ = current_ram_;
        }

        void GameLogic::updateRAM()
        {
            if (!nes_)
                return;

            // Move the last frame's RAM to the previous_ram buffer
            previous_ram_ = std::move(current_ram_);

            // Get the new RAM state from the emulator
            const uint8_t *ram_ptr = nes_->get_ram_pointer();
            // Since current_ram_ is being moved, we need to ensure it has the correct size
            current_ram_.resize(2048);
            // Copy the raw memory from the emulator into our std::vector buffer
            std::copy(ram_ptr, ram_ptr + 2048, current_ram_.begin());
        }

        void GameLogic::frameadvance(uint8_t controller_value)
        {
            nes_->step(controller_value, 1);
        }

    } // namespace games
} // namespace hcle