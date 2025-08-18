#pragma once

#include <vector>
#include <cstdint>
#include <memory>
#include <algorithm>
#include <mutex>

#include "hcle/emucore/nes.hpp"
#include "hcle/emucore/utils.hpp"

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
            virtual inline void initialize(cynes::NES *nes)
            {
                nes_ = nes;
                current_ram_ptr_ = nes_->get_ram_pointer();
                backup_state_.resize(nes_->size());
                updateRAM();
            }

            virtual ~GameLogic() = default;
            virtual GameLogic *clone() const = 0;
            virtual float getReward() = 0;
            virtual bool isDone() = 0;
            virtual void onStep() {}
            virtual const std::vector<uint8_t> getActionSet() { return action_set; }
            void frameadvance(uint8_t controller_value) { nes_->step(controller_value, 1); }

            void updateRAM()
            {
                if (!nes_)
                    return;
                std::memcpy(previous_ram_.data(), current_ram_ptr_, RAM_SIZE);
            }

            void reset()
            {
                if (has_backup_)
                {
                    nes_->load(backup_state_.data());
                }
                else
                {
                    nes_->reset();
                    nes_->step(NES_INPUT_NONE, 1);
                }
            }

        protected:
            cynes::NES *nes_ = nullptr;
            const uint8_t *current_ram_ptr_ = nullptr;
            std::array<uint8_t, 2048> previous_ram_;

            std::vector<uint8_t> action_set = std::vector<uint8_t>{0};

            static inline bool has_backup_ = false;
            static inline std::vector<uint8_t> backup_state_;
            static inline std::mutex backup_mutex_;

            void createBackup()
            {
                std::lock_guard<std::mutex> lock(backup_mutex_);
                nes_->save(backup_state_.data());
                has_backup_ = true;
            }

            float changeIn(int address)
            {
                return static_cast<float>(current_ram_ptr_[address] - previous_ram_[address]);
            }
        };
    } // namespace games
} // namespace hcle