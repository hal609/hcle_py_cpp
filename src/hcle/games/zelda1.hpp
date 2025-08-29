// src/hcle/games/zeldalogic.hpp
#pragma once
#include "game_logic.hpp"
#include <vector>
#include <numeric>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace hcle
{
   namespace games
   {

      class Zelda1Logic : public GameLogic
      {
      public:
         Zelda1Logic()
         {
            action_set = {
                NES_INPUT_NONE,
                NES_INPUT_UP,
                NES_INPUT_DOWN,
                NES_INPUT_LEFT,
                NES_INPUT_RIGHT,
                NES_INPUT_A,
                NES_INPUT_B,
                NES_INPUT_START,
            };
         }

         GameLogic *clone() const override { return new Zelda1Logic(*this); }

      private:
         // --- Tier 1: Essential RAM Addresses ---
         static const int GAME_MODE = 0x0012;
         static const int HEART_CONTAINERS = 0x066F;
         static const int PARTIAL_HEART = 0x0670;
         static const int MAP_LOCATION = 0x00EB;
         static const int LINK_X = 0x0070;
         static const int LINK_Y = 0x0084;
         static const int RUPEES = 0x066D;
         static const int KEYS = 0x066E;
         static const int BOMBS = 0x0658;

         // --- Tier 2: Important RAM Addresses ---
         static const int KILLED_ENEMY_COUNT = 0x0627;
         // Block of memory for major item collection
         static const int ITEM_BLOCK_START = 0x0657; // Current Sword
         static const int ITEM_BLOCK_END = 0x0676;   // Magic Shield

         /**
          * @brief Checks if the game is in the main playable state.
          */
         bool in_game() const
         {
            // return true;
            return current_ram_ptr_[GAME_MODE] > 1 && current_ram_ptr_[GAME_MODE] < 14;
         }

         /**
          * @brief Skips non-playable game states like menus and screen transitions.
          */
         void skip_menus_and_transitions()
         {
            // printf("Current game mode: %d\n", current_ram_ptr_[GAME_MODE]);
            while (current_ram_ptr_[GAME_MODE] < 2) //! in_game())
            {
               frameadvance(NES_INPUT_START);
               frameadvance(NES_INPUT_NONE);
            }
            while (current_ram_ptr_[GAME_MODE] == 14)
            {
               frameadvance(NES_INPUT_A);
               frameadvance(NES_INPUT_NONE, 5);
               frameadvance(NES_INPUT_A);
               frameadvance(NES_INPUT_NONE, 5);
               frameadvance(NES_INPUT_SELECT);
               frameadvance(NES_INPUT_NONE, 5);
               frameadvance(NES_INPUT_SELECT);
               frameadvance(NES_INPUT_NONE, 5);
               frameadvance(NES_INPUT_SELECT);
               frameadvance(NES_INPUT_NONE, 60);
               frameadvance(NES_INPUT_START);
               frameadvance(NES_INPUT_NONE, 60);
               frameadvance(NES_INPUT_START);
               frameadvance(NES_INPUT_NONE, 120);
            }
         }

         float get_health(const uint8_t *ram) const
         {
            // High nibble of 0x066F is (total containers - 1)
            int num_containers = ((ram[HEART_CONTAINERS] >> 4) & 0x0F) + 1;
            // Low nibble is filled hearts, but 0x0670 is more precise
            return static_cast<float>(ram[PARTIAL_HEART]);
         }

         int check_new_items() const
         {
            int new_items = 0;
            // Iterate over the block of memory containing major items
            for (int addr = ITEM_BLOCK_START; addr <= ITEM_BLOCK_END; ++addr)
            {
               // Reward for a change from 0 (not possessed) to non-zero (possessed)
               if (previous_ram_[addr] == 0 && current_ram_ptr_[addr] > 0)
               {
                  new_items++;
               }
            }
            return new_items;
         }

      public:
         bool isDone() override
         {
            return get_health(current_ram_ptr_) == 0;
         }

         double getReward() override
         {
            // --- Penalties ---
            double health_change = get_health(current_ram_ptr_) - get_health(previous_ram_.data());
            double damage_penalty = std::min(health_change, 0.0); // Negative reward for taking damage

            // --- Rewards ---
            double rupee_reward = static_cast<double>(changeIn(RUPEES));
            double key_reward = static_cast<double>(changeIn(KEYS)) * 5.0;   // Keys are valuable
            double bomb_reward = static_cast<double>(changeIn(BOMBS)) * 0.5; // Bombs are less valuable
            double combat_reward = static_cast<double>(changeIn(KILLED_ENEMY_COUNT)) * 2.0;
            double exploration_reward = (current_ram_ptr_[MAP_LOCATION] != previous_ram_[MAP_LOCATION]) ? 10.0 : 0.0;
            double major_item_reward = static_cast<double>(check_new_items()) * 50.0; // Big reward for major items

            // Combine all reward components
            double total_reward = damage_penalty +
                                  rupee_reward +
                                  key_reward +
                                  bomb_reward +
                                  combat_reward +
                                  exploration_reward +
                                  major_item_reward;

            // Return a scaled value
            return total_reward / 10.0;
         }

         void onStep() override
         {
            skip_menus_and_transitions();
            if (in_game() && !has_backup_)
            {
               createBackup();
            }
         }
      };

   } // namespace games
} // namespace hcle