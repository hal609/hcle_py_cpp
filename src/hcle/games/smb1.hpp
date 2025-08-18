// src/hcle/games/smb1.hpp
#pragma once
#include "game_logic.hpp"

namespace hcle
{
    namespace games
    {

        class SMB1Logic : public GameLogic
        {
        public:
            SMB1Logic() { action_set = {NES_INPUT_RIGHT | NES_INPUT_B,
                                        NES_INPUT_RIGHT | NES_INPUT_B | NES_INPUT_A}; }

            GameLogic *SMB1Logic::clone() const override { return new SMB1Logic(*this); }

        private:
            bool in_game() { return current_ram_ptr_[LEVEL_LOADING] == 3 && current_ram_ptr_[GAME_MODE] != 0; }
            bool is_dead() { return current_ram_ptr_[PLAYER_STATE] == 0x0B || current_ram_ptr_[Y_VIEWPORT] > 0x1; }

            static const int PLAYER_STATE = 0x000E;
            static const int Y_VIEWPORT = 0x00B5;
            static const int GAME_MODE = 0x0770;
            static const int CURRENT_PAGE = 0x006D;
            static const int X_POS = 0x0086;
            static const int LEVEL_LOADING = 0x0772;
            static const int LEVEL_NUM = 0x0760;
            static const int WORLD_NUM = 0x075F;
            static const int COINS = 0x075E;
            static const int POWERUP_STATE = 0x0756;

        public:
            bool isDone() override { return is_dead(); }

            float getReward() override
            {
                if (is_dead())
                    return -20.0f;

                // Calculate change in X position
                int current_x = (static_cast<int>(current_ram_ptr_[CURRENT_PAGE]) << 8) | current_ram_ptr_[X_POS];
                int previous_x = (static_cast<int>(previous_ram_[CURRENT_PAGE]) << 8) | previous_ram_[X_POS];

                float x_reward = static_cast<float>(current_x - previous_x);
                float level_reward = changeIn(LEVEL_NUM);
                float world_reward = changeIn(WORLD_NUM);
                float powerup_reward = changeIn(POWERUP_STATE);
                float coin_reward = changeIn(COINS);

                // Give a small penalty for each frame to encourage progress
                float time_penalty = -0.1f;

                return x_reward + level_reward * 50.0f + world_reward * 100.0f + powerup_reward * 10.0f + coin_reward + time_penalty;
            }

            void onStep() override
            {
                if (in_game())
                {
                    if (!has_backup_)
                        createBackup();
                }
                else
                {
                    this->frameadvance(NES_INPUT_START);
                    this->frameadvance(NES_INPUT_NONE);
                }
            }
        };

    } // namespace games
} // namespace hcle