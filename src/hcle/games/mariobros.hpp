// src/hcle/games/mariobros.hpp
#pragma once
#include "game_logic.hpp"
#include <vector>
#include <cstdint>

namespace hcle
{
    namespace games
    {
        class MarioBrosLogic : public GameLogic
        {
        public:
            MarioBrosLogic()
            {
                action_set = {
                    NES_INPUT_NONE,
                    NES_INPUT_LEFT,
                    NES_INPUT_RIGHT,
                    NES_INPUT_A,
                    NES_INPUT_RIGHT | NES_INPUT_A,
                    NES_INPUT_LEFT | NES_INPUT_A,
                };
            }

            GameLogic *clone() const override { return new MarioBrosLogic(*this); }

        private:
            // RAM addresses
            static const int P1_SCORE_1 = 0x0095; // Rightmost digit
            static const int P1_SCORE_2 = 0x0096;
            static const int P1_SCORE_3 = 0x0097; // Leftmost digit
            static const int P1_LIVES = 0x0048;
            static const int TIMER = 0x002D;

            int get_lives() const
            {
                return current_ram_ptr_[P1_LIVES];
            }

            long long get_score(const uint8_t *ram) const
            {
                // The score is stored as decimal digits, not binary.
                return ram[P1_SCORE_1] +
                       ram[P1_SCORE_2] * 10 +
                       ram[P1_SCORE_3] * 100;
            }

            bool in_game() const
            {
                return get_lives() > 0;
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

        public:
            bool isDone() override
            {
                return get_lives() == 1;
            }

            double getReward() override
            {
                double reward = -0.01;
                long long score_change = get_score(current_ram_ptr_) - get_score(previous_ram_.data());
                reward += static_cast<double>(score_change);

                if (get_lives() != 2)
                {
                    reward = -20.0;
                }

                return reward / 10000.0;
            }

            void onStep() override
            {
                // Hack timer to 0 to skip wait at the start of a level
                current_ram_ptr_[TIMER] = 0;

                skip_between_rounds();

                if (in_game() && !has_backup_)
                {
                    createBackup();
                }
            }
        };
    } // namespace games
} // namespace hcle