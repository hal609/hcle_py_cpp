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
         static const int IN_MENU = 0x008E;
         static const int VIRUS_COUNT = 0x008F;
         static const int GAME_STATE = 0x0090;

         /**
          * @brief Checks if the game is currently in a menu.
          */
         bool in_menu() const
         {
            return current_ram_ptr_[IN_MENU] == 0x01;
         }

         /**
          * @brief Gets the number of viruses remaining on screen.
          */
         int get_virus_count(const uint8_t *ram) const
         {
            return ram[VIRUS_COUNT];
         }

      public:
         /**
          * @brief Determines if the episode has ended (level clear or game over).
          */
         bool isDone() override
         {
            // Episode is done if the game state indicates a win or a loss.
            uint8_t game_state = current_ram_ptr_[GAME_STATE];
            return game_state == 0x03 || game_state == 0x04;
         }

         /**
          * @brief Calculates the reward for the last step.
          */
         double getReward() override
         {
            double reward = 0.0;
            uint8_t game_state = current_ram_ptr_[GAME_STATE];

            // Reward for clearing viruses
            int virus_change = get_virus_count(previous_ram_.data()) - get_virus_count(current_ram_ptr_);
            reward += virus_change * 100.0;

            // Big reward for clearing the level
            if (game_state == 0x03) // Level Clear
            {
               reward += 500.0;
            }
            // Big penalty for losing
            else if (game_state == 0x04) // Game Over
            {
               reward -= 500.0;
            }

            return reward / 100.0;
         }

         /**
          * @brief Logic to execute after each step, including skipping menus.
          */
         void onStep() override
         {
            // Skip through the title screen and level select menus
            while (in_menu())
            {
               frameadvance(NES_INPUT_START);
               frameadvance(NES_INPUT_NONE);
            }

            // If the game has started and we don't have a save state, create one
            if (!has_backup_ && !in_menu())
            {
               createBackup();
            }
         }
      };

   } // namespace games
} // namespace hcle