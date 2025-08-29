// src/hcle/games/golf.hpp
#pragma once
#include "game_logic.hpp"
#include <vector>
#include <cmath>     // For std::min, std::max
#include <algorithm> // For std::max
#include <cstdint>

namespace hcle
{
    namespace games
    {
        class GolfLogic : public GameLogic
        {
        public:
            GolfLogic()
            {
                action_set = {
                    NES_INPUT_LEFT,
                    NES_INPUT_RIGHT,
                    NES_INPUT_UP,
                    NES_INPUT_DOWN,
                    NES_INPUT_A};
            }

            GameLogic *clone() const override { return new GolfLogic(*this); }

        private:
            // RAM addresses
            static const int SCORE = 0x000F;
            static const int GAME_STATE = 0x0002;
            static const int ON_GREEN = 0x0049;
            static const int STROKES = 0x002C;
            static const int DIST_TO_HOLE_L = 0x0058;
            static const int DIST_TO_HOLE_H = 0x0059;
            static const int DIST_ON_GREEN = 0x005E;
            static const int PAR = 0x0083;
            static const int HOLE_NUM = 0x002B;

            bool in_game() const
            {
                return current_ram_ptr_[GAME_STATE] != 0x63;
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

            float dist_to_hole(const uint8_t *ram) const
            {
                if (ram[ON_GREEN] == 1)
                {
                    return static_cast<float>(ram[DIST_ON_GREEN]);
                }
                return static_cast<float>(ram[DIST_TO_HOLE_L] | (ram[DIST_TO_HOLE_H] << 8));
            }

        public:
            bool isDone() override
            {
                return current_ram_ptr_[STROKES] > current_ram_ptr_[PAR] * 2;
            }

            double getReward() override
            {
                double reward = 0.01;

                double dist_change = dist_to_hole(current_ram_ptr_) - dist_to_hole(previous_ram_.data());
                int strokes_change = (current_ram_ptr_[STROKES] != previous_ram_[STROKES]) ? 1 : 0;
                int score_change = static_cast<int>(current_ram_ptr_[SCORE]) - static_cast<int>(previous_ram_[SCORE]);
                int hole_change = static_cast<int>(current_ram_ptr_[HOLE_NUM]) - static_cast<int>(previous_ram_[HOLE_NUM]);

                reward -= std::min(dist_change, 0.0);
                reward -= strokes_change * 1000.0;
                reward -= std::max(score_change, 0) * 10.0;
                reward += hole_change * 10000.0;

                return reward / 10000.0;
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