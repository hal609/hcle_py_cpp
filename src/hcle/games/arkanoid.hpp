// src/hcle/games/arkanoid.hpp
#pragma once
#include "game_logic.hpp"
#include <vector>
#include <cstdlib>
#include <chrono>
#include <iomanip>

namespace hcle
{
    namespace games
    {

        class ArkanoidLogic : public GameLogic
        {
        public:
            ArkanoidLogic()
            {
                action_set = {
                    NES_INPUT_LEFT,
                    NES_INPUT_RIGHT,
                    NES_INPUT_A};
            }

            GameLogic *clone() const override { return new ArkanoidLogic(*this); }

        private:
            static const int CURRENT_LIVES = 0x000D;
            static const int GAME_STATE = 0x000A;            // 0x1 on title, 0x4 after start on title, 0x5 during level load, 0x6 then 0x7 during ready, 0x8 when ball on paddel, 0x10 while ball flying in game,
            static const int TITLE_WAIT_TIMER = 0x013F;      // Title screen waits until value is 0x0B before loading the first level
            static const int LEVEL_LOAD_WAIT_TIMER = 0x013B; // Each level waits for this value to be 0 before starting gameplay
            inline static const std::vector<int> P1_SCORE = {0x0370, 0x0371, 0x0372, 0x0373, 0x0374, 0x0375};

            bool inGame()
            {
                return m_current_ram_ptr[GAME_STATE] == 0x10 || m_current_ram_ptr[GAME_STATE] == 0x08;
            }

            bool onTitle()
            {
                return m_current_ram_ptr[GAME_STATE] == 0x1;
            }

            int getScore()
            {
                int score = 0;
                for (int loc : P1_SCORE)
                {
                    int digit = m_current_ram_ptr[loc];
                    score = score * 10 + digit;
                }
                return score;
            }

            int getPreScore()
            {
                int score = 0;
                for (int loc : P1_SCORE)
                {
                    int digit = m_previous_ram[loc];
                    score = score * 10 + digit;
                }
                return score;
            }

            void skipLockedStates()
            {
                if (onTitle())
                {
                    for (int i = 0; i < 18; i++)
                    {
                        m_current_ram_ptr[TITLE_WAIT_TIMER] = 0x0B; // Hack timer to 0 to skip title
                        frameadvance(NES_INPUT_START);
                    }
                }
                else if (!inGame())
                {
                    m_current_ram_ptr[LEVEL_LOAD_WAIT_TIMER] = 0x0; // Hack timer to 0 to skip wait
                    frameadvance(NES_INPUT_NONE);
                }
            }

        public:
            bool isDone() override
            {
                return (inGame() && m_current_ram_ptr[CURRENT_LIVES] < 0x03);
            }

            void onStep() override
            {
                skipLockedStates();

                if (inGame() && !has_backup_)
                {
                    createBackup();
                }
            }

            double getReward() override
            {
                double reward = -0.1; // Small penalty each frame to encorage speed

                reward += static_cast<double>(getScore() - getPreScore()) / 10;

                // Penalty for game over
                if (isDone())
                {
                    reward -= 20.0;
                }
                return reward / 100.0;
            }
        };

    } // namespace games
} // namespace hcle