// src/hcle/games/mtpo.hpp
#pragma once
#include "game_logic.hpp"
#include <vector>
#include <algorithm> // For std::max
#include <cstdint>

namespace hcle
{
    namespace games
    {
        class MTPO_Logic : public GameLogic
        {
        public:
            MTPO_Logic()
            {
                action_set = {
                    NES_INPUT_NONE,
                    NES_INPUT_LEFT,
                    NES_INPUT_B,
                    NES_INPUT_UP | NES_INPUT_B};
            }

            GameLogic *clone() const override { return new MTPO_Logic(*this); }

        private:
            // RAM addresses
            static const int MAC_HP = 0x0391;
            static const int OPP_HP = 0x0398;
            static const int TIMER_DIGIT = 0x0305;
            static const int IN_FIGHT_FLAG = 0x0004; // Seems to be FF when in a fight

            bool in_fight() const
            {
                return current_ram_ptr_[IN_FIGHT_FLAG] == 0xFF;
            }

            void skip_between_rounds()
            {
                while (!in_fight())
                {
                    frameadvance(NES_INPUT_NONE);
                    frameadvance(NES_INPUT_NONE);
                    frameadvance(NES_INPUT_START);
                    frameadvance(NES_INPUT_START);
                }
            }

        public:
            bool isDone() override
            {
                int mac_hp_change = static_cast<int>(current_ram_ptr_[MAC_HP]) - static_cast<int>(previous_ram_[MAC_HP]);
                return mac_hp_change < -3;
            }

            double getReward() override
            {
                int opp_hp_change = static_cast<int>(current_ram_ptr_[OPP_HP]) - static_cast<int>(previous_ram_[OPP_HP]);
                int mac_hp_change = static_cast<int>(current_ram_ptr_[MAC_HP]) - static_cast<int>(previous_ram_[MAC_HP]);

                double hit_reward = static_cast<double>(std::max(-opp_hp_change, 0));
                double health_penalty = static_cast<double>(std::max(-mac_hp_change, 0));

                return hit_reward - health_penalty;
            }

            void onStep() override
            {
                // This logic is slightly different from the Python version to be more robust.
                // It ensures we skip any non-fight states.
                if (!in_fight())
                {
                    skip_between_rounds();
                }

                if (in_fight() && current_ram_ptr_[TIMER_DIGIT] != 0 && !has_backup_)
                {
                    createBackup();
                }
            }
        };

    } // namespace games
} // namespace hcle