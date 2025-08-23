// src/hcle/games/tetris.hpp
#pragma once
#include "game_logic.hpp"
#include <vector>

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
                    NES_INPUT_NONE,
                    NES_INPUT_LEFT,
                    NES_INPUT_DOWN,
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

            bool in_game()
            {
                return current_ram_ptr_[GAME_PHASE] != 0;
            }

            void skip_between_rounds()
            {
                while (!in_game())
                {
                    frameadvance(NES_INPUT_NONE);
                    frameadvance(NES_INPUT_NONE);
                    frameadvance(NES_INPUT_START);
                    frameadvance(NES_INPUT_START);
                }
            }

            long long get_score(const uint8_t *ram)
            {
                // The score is stored in little-endian format across three bytes.
                return ram[SCORE_FIRST_BYTE] |
                       (static_cast<long long>(ram[SCORE_SECOND_BYTE]) << 8) |
                       (static_cast<long long>(ram[SCORE_THIRD_BYTE]) << 16);
            }

        public:
            bool isDone() override
            {
                return current_ram_ptr_[GAME_OVER] == 0x0A;
            }

            float getReward() override
            {
                float reward = 0.01f; // Small reward for surviving

                // Reward based on score change
                long long current_score = get_score(current_ram_ptr_);
                long long previous_score = get_score(previous_ram_.data());
                reward += static_cast<float>(current_score - previous_score) / 1000.0f;

                // Penalty for game over
                if (isDone())
                {
                    reward -= 20.0f;
                }

                return reward / 100.0f;
            }

            void onStep() override
            {
                if (!in_game())
                {
                    skip_between_rounds();
                }

                if (in_game() && !has_backup_)
                {
                    createBackup();
                }
            }
        };

    } // namespace games
} // namespace hcle