#pragma once

#include <SDL.h>
#include <cstdint>
#include <unordered_map>

namespace hcle
{
    namespace common
    {

        /**
         * @class NESController
         * @brief Handles keyboard input and translates it into an 8-bit NES controller state.
         *
         * This class maps SDL keyboard scancodes to the corresponding NES button bitmasks.
         * It allows for checking the combined state of all pressed keys in a single byte,
         * which can be sent to the emulator environment.
         */
        class NESController
        {
        public:
            // Standard 8-bit NES controller button bitmasks
            static constexpr uint8_t NES_INPUT_NONE = 0x00;
            static constexpr uint8_t NES_INPUT_RIGHT = 0x01;
            static constexpr uint8_t NES_INPUT_LEFT = 0x02;
            static constexpr uint8_t NES_INPUT_DOWN = 0x04;
            static constexpr uint8_t NES_INPUT_UP = 0x08;
            static constexpr uint8_t NES_INPUT_START = 0x10;
            static constexpr uint8_t NES_INPUT_SELECT = 0x20;
            static constexpr uint8_t NES_INPUT_B = 0x40;
            static constexpr uint8_t NES_INPUT_A = 0x80;

            /**
             * @brief Constructs the NESController and sets up default key mappings.
             */
            NESController()
            {
                // Default key mappings. These can be customized if needed.
                key_map_[SDL_SCANCODE_RIGHT] = NES_INPUT_RIGHT;
                key_map_[SDL_SCANCODE_LEFT] = NES_INPUT_LEFT;
                key_map_[SDL_SCANCODE_DOWN] = NES_INPUT_DOWN;
                key_map_[SDL_SCANCODE_UP] = NES_INPUT_UP;
                key_map_[SDL_SCANCODE_RETURN] = NES_INPUT_START;  // Enter/Return key
                key_map_[SDL_SCANCODE_RSHIFT] = NES_INPUT_SELECT; // Right Shift
                key_map_[SDL_SCANCODE_Z] = NES_INPUT_B;           // 'Z' key for B button
                key_map_[SDL_SCANCODE_X] = NES_INPUT_A;           // 'X' key for A button
            }

            /**
             * @brief Polls the current keyboard state and returns the combined 8-bit action.
             * @return A uint8_t where each bit represents a pressed NES button.
             */
            uint8_t getAction() const
            {
                // Update SDL's internal event state to get the latest keyboard snapshot.
                SDL_PumpEvents();
                const Uint8 *key_states = SDL_GetKeyboardState(NULL);

                uint8_t combined_action = NES_INPUT_NONE;

                // Iterate through our key map and check if any mapped keys are pressed.
                for (const auto &pair : key_map_)
                {
                    SDL_Scancode key = pair.first;
                    uint8_t button_mask = pair.second;
                    if (key_states[key])
                    {
                        // If the key is pressed, add its button bit to our combined action
                        // using a bitwise OR.
                        combined_action |= button_mask;
                    }
                }

                return combined_action;
            }

        private:
            std::unordered_map<SDL_Scancode, uint8_t> key_map_;
        };

    } // namespace common
} // namespace hcle