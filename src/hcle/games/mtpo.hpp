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
                action_set.resize(256);
                std::iota(action_set.begin(), action_set.end(), 0);
                // action_set = {
                //     NES_INPUT_NONE,
                //     NES_INPUT_LEFT,
                //     NES_INPUT_B,
                //     NES_INPUT_UP | NES_INPUT_B};
            }

            GameLogic *clone() const override { return new MTPO_Logic(*this); }

        private:
            static const int MAC_HP = 0x0391;
            static const int OPP_HP = 0x0398;
            static const int TIMER_DIGIT = 0x0305;
            static const int inFight_FLAG = 0x0004; // Seems to be FF when in a fight

            bool inFight() const
            {
                return m_current_ram_ptr[inFight_FLAG] == 0xFF;
            }

            bool onMainTitle() const
            {
                return m_current_ram_ptr[inFight_FLAG] == 0x02;
            }

            void skipBetweenRounds()
            {
                if (!inFight())
                {
                    frameadvance(NES_INPUT_NONE);
                    frameadvance(NES_INPUT_START, 2);
                }
            }

            void skipTitle()
            {
                m_current_ram_ptr[0x7E0] = 0x32;
                m_current_ram_ptr[0x7E1] = 0x1A;
                m_current_ram_ptr[0x4C6] = 0x06;
                m_current_ram_ptr[0x4C7] = 0x40;
                frameadvance(NES_INPUT_NONE, 10);
                m_current_ram_ptr[0x4C8] = 0x10;
                frameadvance(NES_INPUT_NONE, 150);
                m_current_ram_ptr[0x01E] = 0x80;
                m_current_ram_ptr[0x01F] = 0x02;
                frameadvance(NES_INPUT_NONE, 20);
                frameadvance(NES_INPUT_START);
            }

        public:
            bool isDone() override
            {
                int mac_hp_change = static_cast<int>(m_current_ram_ptr[MAC_HP]) - static_cast<int>(m_previous_ram[MAC_HP]);
                return mac_hp_change < -3;
            }

            double getReward() override
            {
                int opp_hp_change = static_cast<int>(m_current_ram_ptr[OPP_HP]) - static_cast<int>(m_previous_ram[OPP_HP]);
                int mac_hp_change = static_cast<int>(m_current_ram_ptr[MAC_HP]) - static_cast<int>(m_previous_ram[MAC_HP]);

                double hit_reward = static_cast<double>(std::max(-opp_hp_change, 0));
                double health_penalty = static_cast<double>(std::max(-mac_hp_change, 0));

                return (hit_reward - health_penalty) / 100.0;
            }

            void onStep() override
            {
                if (onMainTitle())
                {
                    skipTitle();
                }
                if (!inFight())
                {
                    skipBetweenRounds();
                }
                // if (m_current_ram_ptr[0x01F] != 0x01)
                // {
                //     m_current_ram_ptr[0x01F] = 0x01;
                // }
                // if (m_current_ram_ptr[0x093] > 0x0)
                // {
                //     m_current_ram_ptr[0x092] = 0x08;
                //     m_current_ram_ptr[0x093] = 0x083;
                // }

                if (inFight() && m_current_ram_ptr[TIMER_DIGIT] != 0 && !has_backup_)
                {
                    createBackup();
                }
            }
        };

    } // namespace games
} // namespace hcle