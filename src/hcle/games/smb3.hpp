// src/hcle/games/smb3.hpp
#pragma once
#include "game_logic.hpp"
#include <vector>
#include <algorithm> // For std::max
#include <cstdint>

namespace hcle
{
    namespace games
    {
        class SMB3Logic : public GameLogic
        {
        public:
            SMB3Logic()
            {
                // action_set.resize(256);
                // std::iota(action_set.begin(), action_set.end(), 0);
                action_set = {
                    NES_INPUT_NONE,
                    NES_INPUT_LEFT,
                    NES_INPUT_RIGHT,
                    NES_INPUT_RIGHT | NES_INPUT_B,
                    NES_INPUT_RIGHT | NES_INPUT_A,
                    NES_INPUT_RIGHT | NES_INPUT_B | NES_INPUT_A,
                    NES_INPUT_A};
            }
            GameLogic *clone() const override { return new SMB3Logic(*this); }

        private:
            // RAM addresses
            static const int X_POS_HI = 0x0075;
            static const int X_POS_LO = 0x0090;
            static const int LIVES = 0x0736;
            static const int IN_LEVEL_TIMER = 0x05EE;
            static const int P_METER = 0x03DD;
            static const int WIN_FLAG = 0x066F;
            static const int IS_ON_MAP = 0x0014;
            static const int MAP_SCREEN_Y = 0x7976;
            static const int MAP_SCREEN_X = 0x797A;
            static const int WORLD_NUM = 0x0727;
            static const int IS_DYING = 0x00F1;
            static const int TITLE_STATE = 0x00DE;

            bool inGame() const
            {
                return m_current_ram_ptr[IS_ON_MAP] == 0 && m_current_ram_ptr[IN_LEVEL_TIMER] > 0;
            }

            void advance_n_frames(int n, uint8_t action = 0)
            {
                for (int i = 0; i < n; ++i)
                {
                    frameadvance(action);
                    if (inGame())
                        break;
                }
            }

            void skip_between_rounds()
            {
                while (!inGame())
                {
                    if (m_current_ram_ptr[TITLE_STATE] != 0)
                    {
                        frameadvance(NES_INPUT_NONE);
                        frameadvance(NES_INPUT_START);
                    }
                    else if (m_current_ram_ptr[WORLD_NUM] == 0)
                    {
                        uint8_t map_x = m_current_ram_ptr[MAP_SCREEN_X];
                        uint8_t map_y = m_current_ram_ptr[MAP_SCREEN_Y];

                        if (map_x == 32 && map_y == 64)
                        { // Start of world
                            frameadvance(NES_INPUT_NONE, 240);
                            frameadvance(NES_INPUT_RIGHT);
                            frameadvance(NES_INPUT_NONE, 60);
                            frameadvance(NES_INPUT_UP);
                            frameadvance(NES_INPUT_NONE, 60);
                            frameadvance(NES_INPUT_A);
                        }
                        else if ((map_x == 64 || map_x == 128) && map_y == 32)
                        { // Level 1/2
                            frameadvance(NES_INPUT_RIGHT);
                            frameadvance(NES_INPUT_A);
                        }
                        else if (map_x == 160 && map_y == 32)
                        { // Level 3
                            frameadvance(NES_INPUT_NONE, 240 * 4);
                            frameadvance(NES_INPUT_RIGHT);
                            frameadvance(NES_INPUT_NONE, 120);
                            frameadvance(NES_INPUT_DOWN);
                            frameadvance(NES_INPUT_NONE, 120);
                            frameadvance(NES_INPUT_LEFT);
                            frameadvance(NES_INPUT_NONE, 120);
                            frameadvance(NES_INPUT_A);
                        }
                        else if (map_x == 160 && map_y == 64)
                        { // Level 4
                            frameadvance(NES_INPUT_NONE, 240);
                            frameadvance(NES_INPUT_LEFT);
                            frameadvance(NES_INPUT_NONE, 60);
                            frameadvance(NES_INPUT_DOWN);
                            frameadvance(NES_INPUT_NONE, 60);
                            frameadvance(NES_INPUT_LEFT);
                            frameadvance(NES_INPUT_NONE, 60);
                            frameadvance(NES_INPUT_A);
                        }
                        else
                        {
                            frameadvance(NES_INPUT_NONE);
                        }
                    }
                    else
                    {
                        frameadvance(NES_INPUT_NONE);
                    }
                    if (isDone())
                        return;
                }
            }

            long long get_mario_pos(const uint8_t *ram) const
            {
                return (static_cast<long long>(ram[X_POS_HI]) << 8) | ram[X_POS_LO];
            }

        public:
            bool isDone() override
            {
                return m_current_ram_ptr[LIVES] < m_previous_ram[LIVES] || m_current_ram_ptr[IS_DYING] != 0;
            }

            double getReward() override
            {
                double reward = -0.1;
                double x_pos_change = static_cast<double>(get_mario_pos(m_current_ram_ptr) - get_mario_pos(m_previous_ram.data()));

                if (x_pos_change > 10.0 || x_pos_change < -10.0)
                    x_pos_change = 0;
                reward += x_pos_change;

                if (isDone())
                    reward -= 20.0;
                if (m_current_ram_ptr[WIN_FLAG] > 0)
                    reward += 50.0;
                if (m_current_ram_ptr[P_METER] > m_previous_ram[P_METER])
                    reward += 0.5;

                return reward;
            }

            void onStep() override
            {
                skip_between_rounds();
                if (inGame() && !has_backup_)
                {
                    createBackup();
                }
            }
        };
    } // namespace games
} // namespace hcle