// src/hcle/games/smb2.hpp
#pragma once
#include "game_logic.hpp"
#include <vector>
#include <algorithm> // For std::max
#include <cstdint>

namespace hcle
{
    namespace games
    {
        class SMB2Logic : public GameLogic
        {
        public:
            SMB2Logic()
            {
                action_set = {
                    NES_INPUT_NONE,
                    NES_INPUT_LEFT,
                    NES_INPUT_RIGHT,
                    NES_INPUT_UP,
                    NES_INPUT_DOWN,
                    NES_INPUT_A,
                    NES_INPUT_B,
                    NES_INPUT_DOWN | NES_INPUT_B,
                    NES_INPUT_RIGHT | NES_INPUT_A,
                    NES_INPUT_RIGHT | NES_INPUT_B,
                    NES_INPUT_LEFT | NES_INPUT_A,
                    NES_INPUT_LEFT | NES_INPUT_B,
                };
            }
            GameLogic *clone() const override { return new SMB2Logic(*this); }

        private:
            // RAM addresses
            static const int GAME_STATE = 0x00CD;
            static const int PLAYER_X_PAGE = 0x0014;
            static const int PLAYER_X_POS = 0x0028;
            static const int PLAYER_HEALTH = 0x04C2;
            static const int LIVES = 0x04ED;
            static const int LEVEL_TRANSITION = 0x04EC;
            static const int CURRENT_LEVEL = 0x0531;
            static const int CURRENT_AREA = 0x04E7;

            bool in_game() const { return current_ram_ptr_[GAME_STATE] != 0; }

            void skip_between_rounds()
            {
                while (!in_game())
                {
                    frameadvance(NES_INPUT_START);
                    frameadvance(NES_INPUT_NONE);
                    frameadvance(NES_INPUT_A);
                    frameadvance(NES_INPUT_NONE);
                }
            }

            long long get_progress_score(const uint8_t *ram) const
            {
                long long level = ram[CURRENT_LEVEL];
                long long area = ram[CURRENT_AREA];
                long long x_page = ram[PLAYER_X_PAGE];
                long long x_pos = ram[PLAYER_X_POS];
                return (level * 100000) + (area * 10000) + (x_page * 256) + x_pos;
            }

        public:
            bool isDone() override
            {
                return current_ram_ptr_[LEVEL_TRANSITION] == 0x02 || current_ram_ptr_[LIVES] < previous_ram_[LIVES];
            }

            double getReward() override
            {
                long long current_progress = get_progress_score(current_ram_ptr_);
                long long previous_progress = get_progress_score(previous_ram_.data());
                double progress_reward = static_cast<double>(current_progress - previous_progress);

                double damage_penalty = (current_ram_ptr_[PLAYER_HEALTH] < previous_ram_[PLAYER_HEALTH]) ? -25.0 : 0.0;
                double finish_bonus = (current_ram_ptr_[LEVEL_TRANSITION] == 0x03) ? 100.0 : 0.0;

                return std::max(progress_reward, 0.0) + damage_penalty + finish_bonus;
            }

            void onStep() override
            {
                skip_between_rounds();
                if (in_game() && !has_backup_)
                {
                    createBackup();
                }
            }
        };
    } // namespace games
} // namespace hcle