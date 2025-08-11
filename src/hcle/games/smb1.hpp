// src/hcle/games/smb1.hpp
#pragma once
#include "hcle/environment/game_logic.hpp"

namespace hcle
{
    namespace games
    {

        class SMB1Logic : public GameLogic
        {
        public:
            SMB1Logic(); // Constructor can set up the action set
            float getReward() override;
            bool isDone() override;
            void onStep() override; // For checking if in-game to make savestate
            void onReset() override;
            const std::vector<uint8_t> &getActionSet() override { return action_set_; }

        private:
            bool in_game();
            bool is_dead();

            // RAM Addresses
            static const int PLAYER_STATE = 0x000E;
            static const int Y_VIEWPORT = 0x00B5;
            static const int GAME_MODE = 0x0770;
            static const int CURRENT_PAGE = 0x006D;
            static const int X_POS = 0x0086;

            std::vector<uint8_t> action_set_;
            bool has_backup_ = false;

            std::vector<uint8_t> backup_state_;
        };

    } // namespace games
} // namespace hcle