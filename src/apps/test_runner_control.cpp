#include <iostream>
// #include <random> // No longer needed for random actions
#include <vector>
#include <thread>
#include <chrono>
#include <fstream>
#include <stdexcept>

// [ADD THIS] - Include the main SDL header for keyboard input
#include <SDL.h>

#include "hcle/environment/hcle_vector_environment.hpp"

void saveObsToRaw(std::vector<uint8_t> *obs_buffer)
{
    std::cout << "\n--- Saving observation at step 100 to test_output.raw ---\n";
    std::ofstream raw_image("test_output.raw", std::ios::binary);
    if (raw_image)
    {
        raw_image.write(reinterpret_cast<const char *>(obs_buffer->data()), obs_buffer->size());
        raw_image.close();
        std::cout << "Save complete.\n";
    }
    else
    {
        std::cerr << "Error: Could not open test_output.raw for writing.\n";
    }
}

int main(int argc, char **argv)
{
    // --- Configuration ---
    // Note: This code assumes num_envs = 1 for keyboard control
    const int num_envs = 1;
    const std::string rom_path = "C:\\Users\\offan\\Downloads\\hcle_py_cpp\\src\\hcle\\python\\hcle_py\\roms\\smb1.bin";
    const std::string game_name = "smb1";
    const std::string render_mode = "human";
    const int num_steps = 1000;

    // [ADD THIS] - SDL must be initialized to handle events.
    // The environment likely does this, but it's safe to ensure it's done.
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
        return 1;
    }

    try
    {
        std::cout << "Creating HCLEVectorEnvironment (num_envs=" << num_envs << ")...\n";
        hcle::environment::HCLEVectorEnvironment env(num_envs, rom_path, game_name, render_mode, 84, 84, 4, true, false, 4);
        printf("HCLEVectorEnvironment created.\n");

        // --- Pre-allocate memory buffers for results ---
        const size_t single_obs_size = env.getObservationSize();
        std::vector<uint8_t> obs_buffer(num_envs * single_obs_size);
        std::vector<float> reward_buffer(num_envs);
        std::vector<uint8_t> done_buffer(num_envs);

        // --- Reset environments to get initial state ---
        std::cout << "Resetting environments...\n";
        env.reset(obs_buffer.data(), reward_buffer.data(), done_buffer.data());
        std::cout << "Environments reset.\n";

        // --- Setup for the main loop ---
        const size_t action_space_size = env.getActionSet().size();
        std::cout << "Action space size: " << action_space_size << "\n";
        std::vector<int> actions(num_envs);

        // --- Performance tracking ---
        double total_reward = 0.0;
        using clock = std::chrono::steady_clock;
        auto run_start = clock::now();

        std::cout << "Starting main loop for " << num_steps << " steps...\n";
        std::cout << "Controls: [Left Arrow], [Right Arrow], [Spacebar for Jump]\n";
        for (int step = 0; step < num_steps; ++step)
        {
            // --- [MODIFIED BLOCK] Get keyboard input instead of random actions ---

            // 1. Update SDL's internal event state
            SDL_PumpEvents();

            // 2. Get a snapshot of the current keyboard state
            const Uint8 *key_states = SDL_GetKeyboardState(NULL);

            // 3. Determine the action. Default to 0 (no-op).
            int player_action = 0;

            // This mapping is based on a common action set for SMB1.
            // You can customize the keys and actions here.
            if (key_states[SDL_SCANCODE_RIGHT])
            {
                player_action = 1; // Right
            }
            else if (key_states[SDL_SCANCODE_LEFT])
            {
                player_action = 1; // Left
            }
            else if (key_states[SDL_SCANCODE_SPACE])
            {
                player_action = 1; // A (Jump)
            }

            // Since num_envs is 1, we just set the first action.
            actions[0] = player_action;
            // --- [END MODIFIED BLOCK] ---

            // Asynchronously send actions and synchronously wait for results
            env.send(actions);
            env.recv(obs_buffer.data(), reward_buffer.data(), done_buffer.data());

            // Accumulate rewards for performance metric (optional)
            for (int i = 0; i < num_envs; ++i)
            {
                total_reward += reward_buffer[i];
            }
            printf("Step %d: Total Reward = %.2f\n", step + 1, total_reward);

            if ((step + 1) % 100 == 0)
            {
                std::cout << "Step " << (step + 1) << " complete.\n";
            }
        }

        auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(clock::now() - run_start).count();
        double average_reward = total_reward / (num_steps * num_envs);

        std::cout << "\n--- Run Summary ---\n";
        std::cout << "Total steps: " << num_steps * num_envs << "\n";
        std::cout << "Total time: " << elapsed << " seconds\n";
        std::cout << "Average FPS: " << ((num_steps * num_envs) / elapsed) << "\n";
        std::cout << "Average reward per step: " << average_reward << "\n";
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Exception caught: " << ex.what() << "\n";
        SDL_Quit(); // Clean up SDL on error
        return 1;
    }

    SDL_Quit(); // Clean up SDL before exiting
    return 0;
}