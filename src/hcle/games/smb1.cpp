#include "hcle/games/smb1.hpp"
#include "hcle/emucore/nes.hpp" // For access to the NES object
#include <vector>

namespace hcle
{
    namespace games
    {

        // These are the valid button combinations for this game
        const std::vector<uint8_t> SMB1_ACTION_SET = {
            NES_INPUT_RIGHT | NES_INPUT_B,
            NES_INPUT_RIGHT | NES_INPUT_B | NES_INPUT_A,
        };

        SMB1Logic::SMB1Logic()
        {
            action_set_ = SMB1_ACTION_SET;
        }

        bool SMB1Logic::in_game()
        {
            // Logic from your python env: LEVEL_LOADING == 3 and GAME_MODE != 0
            // Access RAM via the protected current_ram_ vector
            return current_ram_[0x0772] == 3 && current_ram_[0x0770] != 0;
        }

        bool SMB1Logic::is_dead()
        {
            // Logic from your python env: PLAYER_STATE == 0x0B or Y_VIEWPORT > 0x1
            return current_ram_[PLAYER_STATE] == 0x0B || current_ram_[Y_VIEWPORT] > 0x1;
        }

        void SMB1Logic::onStep()
        {
            if (!in_game())
            {
                if (!has_backup_)
                {
                    unsigned int state_size = nes_->size();
                    backup_state_.resize(state_size);
                    nes_->save(backup_state_.data());
                    has_backup_ = true;
                }
                this->frameadvance(0b00010000);
                this->frameadvance(NES_INPUT_NONE);
            }
        }

        void SMB1Logic::onReset()
        {
            // After a reset (e.g., on death), reload the savestate if we have one
            if (has_backup_)
            {
                nes_->load(backup_state_.data());
            }
            // We don't reset the has_backup_ flag here, so we can always
            // return to the start of the level on subsequent resets.
        }

        float SMB1Logic::getReward()
        {
            if (is_dead())
            {
                return -20.0f; // Large penalty for dying
            }

            // Calculate change in X position
            // current_pos = (page * 256) + x_pos
            int current_x = (static_cast<int>(current_ram_[0x006D]) << 8) | current_ram_[0x0086];
            int previous_x = (static_cast<int>(previous_ram_[0x006D]) << 8) | previous_ram_[0x0086];

            float x_reward = static_cast<float>(current_x - previous_x);

            // Give a small penalty for each frame to encourage progress
            float time_penalty = -0.1f;

            return x_reward + time_penalty;
        }

        bool SMB1Logic::isDone()
        {
            return is_dead();
        }

    } // namespace games
} // namespace hcle