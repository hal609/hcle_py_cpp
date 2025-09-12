#pragma once
#include "game_logic.hpp"

namespace hcle
{
    namespace games
    {

        class BubbleBobbleLogic : public GameLogic
        {
        public:
            BubbleBobbleLogic()
            {
                action_set = {
                    NES_INPUT_LEFT,
                    NES_INPUT_RIGHT,
                    NES_INPUT_A,
                    NES_INPUT_B};
                // action_set.resize(256);
                // std::iota(action_set.begin(), action_set.end(), 0);
            }

            GameLogic *
            clone() const override
            {
                return new BubbleBobbleLogic(*this);
            }

        private:
            static const int IN_GAME = 0x0012;
            static const int LIVES = 0x002E;
            static const int CURRENT_LEVEL = 0x0401;
            inline static const std::vector<int> SCORE_DIGITS = {0x0445, 0x0446, 0x0447, 0x0448, 0x0449, 0x044A};

            bool inGame()
            {
                return m_current_ram_ptr[IN_GAME] == 1;
            }

            bool isDead()
            {
                return (changeIn(LIVES) < 0);
            }

            bool isBusy()
            {
                return m_current_ram_ptr[IN_GAME] != 1;
            }

            double getScore(uint8_t *ram_ptr)
            {
                double score = 0.0;
                double multiplier = 1;

                for (int i = 5; i >= 0; i--)
                {
                    double score_digit = static_cast<double>(ram_ptr[SCORE_DIGITS[i]]);
                    if (score_digit == 0x27)
                        break;
                    multiplier *= 10;
                    score += score_digit * multiplier;
                }
                return score;
            }

        public:
            bool isDone() override
            {
                return isDead();
            }

            double getReward() override
            {
                double reward = -1.0;

                double score_delta = getScore(m_current_ram_ptr) - getScore(m_previous_ram.data());

                if (score_delta > 0 && score_delta < 10000)
                {
                    reward += score_delta;
                }

                if (isDead())
                    reward -= 1000;

                return (reward / 10000.0);
            }

            void onStep() override
            {
                if (inGame())
                {
                    if (!has_backup_)
                        createBackup();
                }
                else
                {
                    while (!inGame())
                    {
                        this->frameadvance(NES_INPUT_START);
                        this->frameadvance(NES_INPUT_NONE);
                    }
                }
                while (isBusy())
                {
                    this->frameadvance(NES_INPUT_NONE);
                }
            };
        };
    } // namespace games
} // namespace hcle