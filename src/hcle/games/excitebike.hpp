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
                NES_INPUT_LEFT,
                NES_INPUT_RIGHT,
                NES_INPUT_A, // Accelerate (Normal)
                NES_INPUT_B, // Accelerate (Turbo)
                NES_INPUT_A | NES_INPUT_LEFT,
                NES_INPUT_A | NES_INPUT_RIGHT,
                NES_INPUT_B | NES_INPUT_LEFT,
                NES_INPUT_B | NES_INPUT_RIGHT,
            };
            finish_time_ = -1; // Use -1 to indicate not finished
         }

         GameLogic *clone() const override { return new ExcitebikeLogic(*this); }

      private:
         long long finish_time_;

         // RAM addresses
         static const int RACING_FLAG = 0x004F;
         static const int PLAYER_SPEED = 0x00F3;
         static const int MOTOR_TEMP = 0x03E3;
         static const int GAME_TIMER_MIN = 0x0068;
         static const int GAME_TIMER_SEC = 0x0069;
         static const int GAME_TIMER_HUN = 0x006A;
         static const int PLAYER_STATUS = 0x00F2;

         bool in_game() const
         {
            return current_ram_ptr_[RACING_FLAG] == 0x01;
         }

         void skip_between_rounds()
         {
            while (!in_game())
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
            finish_time_ = -1;
            if (!has_backup_)
            {
               for (int i = 0; i < 30; i++)
               {
                  frameadvance(NES_INPUT_NONE);
                  frameadvance(NES_INPUT_START);
               }
            }
            // You may want to add the startup frameadvance logic from the Python _did_reset here
         }

         bool isDone() override
         {
            long long current_time = get_time(current_ram_ptr_);

            if (in_game() && finish_time_ < 0)
            {
               long long previous_time = get_time(previous_ram_.data());
               if (current_time == previous_time && current_time > 0)
               {
                  finish_time_ = current_time;
                  return true;
               }
            }
            return false;
         }

         double getReward() override
         {
            double reward = -0.01f;

            reward += static_cast<double>(current_ram_ptr_[PLAYER_SPEED]) / 10.0f;

            if (current_ram_ptr_[MOTOR_TEMP] >= 32)
            {
               reward -= 20.0f;
            }

            uint8_t status = current_ram_ptr_[PLAYER_STATUS];
            if (status != 0 && status != 4)
            {
               reward -= 5.0f;
            }

            return reward / 10000.0f;
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