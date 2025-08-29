// src/hcle/games/lolo.hpp
#pragma once
#include "game_logic.hpp"
#include <vector>
#include <cstdint>

namespace hcle
{
   namespace games
   {

      class Lolo1Logic : public GameLogic
      {
      public:
         Lolo1Logic()
         {
            action_set = {
                NES_INPUT_NONE,
                NES_INPUT_UP,
                NES_INPUT_DOWN,
                NES_INPUT_LEFT,
                NES_INPUT_RIGHT,
                NES_INPUT_A, // Use Magic Shot
                NES_INPUT_B, // Use PW Item (Bridge/Arrow/Mallet)
            };
         }

         // Required for polymorphic cloning
         GameLogic *clone() const override { return new Lolo1Logic(*this); }

      private:
         // --- RAM Address Constants ---
         static const int LIVES_REMAINING = 0x0057;
         static const int MAGIC_SHOTS = 0x0058;
         static const int TOTAL_HEART_FRAMES = 0x0086;
         static const int COLLECTED_HEART_FRAMES = 0x0087;

         /**
          * @brief Checks if the agent is in a playable state.
          * For Lolo, this is true as long as the agent has lives remaining.
          * A more specific check for menus could be added if needed.
          */
         bool in_game() const
         {
            return current_ram_ptr_[LIVES_REMAINING] > 0 && current_ram_ptr_[LIVES_REMAINING] < 32;
         }

      public:
         bool isDone() override
         {
            return current_ram_ptr_[LIVES_REMAINING] < previous_ram_[LIVES_REMAINING];
         }

         double getReward() override
         {
            double reward = 0.0;

            int hearts_collected_change = std::abs(changeIn(COLLECTED_HEART_FRAMES));
            reward += hearts_collected_change * 50.0;

            reward += std::abs(static_cast<double>(changeIn(MAGIC_SHOTS))) * 2.0;

            if (current_ram_ptr_[COLLECTED_HEART_FRAMES] == current_ram_ptr_[TOTAL_HEART_FRAMES] &&
                previous_ram_[COLLECTED_HEART_FRAMES] < previous_ram_[TOTAL_HEART_FRAMES])
            {
               reward += 200.0;
            }

            if (isDone())
            {
               reward -= 100.0;
            }

            // Scale the final reward to a reasonable range.
            return reward / 100.0;
         }

         void onStep() override
         {
            while (!in_game())
            {
               frameadvance(NES_INPUT_NONE);
               frameadvance(NES_INPUT_START);
            }
            if (in_game() && !has_backup_)
            {
               createBackup();
            }
         }
      };

   } // namespace games
} // namespace hcle