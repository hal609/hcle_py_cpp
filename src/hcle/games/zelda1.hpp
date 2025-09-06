// src/hcle/games/zeldalogic.hpp
#pragma once
#include "game_logic.hpp"
#include <vector>
#include <numeric>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include <set>

namespace hcle
{
   namespace games
   {

      class Zelda1Logic : public GameLogic
      {
      public:
         Zelda1Logic()
         {
            action_set.resize(256);
            std::iota(action_set.begin(), action_set.end(), 0);
            // action_set = {
            //     NES_INPUT_NONE,
            //     NES_INPUT_UP,
            //     NES_INPUT_DOWN,
            //     NES_INPUT_LEFT,
            //     NES_INPUT_RIGHT,
            //     NES_INPUT_A,
            //     NES_INPUT_B,
            //     //  NES_INPUT_START,
            // };
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

         std::set<int> m_visited_locations;

         bool inGame() const
         {
            // return true;
            return m_current_ram_ptr[GAME_MODE] > 1 && m_current_ram_ptr[GAME_MODE] < 14;
         }

         void onReset()
         {
            m_visited_locations.clear();
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
               // Skip addresses which are not new major items
               if ((addr) == PARTIAL_HEART || addr == RUPEES || addr == KEYS)
                  continue;

               // If number of heart containers is set to 0x22 then this is just the defaul
               // so no reward should be returned
               if (addr == HEART_CONTAINERS && m_current_ram_ptr[addr] != 0x22)
                  continue;

               // Reward for a change from lower value (not possessed) to higer value (possessed/upgraded)
               if (m_previous_ram[addr] < m_current_ram_ptr[addr])
               {
                  new_items++;
               }
            }
            return new_items;
         }

         int newMapLocationFound()
         {
            if (m_visited_locations.find(m_current_ram_ptr[MAP_LOCATION]) == m_visited_locations.end())
            {
               m_visited_locations.insert(m_current_ram_ptr[MAP_LOCATION]);
               return 1;
            }
            return 0;
         }

      public:
         bool isDone() override
         {
            return getHealth(m_current_ram_ptr) == 0;
         }

         double getReward() override
         {
            double reward = 0; //-0.001;

            // --- Penalties ---
            double health_change = getHealth(m_current_ram_ptr) - getHealth(m_previous_ram.data());
            double damage_penalty = std::min(health_change, 0.0); // Negative reward for taking damage

            // --- Rewards ---
            double rupee_reward = static_cast<double>(changeIn(RUPEES));
            double key_reward = static_cast<double>(changeIn(KEYS)) * 5.0;   // Keys are valuable
            double bomb_reward = static_cast<double>(changeIn(BOMBS)) * 0.5; // Bombs are less valuable
            // double combat_reward = static_cast<double>(changeIn(KILLED_ENEMY_COUNT)) * 2.0;
            double exploration_reward = newMapLocationFound() * 5.0;
            double major_item_reward = static_cast<double>(checkNewItems()) * 100.0; // Big reward for major items

            // Combine all reward components
            reward += damage_penalty / 100 +
                      rupee_reward +
                      key_reward +
                      bomb_reward +
                      //  combat_reward +
                      exploration_reward +
                      major_item_reward;

            if (isDone())
            {
               reward -= 20.0;
            }
            // Return scaled value
            return reward / 1000.0;
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