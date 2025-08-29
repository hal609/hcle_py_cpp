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
                action_set.resize(256);
                std::iota(action_set.begin(), action_set.end(), 0);
                // action_set = {
                //     NES_INPUT_NONE,
                //     NES_INPUT_LEFT,
                //     NES_INPUT_DOWN,
                //     NES_INPUT_RIGHT,
                //     NES_INPUT_A};
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

            bool in_game()
            {
                return current_ram_ptr_[GAME_PHASE] != 0;
            }

            void skip_between_rounds()
            {
                while (!in_game())
                {
                    shuffle_rng();
                    frameadvance(NES_INPUT_START);
                    frameadvance(NES_INPUT_NONE);
                }
            }

            uint8_t score_hex_to_int(uint8_t score)
            {
                return ((score / 16) * 10) + (score % 16);
            }

            int get_score(const uint8_t *ram)
            {
                int tot_score = 0;
                tot_score += score_hex_to_int(ram[SCORE_THIRD_BYTE]);
                tot_score += score_hex_to_int(ram[SCORE_SECOND_BYTE]) * 100;
                tot_score += score_hex_to_int(ram[SCORE_FIRST_BYTE]) * 10000;
                return tot_score;
            }

            void shuffle_rng()
            {
                auto p1 = std::chrono::system_clock::now();
                std::srand(std::chrono::duration_cast<std::chrono::nanoseconds>(p1.time_since_epoch()).count());

                current_ram_ptr_[RNG] = std::rand() % 255;
                current_ram_ptr_[RNG + 1] = std::rand() % 255;
                std::vector<uint8_t> pieces = {0x02, 0x07, 0x08, 0x0A, 0x0B, 0x0E, 0x12};
                current_ram_ptr_[0x00BF] = pieces[std::rand() % 7];
                current_ram_ptr_[0x0019] = std::rand() % 255;
            }

            void onReset() override
            {
                frameadvance(NES_INPUT_NONE);
                frameadvance(NES_INPUT_NONE);
                shuffle_rng();
            }

        public:
            bool isDone() override
            {
                return current_ram_ptr_[GAME_OVER] == 0x0A;
            }

            double getReward() override
            {
                double reward = 0.01; // Small reward for surviving

                // Reward based on score change
                int current_score = get_score(current_ram_ptr_);
                int previous_score = get_score(previous_ram_.data());
                reward += static_cast<double>(current_score - previous_score);

                // Penalty for game over
                if (isDone())
                {
                    reward -= 20.0;
                }

                return reward / 100.0;
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