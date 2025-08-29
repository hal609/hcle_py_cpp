// src/hcle/games/tmnt.hpp
#pragma once
#include "game_logic.hpp"
#include <vector>
#include <set>
#include <utility>   // For std::pair
#include <algorithm> // For std::max
#include <cstdint>

namespace hcle
{
    namespace games
    {
        class TMNTLogic : public GameLogic
        {
        public:
            TMNTLogic()
            {
                action_set = {
                    NES_INPUT_NONE,
                    NES_INPUT_RIGHT,
                    NES_INPUT_LEFT,
                    NES_INPUT_UP,
                    NES_INPUT_DOWN,
                    NES_INPUT_A,
                    NES_INPUT_B,
                    NES_INPUT_A | NES_INPUT_RIGHT,
                    NES_INPUT_A | NES_INPUT_LEFT,
                };
            }

            GameLogic *clone() const override { return new TMNTLogic(*this); }

        private:
            // State for tracking exploration
            std::set<std::pair<uint8_t, uint8_t>> visited_overworld_coords_;

            // RAM addresses
            static const int OVERWORLD_X = 0x0010;
            static const int OVERWORLD_Y = 0x0011;
            static const int LEVEL_X = 0x00A0;
            static const int MAP_ID = 0x0020;
            static const int BOSS_HEALTH = 0x04D0;
            static const int TURTLE_HEALTH_START = 0x0077;
            static const int LIVES = 0x0046;
            static const int IN_GAME = 0x003C;
            static const int ON_CSS = 0x0035; // On Character Select Screen

            bool in_game() const { return current_ram_ptr_[IN_GAME] == 1; }
            bool is_on_overworld() const { return current_ram_ptr_[MAP_ID] == 0x00; }

            void skip_menus()
            {
                while (!in_game() || current_ram_ptr_[ON_CSS] == 1)
                {
                    frameadvance(NES_INPUT_NONE);
                    frameadvance(NES_INPUT_START);
                }
            }

        public:
            void onReset() override
            {
                visited_overworld_coords_.clear();
            }

            bool isDone() override
            {
                // Done if lives are not at the starting value (3)
                return current_ram_ptr_[LIVES] != 3;
            }

            double getReward() override
            {
                double reward = -0.01; // Time penalty

                if (is_on_overworld())
                {
                    std::pair<uint8_t, uint8_t> coords = {current_ram_ptr_[OVERWORLD_X], current_ram_ptr_[OVERWORLD_Y]};

                    if (!visited_overworld_coords_.contains(coords))
                    {
                        reward += 0.5; // Exploration reward
                        visited_overworld_coords_.insert(coords);
                    }
                }
                else
                { // Inside a level
                    int x_progress = static_cast<int>(current_ram_ptr_[LEVEL_X]) - static_cast<int>(previous_ram_[LEVEL_X]);
                    reward += static_cast<double>(std::max(x_progress, 0)); // Progress reward
                }

                // Boss defeat reward
                if (previous_ram_[BOSS_HEALTH] > 0 && current_ram_ptr_[BOSS_HEALTH] == 0)
                {
                    reward += 50.0;
                }

                // Damage penalty
                for (int i = 0; i < 4; ++i)
                {
                    int health_addr = TURTLE_HEALTH_START + i;
                    if (current_ram_ptr_[health_addr] < previous_ram_[health_addr])
                    {
                        reward -= (current_ram_ptr_[health_addr] == 0) ? 30.0 : 5.0;
                    }
                }

                return reward / 100.0;
            }

            void onStep() override
            {
                skip_menus();
                if (in_game() && !has_backup_)
                {
                    createBackup();
                }
            }
        };
    } // namespace games
} // namespace hcle