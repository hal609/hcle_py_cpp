// src/hcle/games/tetris.hpp
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

        class TetrisLogic : public GameLogic
        {
        public:
            TetrisLogic()
            {
                // action_set.resize(256);
                // std::iota(action_set.begin(), action_set.end(), 0);
                action_set = {
                    NES_INPUT_DOWN,
                    NES_INPUT_NONE,
                    NES_INPUT_LEFT,
                    NES_INPUT_RIGHT,
                    NES_INPUT_A};
            }

            GameLogic *clone() const override { return new TetrisLogic(*this); }

        private:
            static const int GAME_PHASE = 0x0048;
            static const int SCORE_THIRD_BYTE = 0x0053;
            static const int SCORE_SECOND_BYTE = 0x0054;
            static const int SCORE_FIRST_BYTE = 0x0055;
            static const int GAME_OVER = 0x0058; // Becomes 0x0A when the game ends
            static const int RNG = 0x0017;
            static const int NEXT_PIECE = 0x00BF;

            bool inGame()
            {
                return m_current_ram_ptr[GAME_PHASE] != 0;
            }

            void skipBetweenRounds()
            {
                while (!inGame())
                {
                    shuffleRNG();
                    frameadvance(NES_INPUT_START);
                    frameadvance(NES_INPUT_NONE);
                }
            }

            uint8_t scoreHexToInt(uint8_t score)
            {
                return ((score / 16) * 10) + (score % 16);
            }

            int getScore(const uint8_t *ram)
            {
                int tot_score = 0;
                tot_score += scoreHexToInt(ram[SCORE_THIRD_BYTE]);
                tot_score += scoreHexToInt(ram[SCORE_SECOND_BYTE]) * 100;
                tot_score += scoreHexToInt(ram[SCORE_FIRST_BYTE]) * 10000;
                return tot_score;
            }

            int getLineCount(const uint8_t *ram)
            {
                int tot_score = 0;
                tot_score += scoreHexToInt(ram[0x0050]);
                tot_score += scoreHexToInt(ram[0x0051]) * 100;
                return tot_score;
            }

            void shuffleRNG()
            {
                auto p1 = std::chrono::system_clock::now();
                std::srand(std::chrono::duration_cast<std::chrono::nanoseconds>(p1.time_since_epoch()).count());

                m_current_ram_ptr[RNG] = std::rand() % 255;
                m_current_ram_ptr[RNG + 1] = std::rand() % 255;
                std::vector<uint8_t> pieces = {0x02, 0x07, 0x08, 0x0A, 0x0B, 0x0E, 0x12};
                m_current_ram_ptr[0x00BF] = pieces[std::rand() % 7];
                m_current_ram_ptr[0x0019] = std::rand() % 255;
            }

            void onReset() override
            {
                frameadvance(NES_INPUT_NONE);
                shuffleRNG();
            }

        public:
            bool isDone() override
            {
                return m_current_ram_ptr[GAME_OVER] > 0x0;
            }

            double getReward() override
            {
                double reward = 0.1; // Small reward for surviving
                if (m_current_ram_ptr[0x0041] < m_previous_ram[0x0048] && m_previous_ram[0x0041] > 0)
                {
                    // printf("Piece dropped at %d\n", m_current_ram_ptr[0x0041]);
                    // printf("Adding reward %f\n", (static_cast<float>(m_current_ram_ptr[0x0041]) - 10.0f) / 200.0f);
                    reward += -1 + ((static_cast<float>(m_previous_ram[0x0041])) / 20.0f);
                }

                // Reward based on score change
                // int current_score = getScore(m_current_ram_ptr);
                // int previous_score = getScore(m_previous_ram.data());
                // reward += static_cast<double>(current_score - previous_score);

                int current_lines = getLineCount(m_current_ram_ptr);
                int previous_lines = getLineCount(m_previous_ram.data());
                reward += static_cast<double>(current_lines - previous_lines) * 100;

                // Penalty for game over
                if (isDone())
                {
                    reward -= 20.0;
                }
                return reward / 100.0;
            }

            void onStep() override
            {
                skipBetweenRounds();

                if (inGame() && !has_backup_)
                {
                    createBackup();
                }
            }
        };

    } // namespace games
} // namespace hcle