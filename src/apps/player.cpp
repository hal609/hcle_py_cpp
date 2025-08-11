// src/apps/player.cpp

#include "hcle/emucore/nes.hpp"
#include "hcle/common/display.hpp"
#include <iostream>

// Helper function to get keyboard state remains the same
uint8_t get_controller_state()
{
    const Uint8 *keys = SDL_GetKeyboardState(nullptr);
    uint8_t state = 0;
    if (keys[SDL_SCANCODE_X])
        state |= 0x80; // A
    if (keys[SDL_SCANCODE_Z])
        state |= 0x40; // B
    if (keys[SDL_SCANCODE_A])
        state |= 0x20; // Select
    if (keys[SDL_SCANCODE_S])
        state |= 0x10; // Start
    if (keys[SDL_SCANCODE_UP])
        state |= 0x08; // Up
    if (keys[SDL_SCANCODE_DOWN])
        state |= 0x04; // Down
    if (keys[SDL_SCANCODE_LEFT])
        state |= 0x02; // Left
    if (keys[SDL_SCANCODE_RIGHT])
        state |= 0x01; // Right
    return state;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <path_to_rom>" << std::endl;
        return 1;
    }
    std::string rom_path = argv[1];

    try
    {
        // --- Initialization ---
        cynes::NES nes(rom_path.c_str());
        hcle::common::Display display("CyNES Player", 256, 240, 3);

        // --- Main Loop ---
        bool quit = false;
        while (!quit)
        {
            // 1. Process Events: Check if the user wants to close the window.
            quit = display.processEvents();

            // 2. Get Input: Read keyboard state for the controller.
            uint8_t controller = get_controller_state();

            // 3. Step Emulator: Advance the game by one frame.
            nes.step(controller, 1);

            // 4. Update Display: Send the new frame to our display class to be rendered.
            display.update(nes.get_frame_buffer());
        }
    }
    catch (const std::runtime_error &e)
    {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
