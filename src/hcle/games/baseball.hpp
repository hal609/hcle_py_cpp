// src/hcle/games/baseball.hpp
#pragma once
#include "game_logic.hpp"
#include <vector>
#include <algorithm> // For std::max
#include <cstdint>   // For uint8_t

namespace hcle
{
   namespace games
   {

      class BaseballLogic : public GameLogic
      {
      public:
         BaseballLogic()
         {
            // Set the available actions for the agent
            action_set = {
                NES_INPUT_UP,
                NES_INPUT_DOWN,
                NES_INPUT_LEFT,
                NES_INPUT_RIGHT,
                NES_INPUT_B,
                NES_INPUT_A,
            };
         }

         // Required for polymorphic cloning
         GameLogic *clone() const override { return new BaseballLogic(*this); }

      private:
         // RAM addresses for game state
         static const int IN_GAME = 0x001E;
         static const int GAME_STATE = 0x03D0;
         static const int BATTING = 0x000F;
         static const int STRIKES = 0x0062;
         static const int BALLS = 0x0063;
         static const int OUTS = 0x0064;
         static const int SCORE1 = 0x0067;
         static const int SCORE2 = 0x0068;
         static const int IS_TEAM_2 = 0x004B;
         static const int BASES_ADDR = 0x038D;

         /**
          * @brief Checks if the game is on the title screen/menu.
          */
         bool in_menu() const
         {
            return current_ram_ptr_[IN_GAME] == 0x00;
         }

         /**
          * @brief Checks if the current player is batting.
          */
         bool is_batting() const
         {
            return current_ram_ptr_[BATTING] == 0x01;
         }

         /**
          * @brief Calculates the number of bases occupied by the batting team.
          */
         int get_bases() const
         {
            uint8_t value = current_ram_ptr_[BASES_ADDR];
            uint8_t last_four_bits = value & 0x0F; // Get the last 4 bits
            int result = static_cast<int>(last_four_bits) - 10;
            return std::max(result / 2, 0);
         }

         /**
          * @brief Gets the score for the current player's team.
          */
         long long get_score(const uint8_t *ram) const
         {
            return (ram[IS_TEAM_2] == 1) ? ram[SCORE2] : ram[SCORE1];
         }

         /**
          * @brief Gets the score for the opponent's team.
          */
         long long get_opponent_score(const uint8_t *ram) const
         {
            return (ram[IS_TEAM_2] == 1) ? ram[SCORE1] : ram[SCORE2];
         }

         /**
          * @brief Calculates the change in a RAM value, returning 1 if it increased by exactly 1, otherwise 0.
          */
         int calculate_delta(int ram_addr) const
         {
            int delta = static_cast<int>(current_ram_ptr_[ram_addr]) - static_cast<int>(previous_ram_[ram_addr]);
            return (delta == 1) ? 1 : 0;
         }

      public:
         /**
          * @brief Determines if the episode has ended. (Always false for Baseball).
          */
         bool isDone() override
         {
            return false;
         }

         /**
          * @brief Calculates the reward for the last step.
          */
         double getReward() override
         {
            double reward = 0.0;

            long long score_change = get_score(current_ram_ptr_) - get_score(previous_ram_.data());
            long long opp_score_change = get_opponent_score(current_ram_ptr_) - get_opponent_score(previous_ram_.data());

            reward += score_change * 500.0;
            reward -= opp_score_change * 500.0;

            // Note: The original Python names for strikes/outs changes appear swapped.
            // This implementation replicates the original logic, not the names.
            int balls_change = calculate_delta(BALLS);
            int outs_change_from_strikes = calculate_delta(STRIKES); // Python's `outs_change`
            int strikes_change_from_outs = calculate_delta(OUTS);    // Python's `strikes_change`

            if (is_batting())
            {
               reward += get_bases() * 100.0;
               reward -= balls_change;
               reward -= outs_change_from_strikes * 10.0;
               reward -= strikes_change_from_outs * 100.0;
            }
            else // Pitching
            {
               reward += balls_change;
               reward += outs_change_from_strikes * 10.0;
               reward += strikes_change_from_outs * 100.0;
            }

            return reward / 100.0;
         }

         /**
          * @brief Logic to execute after each step, including skipping menus.
          */
         void onStep() override
         {
            // Skip through the title screen and team select menus
            while (in_menu())
            {
               frameadvance(NES_INPUT_NONE);
               frameadvance(NES_INPUT_START);
            }
            while (current_ram_ptr_[GAME_STATE] == 0x80)
            {
               frameadvance(NES_INPUT_NONE);
               frameadvance(NES_INPUT_A);
            }

            // If the game has started and we don't have a save state, create one
            if (!has_backup_ && !in_menu() && current_ram_ptr_[GAME_STATE] != 0x80)
            {
               createBackup();
            }
         }
      };

   } // namespace games
} // namespace hcle