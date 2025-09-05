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
         static const int GAME_MODE = 0x0012;
         static const int HEART_CONTAINERS = 0x066F;
         static const int PARTIAL_HEART = 0x0670;
         static const int MAP_LOCATION = 0x00EB;
         static const int LINK_X = 0x0070;
         static const int LINK_Y = 0x0084;
         static const int RUPEES = 0x066D;
         static const int KEYS = 0x066E;
         static const int BOMBS = 0x0658;

         static const int KILLED_ENEMY_COUNT = 0x0627;
         static const int ITEM_BLOCK_START = 0x0657; // Current Sword
         static const int ITEM_BLOCK_END = 0x0676;   // Magic Shield

         bool inGame() const
         {
            // return true;
            return m_current_ram_ptr[GAME_MODE] > 1 && m_current_ram_ptr[GAME_MODE] < 14;
         }

         void skipMenusAndTransitions()
         {
            // printf("Current game mode: %d\n", m_current_ram_ptr[GAME_MODE]);
            while (m_current_ram_ptr[GAME_MODE] < 2) //! inGame())
            {
               frameadvance(NES_INPUT_START);
               frameadvance(NES_INPUT_NONE);
            }
            while (m_current_ram_ptr[GAME_MODE] == 14)
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

         float getHealth(const uint8_t *ram) const
         {
            // High nibble of 0x066F is (total containers - 1)
            int num_containers = ((ram[HEART_CONTAINERS] >> 4) & 0x0F) + 1;
            // Low nibble is filled hearts, but 0x0670 is more precise
            return static_cast<float>(ram[PARTIAL_HEART]);
         }

         int checkNewItems() const
         {
            int new_items = 0;
            // Iterate over the block of memory containing major items
            for (int addr = ITEM_BLOCK_START; addr <= ITEM_BLOCK_END; ++addr)
            {
               // Reward for a change from 0 (not possessed) to non-zero (possessed)
               if (m_previous_ram[addr] == 0 && m_current_ram_ptr[addr] > 0)
               {
                  new_items++;
               }
            }
            return new_items;
         }

      public:
         bool isDone() override
         {
            return getHealth(m_current_ram_ptr) == 0;
         }

         double getReward() override
         {
            // --- Penalties ---
            double health_change = getHealth(m_current_ram_ptr) - getHealth(m_previous_ram.data());
            double damage_penalty = std::min(health_change, 0.0); // Negative reward for taking damage

            // --- Rewards ---
            double rupee_reward = static_cast<double>(changeIn(RUPEES));
            double key_reward = static_cast<double>(changeIn(KEYS)) * 5.0;   // Keys are valuable
            double bomb_reward = static_cast<double>(changeIn(BOMBS)) * 0.5; // Bombs are less valuable
            double combat_reward = static_cast<double>(changeIn(KILLED_ENEMY_COUNT)) * 2.0;
            double exploration_reward = (m_current_ram_ptr[MAP_LOCATION] != m_previous_ram[MAP_LOCATION]) ? 10.0 : 0.0;
            double major_item_reward = static_cast<double>(checkNewItems()) * 50.0; // Big reward for major items

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
            skipMenusAndTransitions();
            if (inGame() && !has_backup_)
            {
               createBackup();
            }
         }
      };

   } // namespace games
} // namespace hcle