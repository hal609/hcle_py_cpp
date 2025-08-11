#pragma once

#include <vector>
#include <cstdint>
#include <memory>

const uint8_t NES_INPUT_NONE = 0x00;
const uint8_t NES_INPUT_RIGHT = 0x01;
const uint8_t NES_INPUT_LEFT = 0x02;
const uint8_t NES_INPUT_DOWN = 0x04;
const uint8_t NES_INPUT_UP = 0x08;
const uint8_t NES_INPUT_START = 0x10;
const uint8_t NES_INPUT_SELECT = 0x20;
const uint8_t NES_INPUT_B = 0x40;
const uint8_t NES_INPUT_A = 0x80;

// Forward-declarations
namespace cynes
{
    class NES;
}

namespace hcle
{
    namespace games
    { // Putting GameLogic in a sub-namespace is good practice

        class GameLogic
        {
        public:
            virtual ~GameLogic() = default;
            virtual void initialize(cynes::NES *nes);
            virtual float getReward() = 0;
            virtual bool isDone() = 0;
            virtual void onStep() {}
            virtual void onReset() {}
            virtual const std::vector<uint8_t> &getActionSet() = 0;
            void updateRAM();
            void frameadvance(uint8_t action_index);

        protected:
            cynes::NES *nes_ = nullptr;
            std::vector<uint8_t> previous_ram_;
            std::vector<uint8_t> current_ram_;
        };

    } // namespace games
} // namespace hcle