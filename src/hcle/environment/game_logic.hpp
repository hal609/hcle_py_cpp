#pragma once

#include <vector>
#include <cstdint>
#include <memory>
#include <algorithm>

#include "hcle/emucore/nes.hpp"

const uint8_t NES_INPUT_NONE = 0x00;
const uint8_t NES_INPUT_RIGHT = 0x01;
const uint8_t NES_INPUT_LEFT = 0x02;
const uint8_t NES_INPUT_DOWN = 0x04;
const uint8_t NES_INPUT_UP = 0x08;
const uint8_t NES_INPUT_START = 0x10;
const uint8_t NES_INPUT_SELECT = 0x20;
const uint8_t NES_INPUT_B = 0x40;
const uint8_t NES_INPUT_A = 0x80;

namespace cynes
{
    class NES;
}

namespace hcle
{
    namespace games
    {
        class GameLogic
        {
        public:
            virtual ~GameLogic() = default;
            virtual void initialize(cynes::NES *nes);

            virtual GameLogic *clone() const = 0;
            virtual float getReward() = 0;
            virtual bool isDone() = 0;
            virtual void onStep() {}
            virtual bool onReset() { return false; }
            virtual const std::vector<uint8_t> getActionSet() = 0;

            void updateRAM()
            {
                if (!nes_)
                    return;
                previous_ram_ = std::move(current_ram_);
                const uint8_t *ram_ptr = nes_->get_ram_pointer();
                current_ram_.resize(2048);
                std::copy(ram_ptr, ram_ptr + 2048, current_ram_.begin());
            }

            void frameadvance(uint8_t controller_value) { nes_->step(controller_value, 1); }

        protected:
            cynes::NES *nes_ = nullptr;
            std::vector<uint8_t> previous_ram_;
            std::vector<uint8_t> current_ram_;
        };

        inline void GameLogic::initialize(cynes::NES *nes)
        {
            this->nes_ = nes;
            this->previous_ram_.resize(2048);
            this->current_ram_.resize(2048);
            updateRAM();
            previous_ram_ = current_ram_;
        }
    } // namespace games
} // namespace hcle