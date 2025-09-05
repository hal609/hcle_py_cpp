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

         GameLogic *clone() const override { return new Lolo1Logic(*this); }

      private:
         static const int LIVES_REMAINING = 0x0057;
         static const int MAGIC_SHOTS = 0x0058;
         static const int TOTAL_HEART_FRAMES = 0x0086;
         static const int COLLECTED_HEART_FRAMES = 0x0087;

         bool inGame() const
         {
            return m_current_ram_ptr[LIVES_REMAINING] > 0 && m_current_ram_ptr[LIVES_REMAINING] < 32;
         }

      public:
         bool isDone() override
         {
            return m_current_ram_ptr[LIVES_REMAINING] < m_previous_ram[LIVES_REMAINING];
         }

         double getReward() override
         {
            double reward = 0.0;

            int hearts_collected_change = std::abs(changeIn(COLLECTED_HEART_FRAMES));
            reward += hearts_collected_change * 50.0;

            reward += std::abs(static_cast<double>(changeIn(MAGIC_SHOTS))) * 2.0;

            if (m_current_ram_ptr[COLLECTED_HEART_FRAMES] == m_current_ram_ptr[TOTAL_HEART_FRAMES] &&
                m_previous_ram[COLLECTED_HEART_FRAMES] < m_previous_ram[TOTAL_HEART_FRAMES])
            {
               reward += 200.0;
            }

            if (isDone())
            {
               reward -= 100.0;
            }

            // Scale final reward
            return reward / 100.0;
         }

         void onStep() override
         {
            while (!inGame())
            {
               frameadvance(NES_INPUT_NONE);
               frameadvance(NES_INPUT_START);
            }
            if (inGame() && !has_backup_)
            {
               createBackup();
            }
         }
      };

   } // namespace games
} // namespace hcle