#include "hcle/games/smb1.hpp"
#include "hcle/emucore/nes.hpp" // For access to the NES object
#include <vector>

const uint16_t PLAYER_STATE = 0x000E;
const uint16_t GAME_MODE = 0x0770; // 00 - Start demo, 01 - Start normal, 02 - End current world, 03 - End game (dead)
const uint16_t ON_PRELEVEL = 0x075E;
const uint16_t LEVEL_LOADING = 0x0772;
const uint16_t Y_VIEWPORT = 0x00B5; // Greater than 1 means off screen
const uint16_t CURRENT_PAGE = 0x006D;
const uint16_t X_POS = 0x0086;

namespace hcle
{
    namespace games
    {
        bool SMB1Logic::in_game() { return current_ram_[LEVEL_LOADING] == 3 && current_ram_[GAME_MODE] != 0; }
        bool SMB1Logic::is_dead() { return current_ram_[PLAYER_STATE] == 0x0B || current_ram_[Y_VIEWPORT] > 0x1; }
        bool SMB1Logic::isDone() { return is_dead(); }

        void SMB1Logic::onStep()
        {
            if (in_game())
            {
                if (!has_backup_)
                {
                    unsigned int state_size = nes_->size();
                    backup_state_.resize(state_size);
                    nes_->save(backup_state_.data());
                    has_backup_ = true;
                }
            }
            else
            {
                this->frameadvance(NES_INPUT_START);
                this->frameadvance(NES_INPUT_NONE);
            }
        }

        bool SMB1Logic::onReset()
        {
            if (has_backup_)
            {
                nes_->load(backup_state_.data());
                return true;
            }
            return false;
        }

        float SMB1Logic::getReward()
        {
            if (is_dead())
                return -20.0f;

            // Calculate change in X position
            int current_x = (static_cast<int>(current_ram_[CURRENT_PAGE]) << 8) | current_ram_[X_POS];
            int previous_x = (static_cast<int>(previous_ram_[CURRENT_PAGE]) << 8) | previous_ram_[X_POS];

            float x_reward = static_cast<float>(current_x - previous_x);

            // Give a small penalty for each frame to encourage progress
            float time_penalty = -0.1f;

            return x_reward + time_penalty;
        }

    } // namespace games
} // namespace hcle