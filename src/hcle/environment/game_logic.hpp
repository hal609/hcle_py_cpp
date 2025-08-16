#pragma once

#include <vector>
#include <cstdint>
#include <memory>
#include <algorithm>
#include <mutex>

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

const int RAM_SIZE = 2048;

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
            virtual void reset() {}
            virtual const std::vector<uint8_t> getActionSet() { return std::vector<uint8_t>{0}; }

            void updateRAM()
            {
                if (!nes_)
                    return;
                std::memcpy(previous_ram_.data(), current_ram_ptr_, RAM_SIZE);
            }

            void frameadvance(uint8_t controller_value) { nes_->step(controller_value, 1); }

        protected:
            cynes::NES *nes_ = nullptr;
            const uint8_t *current_ram_ptr_ = nullptr;
            std::array<uint8_t, 2048> previous_ram_;

            int *ram_ptr_ = nullptr;
            int ram_frame_ = 0;
        };

        inline void GameLogic::initialize(cynes::NES *nes)
        {
            nes_ = nes;
            current_ram_ptr_ = nes_->get_ram_pointer();
            updateRAM();
        }
    } // namespace games
} // namespace hcle