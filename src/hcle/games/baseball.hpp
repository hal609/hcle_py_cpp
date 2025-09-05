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

         bool inMenu() const
         {
            return m_current_ram_ptr[IN_GAME] == 0x00;
         }

         bool isBatting() const
         {
            return m_current_ram_ptr[BATTING] == 0x01;
         }

         int getBases() const
         {
            uint8_t value = m_current_ram_ptr[BASES_ADDR];
            uint8_t last_four_bits = value & 0x0F; // Get the last 4 bits
            int result = static_cast<int>(last_four_bits) - 10;
            return std::max(result / 2, 0);
         }

         long long getScore(const uint8_t *ram) const
         {
            return (ram[IS_TEAM_2] == 1) ? ram[SCORE2] : ram[SCORE1];
         }

         long long getOpponentScore(const uint8_t *ram) const
         {
            return (ram[IS_TEAM_2] == 1) ? ram[SCORE1] : ram[SCORE2];
         }

         int calculateDelta(int ram_addr) const
         {
            int delta = static_cast<int>(m_current_ram_ptr[ram_addr]) - static_cast<int>(m_previous_ram[ram_addr]);
            return (delta == 1) ? 1 : 0;
         }

      public:
         bool isDone() override
         {
            return false;
         }

         double getReward() override
         {
            double reward = 0.0;

            long long score_change = getScore(m_current_ram_ptr) - getScore(m_previous_ram.data());
            long long opp_score_change = getOpponentScore(m_current_ram_ptr) - getOpponentScore(m_previous_ram.data());

            reward += score_change * 500.0;
            reward -= opp_score_change * 500.0;

            int balls_change = calculateDelta(BALLS);
            int outs_change_from_strikes = calculateDelta(OUTS);
            int strikes_change_from_outs = calculateDelta(STRIKES);

            if (isBatting())
            {
               reward += getBases() * 100.0;
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

         void onStep() override
         {
            // Skip through title screen and team select menu
            while (inMenu())
            {
               frameadvance(NES_INPUT_NONE);
               frameadvance(NES_INPUT_START);
            }
            while (m_current_ram_ptr[GAME_STATE] == 0x80)
            {
               frameadvance(NES_INPUT_NONE);
               frameadvance(NES_INPUT_A);
            }

            if (!has_backup_ && !inMenu() && m_current_ram_ptr[GAME_STATE] != 0x80)
            {
               createBackup();
            }
         }
      };

   } // namespace games
} // namespace hcle