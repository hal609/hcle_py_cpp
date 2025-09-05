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

            GameLogic *
            clone() const override
            {
                return new SMB1Logic(*this);
            }

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
            static const int PLAYER_FLOAT_STATE = 0x001D; // set to 3 when sliding down flagpole
            inline static const std::vector<int> ENEMY_TYPE_ADDRESSES = {0x0016, 0x0017, 0x0018, 0x0019, 0x001A};
            inline static const std::vector<int> STAGE_OVER_ENEMIES = {0x2D, 0x31}; // Bowser = 0x2D, Flagpole = 0x31

            bool inGame() { return m_current_ram_ptr[LEVEL_LOADING] == 3 && m_current_ram_ptr[GAME_MODE] != 0; }

            bool is_dead()
            {
                return m_current_ram_ptr[PLAYER_STATE] == 0x0B || // Standard death
                       m_current_ram_ptr[PLAYER_STATE] == 0x06 || // Death animation
                       m_current_ram_ptr[Y_VIEWPORT] > 0x1;       // Fell off screen
            }

            bool is_busy()
            {
                uint8_t state = m_current_ram_ptr[PLAYER_STATE];
                return (state >= 0x00 && state <= 0x05);
            }

            bool is_world_over() { return m_current_ram_ptr[GAME_MODE] == 0x14; }

            int get_time()
            {
                return m_current_ram_ptr[TIME_H] * 100 +
                       m_current_ram_ptr[TIME_M] * 10 +
                       m_current_ram_ptr[TIME_L];
            }

            void runout_prelevel_timer() { m_current_ram_ptr[PRE_LEVEL_TIMER] = 0; }

            bool is_stage_over(uint8_t *ram_pointer)
            {
                for (const int &address : ENEMY_TYPE_ADDRESSES)
                {
                    if (count(STAGE_OVER_ENEMIES.begin(), STAGE_OVER_ENEMIES.end(), ram_pointer[address]) > 0)
                    {
                        return ram_pointer[0x001D] == 3;
                    }
                }
                return false;
            }

        public:
            bool isDone() override
            {
                return is_dead();
            }

            double getReward() override
            {
                double reward = 0.0;

                if (is_dead())
                    reward -= 10;

                // Calculate change in X position
                int current_x = (static_cast<int>(m_current_ram_ptr[CURRENT_PAGE]) << 8) | m_current_ram_ptr[X_POS];
                int previous_x = (static_cast<int>(m_previous_ram[CURRENT_PAGE]) << 8) | m_previous_ram[X_POS];

                double x_reward = static_cast<float>(current_x - previous_x);
                x_reward = (x_reward < -3) ? 0 : x_reward;
                double level_reward = std::abs(changeIn(LEVEL_NUM));
                double powerup_reward = std::abs(changeIn(POWERUP_STATE));
                double coin_reward = std::abs(changeIn(COINS));

                double time_penalty = -0.1;

                reward += x_reward + (level_reward * 500.0) + (powerup_reward * 10.0) + coin_reward + time_penalty;
                return (reward / 500.0);
            }

            void onStep() override
            {
                if (inGame())
                {
                    if (!has_backup_)
                        createBackup();
                }
                else
                {
                    this->frameadvance(NES_INPUT_START);
                    this->frameadvance(NES_INPUT_NONE);
                }
                if (is_busy() || is_world_over())
                {
                    runout_prelevel_timer();
                }
                uint8_t timer = m_current_ram_ptr[CHANGE_AREA_TIMER];
                if (timer > 1 && timer < 255)
                {
                    m_current_ram_ptr[CHANGE_AREA_TIMER] = 1;
                }
            }
        };

    } // namespace games
} // namespace hcle