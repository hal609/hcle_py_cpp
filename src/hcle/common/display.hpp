#pragma once

#include <SDL.h>
#include <string>
#include <cstdint>
#include <vector>
#include <memory>

#include "exceptions.hpp"

namespace hcle
{
    namespace common
    {
        class Display
        {
        public:
            Display(const std::string &title, int screen_width, int screen_height, int scale);
            ~Display();

            void update(const uint8_t *pixel_data, bool grayscale);
            bool processEvents();

            static void update_window(std::unique_ptr<Display> &display, const uint8_t *frame_ptr, bool grayscale)
            {
                display->update(frame_ptr, grayscale);
                if (display->processEvents())
                {
                    throw WindowClosedException::WindowClosedException();
                }
            }

        private:
            SDL_Window *m_window = nullptr;
            SDL_Renderer *m_renderer = nullptr;
            SDL_Texture *m_texture = nullptr;
            int m_screen_width;
            int m_screen_height;
            std::vector<uint8_t> m_display_buffer;
        };

    } // namespace common
} // namespace hcle
