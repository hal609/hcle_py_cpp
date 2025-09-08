// src/hcle/games/excitebike.hpp
#pragma once
#include "game_logic.hpp"
#include <vector>
#include <cstdint>

namespace hcle
{
   namespace games
   {
      class ExcitebikeLogic : public GameLogic
      {
      public:
         ExcitebikeLogic()
         {
            action_set = {
                NES_INPUT_NONE,
                NES_INPUT_A, // Accelerate (Normal)
                NES_INPUT_B, // Accelerate (Turbo)
                NES_INPUT_A | NES_INPUT_DOWN,
                NES_INPUT_A | NES_INPUT_UP,
                NES_INPUT_A | NES_INPUT_LEFT,
                NES_INPUT_A | NES_INPUT_RIGHT,
                NES_INPUT_B | NES_INPUT_LEFT,
                NES_INPUT_B | NES_INPUT_RIGHT,
            };
            // action_set.resize(256);
            // std::iota(action_set.begin(), action_set.end(), 0);
         }

         GameLogic *clone() const override { return new ExcitebikeLogic(*this); }

      private:
         // RAM addresses
         static const int RACING_FLAG = 0x004F;
         static const int PLAYER_SPEED = 0x00F3;
         static const int MOTOR_TEMP = 0x03E3;
         static const int GAME_TIMER_MIN = 0x0068;
         static const int GAME_TIMER_SEC = 0x0069;
         static const int GAME_TIMER_HUN = 0x006A;
         static const int PLAYER_STATUS = 0x00F2;
         static const int FINISH_POSITION = 0x00D; // if finish pos is greater than 3 then you cannot continue

         bool inGame() const
         {
            return m_current_ram_ptr[RACING_FLAG] == 0x01;
         }

         void skip_between_rounds()
         {
            if (!inGame())
            {
               frameadvance(NES_INPUT_A);
            }
         }

         long long get_time(const uint8_t *ram) const
         {
            return ram[GAME_TIMER_HUN] | (ram[GAME_TIMER_SEC] << 8) | (ram[GAME_TIMER_MIN] << 16);
         }

      public:
         void onReset() override
         {
            if (!has_backup_)
            {
               for (int i = 0; i < 30; i++)
               {
                  frameadvance(NES_INPUT_NONE);
                  frameadvance(NES_INPUT_START);
               }
            }
         }

         bool isDone() override
         {
            return m_current_ram_ptr[FINISH_POSITION] >= 0x3;
         }

         double getReward() override
         {
            double reward = -0.01;

            reward += static_cast<double>(m_current_ram_ptr[PLAYER_SPEED]) / 10.0;

            if (m_current_ram_ptr[MOTOR_TEMP] >= 32)
            {
               reward -= 20.0;
            }

            uint8_t status = m_current_ram_ptr[PLAYER_STATUS];
            if (status != 0 && status != 4)
            {
               reward -= 5.0;
            }

            if (isDone())
            {
               reward -= 20.0;
            }

            return reward / 10000.0;
         }

         void onStep() override
         {
            skip_between_rounds();
            if (inGame() && !has_backup_)
            {
               createBackup();
            }
         }
      };
   } // namespace games
} // namespace hcle