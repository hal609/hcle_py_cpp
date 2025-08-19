// src/hcle/games/smb1.hpp
#pragma once
#include "game_logic.hpp"

namespace hcle
{
    namespace games
    {

        class KungFuLogic : public GameLogic
        {
        public:
            KungFuLogic() { action_set = {
                                NES_INPUT_UP,
                                NES_INPUT_DOWN,
                                NES_INPUT_LEFT,
                                NES_INPUT_RIGHT,
                                NES_INPUT_B,
                                NES_INPUT_A,
                            }; }

            GameLogic *clone() const override { return new KungFuLogic(*this); }

        private:
            bool in_game() { return current_ram_ptr_[MENU] != 0x00; }
            bool is_dead() { return current_ram_ptr_[DEAD] != 0x00; }
            bool in_attract_mode() { return current_ram_ptr_[ATTRACT] == 1; }

            static const int GAME_STATE = 0x0006;
            static const int ATTRACT = 0x06B;
            static const int IN_PLAY = 0x0390;
            static const int HP = 0x04A6;
            static const int ACTIONABLE = 0x60;
            static const int MENU = 0x5C;
            static const int X_FINE = 0xD4;
            static const int X_LARGE = 0xA3;
            static const int FLOOR = 0x58;
            static const int DEAD = 0x038D;
            static constexpr int SCORE_ADDRS[] = {0x0535, 0x0534, 0x0533, 0x0532};

            int64_t score(const uint8_t *ram) const
            {
                int64_t total_score = 0;
                int64_t mult = 1;
                for (int addr : SCORE_ADDRS)
                {
                    total_score += static_cast<int64_t>(ram[addr]) * mult;
                    mult *= 10;
                }
                return total_score;
            }

            float score_change() const
            {
                return static_cast<float>(score(current_ram_ptr_) - score(previous_ram_.data()));
            }

            float x_change() const
            {
                int64_t change = static_cast<int64_t>(current_ram_ptr_[X_FINE]) - static_cast<int64_t>(previous_ram_[X_FINE]);
                if (std::abs(change) == 255)
                {
                    return -static_cast<float>(change) / 255.0f;
                }
                else if (std::abs(change) > 3)
                {
                    return 0.0f;
                }
                return static_cast<float>(change);
            }

            float hp_change() const
            {
                int64_t change = static_cast<int64_t>(current_ram_ptr_[HP]) - static_cast<int64_t>(previous_ram_[HP]);
                if (change > 0 || current_ram_ptr_[HP] == 0)
                {
                    return 0.0f;
                }
                return static_cast<float>(change);
            }

        public:
            bool isDone() override { return is_dead(); }

            float getReward() override
            {
                if (in_attract_mode())
                {
                    return 0.0f;
                }

                float reward = -0.01f; // Time penalty

                float x_reward = x_change();
                if (current_ram_ptr_[FLOOR] % 2 == 0)
                {
                    reward -= x_reward;
                }
                else
                {
                    reward += x_reward;
                }

                reward += score_change() / 100.0f;
                reward += hp_change();

                if (is_dead())
                {
                    reward -= 100.0f;
                }

                return reward;
            }
            void onStep() override
            {
                if (in_game())
                {
                    if (!has_backup_)
                        createBackup();
                }
                else
                {
                    this->frameadvance(NES_INPUT_NONE);
                    this->frameadvance(NES_INPUT_START);
                }
            }
        };

    } // namespace games
} // namespace hcle