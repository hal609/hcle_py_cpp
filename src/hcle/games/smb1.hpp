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
            SMB1Logic()
            {
                // action_set.resize(256);
                // std::iota(action_set.begin(), action_set.end(), 0);
                action_set = {
                    NES_INPUT_LEFT,
                    NES_INPUT_RIGHT | NES_INPUT_B,
                    NES_INPUT_RIGHT | NES_INPUT_B | NES_INPUT_A};
            }

            GameLogic *clone() const override { return new SMB1Logic(*this); }

        private:
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
            static const int PRE_LEVEL_TIMER = 0x07A0;
            static const int CHANGE_AREA_TIMER = 0x06DE;
            static const int TIME_H = 0x07F8;
            static const int TIME_M = 0x07F9;
            static const int TIME_L = 0x07FA;

            bool in_game() { return current_ram_ptr_[LEVEL_LOADING] == 3 && current_ram_ptr_[GAME_MODE] != 0; }

            bool is_dead()
            {
                return current_ram_ptr_[PLAYER_STATE] == 0x0B || // Standard death
                       current_ram_ptr_[PLAYER_STATE] == 0x06 || // Death animation
                       current_ram_ptr_[Y_VIEWPORT] > 0x1;       // Fell off screen
            }

            bool is_busy()
            {
                uint8_t state = current_ram_ptr_[PLAYER_STATE];
                return (state >= 0x00 && state <= 0x05);
            }

            bool is_world_over() { return current_ram_ptr_[GAME_MODE] == 0x14; }

            int get_time()
            {
                return current_ram_ptr_[TIME_H] * 100 +
                       current_ram_ptr_[TIME_M] * 10 +
                       current_ram_ptr_[TIME_L];
            }

            void runout_prelevel_timer() { current_ram_ptr_[PRE_LEVEL_TIMER] = 0; }

            void skip_change_area()
            {
                uint8_t timer = current_ram_ptr_[CHANGE_AREA_TIMER];
                if (timer > 1 && timer < 255)
                {
                    current_ram_ptr_[CHANGE_AREA_TIMER] = 1;
                }
            }

            void skip_occupied_states()
            {
                while (is_busy() || is_world_over())
                {
                    runout_prelevel_timer();
                    frameadvance(NES_INPUT_NONE);
                }
            }

            void skip_end_of_world()
            {
                if (is_world_over())
                {
                    int time = get_time();
                    while (get_time() == time)
                    {
                        frameadvance(NES_INPUT_NONE);
                    }
                }
            }

        public:
            bool isDone() override { return is_dead(); }

            float getReward() override
            {
                if (is_dead())
                    return -0.2f;

                // Calculate change in X position
                int current_x = (static_cast<int>(current_ram_ptr_[CURRENT_PAGE]) << 8) | current_ram_ptr_[X_POS];
                int previous_x = (static_cast<int>(previous_ram_[CURRENT_PAGE]) << 8) | previous_ram_[X_POS];

                float x_reward = static_cast<float>(current_x - previous_x);
                float level_reward = std::abs(changeIn(LEVEL_NUM));
                float world_reward = std::abs(changeIn(WORLD_NUM));
                float powerup_reward = std::abs(changeIn(POWERUP_STATE));
                float coin_reward = std::abs(changeIn(COINS));

                float time_penalty = -0.1f;

                float reward = x_reward + level_reward * 100.0f + powerup_reward * 10.0f + coin_reward + time_penalty;
                return reward / 100.0f;
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
                skip_occupied_states();
                skip_change_area();
                skip_end_of_world();
            }
        };

    } // namespace games
} // namespace hcle