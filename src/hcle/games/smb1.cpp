#include "hcle/games/smb1.hpp"
#include "hcle/emucore/nes.hpp" // For access to the NES object
#include <vector>

namespace hcle
{
    namespace games
    {

        bool SMB1Logic::has_backup_ = false;
        std::vector<uint8_t> SMB1Logic::backup_state_;

        bool SMB1Logic::in_game() { return current_ram_ptr_[LEVEL_LOADING] == 3 && current_ram_ptr_[GAME_MODE] != 0; }
        bool SMB1Logic::is_dead() { return current_ram_ptr_[PLAYER_STATE] == 0x0B || current_ram_ptr_[Y_VIEWPORT] > 0x1; }
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

        void SMB1Logic::reset()
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

        float SMB1Logic::getReward()
        {
            if (is_dead())
                return -20.0f;

            // Calculate change in X position
            int current_x = (static_cast<int>(current_ram_ptr_[CURRENT_PAGE]) << 8) | current_ram_ptr_[X_POS];
            int previous_x = (static_cast<int>(previous_ram_[CURRENT_PAGE]) << 8) | previous_ram_[X_POS];

            float x_reward = static_cast<float>(current_x - previous_x) * 0.1f;

            // Give a small penalty for each frame to encourage progress
            float time_penalty = -0.1f;

            return x_reward + time_penalty;
        }

    } // namespace games
} // namespace hcle