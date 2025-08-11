#pragma once

#include <SDL.h>
#include <string>
#include <cstdint>

namespace hcle
{
    namespace common
    {

        /**
         * @class Display
         * @brief Encapsulates all SDL windowing and rendering logic.
         * This class manages the SDL_Window, SDL_Renderer, and SDL_Texture,
         * hiding the low-level SDL API from the rest of the application.
         */
        class Display
        {
        public:
            /**
             * @brief Constructs and initializes the display window.
             * @param title The title of the window.
             * @param screen_width The logical width of the content (e.g., NES screen width).
             * @param screen_height The logical height of the content (e.g., NES screen height).
             * @param scale The integer factor to scale the window by.
             */
            Display(const std::string &title, int screen_width, int screen_height, int scale);

            /**
             * @brief Destroys SDL objects and quits SDL.
             */
            ~Display();

            /**
             * @brief Updates the screen with new pixel data.
             * @param pixel_data A pointer to the raw pixel data (e.g., from the NES frame buffer).
             */
            void update(const uint8_t *pixel_data);

            /**
             * @brief Processes SDL events.
             * @return True if the user has requested to quit, false otherwise.
             */
            bool processEvents();

        private:
            SDL_Window *m_window = nullptr;
            SDL_Renderer *m_renderer = nullptr;
            SDL_Texture *m_texture = nullptr;
            int m_screen_width;
        };

    } // namespace common
} // namespace hcle
