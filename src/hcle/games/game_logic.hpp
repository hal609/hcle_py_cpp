#pragma once

#include <vector>
#include <cstdint>
#include <memory>
#include <algorithm>
#include <mutex>
#include <shared_mutex>

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
                std::unique_lock lock(g_savestate_mutex);
                state0.resize(nes_->size());
                state1.resize(nes_->size());
                state2.resize(nes_->size());
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
                    std::lock_guard<std::mutex> lock(backup_mutex_);
                    nes_->load(backup_state_.data());
                }
                else
                {
                    nes_->reset();
                    nes_->step(NES_INPUT_NONE, 1);
                }
            }

            void saveToState(int state_num)
            {

                std::unique_lock lock(g_savestate_mutex);
                if (state_num == 0)
                {
                    nes_->save(state0.data());
                    state0_full = true;
                }
                else if (state_num == 1)
                {
                    nes_->save(state1.data());
                    state1_full = true;
                }
                else if (state_num == 2)
                {
                    nes_->save(state2.data());
                    state2_full = true;
                }
                else
                {
                    throw std::out_of_range("Invalid state number. Must be 0, 1, or 2.");
                }
            }

            void loadFromState(int state_num)
            {

                std::shared_lock lock(g_savestate_mutex);

                if (state_num == 0)
                {
                    if (!state0_full)
                    {
                        throw std::runtime_error("No savestate in slot 0.");
                    }
                    nes_->load(state0.data());
                }
                else if (state_num == 1)
                {
                    if (!state1_full)
                    {
                        throw std::runtime_error("No savestate in slot 1.");
                    }
                    nes_->load(state1.data());
                }
                else if (state_num == 2)
                {
                    if (!state2_full)
                    {
                        throw std::runtime_error("No savestate in slot 2.");
                    }
                    nes_->load(state2.data());
                }
                else
                {
                    throw std::out_of_range("Invalid state number. Must be 0, 1, or 2.");
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

            static inline std::shared_mutex g_savestate_mutex;

            static inline std::vector<uint8_t> state0;
            static inline std::vector<uint8_t> state1;
            static inline std::vector<uint8_t> state2;
            static inline bool state0_full = false;
            static inline bool state1_full = false;
            static inline bool state2_full = false;

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