#include "hcle/common/display.hpp"
#include <stdexcept>

namespace hcle
{
    namespace common
    {

        Display::Display(const std::string &title, int screen_width, int screen_height, int scale)
            : m_screen_width(screen_width)
        {

            if (SDL_Init(SDL_INIT_VIDEO) < 0)
            {
                throw std::runtime_error("SDL could not initialize! SDL_Error: " + std::string(SDL_GetError()));
            }

            m_window = SDL_CreateWindow(
                title.c_str(),
                SDL_WINDOWPOS_CENTERED,
                SDL_WINDOWPOS_CENTERED,
                screen_width * scale,
                screen_height * scale,
                SDL_WINDOW_SHOWN);
            if (!m_window)
            {
                SDL_Quit();
                throw std::runtime_error("Window could not be created! SDL_Error: " + std::string(SDL_GetError()));
            }

            m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
            if (!m_renderer)
            {
                SDL_DestroyWindow(m_window);
                SDL_Quit();
                throw std::runtime_error("Renderer could not be created! SDL_Error: " + std::string(SDL_GetError()));
            }

            m_texture = SDL_CreateTexture(
                m_renderer,
                SDL_PIXELFORMAT_RGB24,
                SDL_TEXTUREACCESS_STREAMING,
                screen_width,
                screen_height);
            if (!m_texture)
            {
                SDL_DestroyRenderer(m_renderer);
                SDL_DestroyWindow(m_window);
                SDL_Quit();
                throw std::runtime_error("Texture could not be created! SDL_Error: " + std::string(SDL_GetError()));
            }
        }

        Display::~Display()
        {
            if (m_texture)
                SDL_DestroyTexture(m_texture);
            if (m_renderer)
                SDL_DestroyRenderer(m_renderer);
            if (m_window)
                SDL_DestroyWindow(m_window);
            SDL_Quit();
        }

        void Display::update(const uint8_t *pixel_data)
        {
            // Update the texture with the new frame data from the emulator
            SDL_UpdateTexture(m_texture, nullptr, pixel_data, m_screen_width * 3);
            SDL_RenderClear(m_renderer);
            // Copy the texture to the renderer
            SDL_RenderCopy(m_renderer, m_texture, nullptr, nullptr);
            // Present the renderer to the screen
            SDL_RenderPresent(m_renderer);
        }

        bool Display::processEvents()
        {
            SDL_Event e;
            while (SDL_PollEvent(&e) != 0)
            {
                printf("keysym: %d\n", e.key.keysym.sym);
                printf("esc pressed: %d\n", e.key.keysym.sym == 27);
                if (e.type == SDL_QUIT)
                {
                    return true; // Quit signal received
                }
                if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
                {
                    return true; // Quit on Escape key
                }
            }
            return false; // No quit signal
        }

    } // namespace common
} // namespace hcle
