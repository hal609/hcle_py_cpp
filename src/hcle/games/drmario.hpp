// src/hcle/games/drmario.hpp
#pragma once
#include "game_logic.hpp"
#include <vector>
#include <cstdint> // For uint8_t

namespace hcle
{
   namespace games
   {

      class DrMarioLogic : public GameLogic
      {
      public:
         DrMarioLogic()
         {
            // action_set.resize(256);
            // std::iota(action_set.begin(), action_set.end(), 0);
            // Set the available actions for the agent
            action_set = {
                NES_INPUT_LEFT,
                NES_INPUT_RIGHT,
                NES_INPUT_A, // Rotate Right
                NES_INPUT_B, // Rotate Left
            };
         }

         // Required for polymorphic cloning
         GameLogic *clone() const override { return new DrMarioLogic(*this); }

      private:
         // RAM addresses for game state
         static const int MODE = 0x0046;
         static const int IN_MENU = 0x008E;
         static const int VIRUS_COUNT = 0x008F;
         static const int GAME_STATE = 0x0090;
         const std::vector<int> SCORE_BYTES = {0x072E, 0x072D, 0x072C, 0x072B, 0x072A, 0x0729, 0x0728};

         /**
          * @brief Checks if the game is currently in a menu.
          */
         bool in_menu() const
         {
            return m_current_ram_ptr[IN_MENU] == 0x01;
         }
         bool inGame() const
         {
            return m_current_ram_ptr[MODE] > 0x01;
         }

         bool playing() const
         {
            return m_current_ram_ptr[MODE] == 0x04;
         }

         bool game_over() const
         {
            return m_current_ram_ptr[MODE] == 0x07;
         }

         int get_virus_count(const uint8_t *ram) const
         {
            return ram[VIRUS_COUNT];
         }

         int get_current_score()
         {
            std::string score_str;
            score_str.reserve(SCORE_BYTES.size()); // Pre-allocate memory for efficiency.

            // Iterate through the addresses, read from RAM, and append to the string.
            for (int loc : SCORE_BYTES)
            {
               // Check if the location is within the bounds of the RAM vector.
               if (loc < 2048)
               {
                  score_str += std::to_string(m_current_ram_ptr[loc]);
               }
            }

            // Convert the concatenated string to a 64-bit integer.
            // Return 0 if the string is empty to avoid an exception from stoll.
            return score_str.empty() ? 0 : std::stoll(score_str);
         }

         int get_previous_score()
         {
            std::string score_str;
            score_str.reserve(SCORE_BYTES.size()); // Pre-allocate memory for efficiency.

            // Iterate through the addresses, read from RAM, and append to the string.
            for (int loc : SCORE_BYTES)
            {
               // Check if the location is within the bounds of the RAM vector.
               if (loc < m_previous_ram.size())
               {
                  score_str += std::to_string(m_previous_ram.at(loc));
               }
            }

            // Convert the concatenated string to a 64-bit integer.
            // Return 0 if the string is empty to avoid an exception from stoll.
            return score_str.empty() ? 0 : std::stoll(score_str);
         }

         int get_score_change()
         {
            return get_current_score() - get_previous_score();
         }

      public:
         bool isDone() override
         {
            return game_over();
         }

         double getReward() override
         {
            double reward = -0.01;

            // Reward for clearing viruses
            // int virus_change = get_virus_count(m_previous_ram.data()) - get_virus_count(m_current_ram_ptr);
            reward += get_score_change();

            if (game_over())
            {
               reward -= 500.0;
            }

            return reward / 500.0;
         }

         /**
          * @brief Logic to execute after each step, including skipping menus.
          */
         void onStep() override
         {
            // Skip through the title screen and level select menus
            while (!inGame())
            {
               frameadvance(NES_INPUT_START);
               frameadvance(NES_INPUT_NONE);
            }

            // If the game has started and we don't have a save state, create one
            if (!has_backup_ && !inGame())
            {
               createBackup();
            }
         }
      };

   } // namespace games
} // namespace hcle