#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <fstream>
#include <stdexcept>
#include <random>

#include <SDL.h>

#include "hcle/environment/hcle_vector_environment.hpp"

int main(int argc, char **argv)
{
    // --- Configuration ---
    const int num_envs = 1; // Controller works with a single environment
    const std::string rom_path = "C:\\Users\\offan\\Documents\\hcle_py_cpp\\src\\hcle\\python\\hcle_py\\data";
    const std::string game_name = "arkanoid";
    const std::string render_mode = "human";
    const int num_steps = 100000;
    const int fps_limit = 60;

    // Initialize SDL for video and event handling
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
        return 1;
    }

    try
    {
        std::cout << "Creating HCLEVectorEnvironment...\n";
        hcle::environment::HCLEVectorEnvironment env(num_envs, rom_path, game_name, render_mode, 84, 84, 1, true, false, 4);

        // --- Pre-allocate memory buffers ---
        const size_t single_obs_size = env.getObservationSize();
        std::vector<uint8_t> obs_buffer(num_envs * single_obs_size);
        std::vector<double> reward_buffer(num_envs);
        std::vector<uint8_t> done_buffer(num_envs);
        std::vector<uint8_t> actions(num_envs);

        size_t action_space = env.getActionSet().size();
        printf("Action space size: %zu\n", action_space);

        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> action_dist(0, action_space - 1);

        // --- Reset environment ---
        std::cout << "Resetting environment...\n";
        env.reset(obs_buffer.data(), reward_buffer.data(), done_buffer.data());

        // --- Performance tracking ---
        double total_reward = 0.0;
        using clock = std::chrono::high_resolution_clock;
        auto run_start = clock::now();
        auto frame_start_time = clock::now();

        std::cout << "Starting main loop for " << num_steps << " steps...\n";
        for (int step = 0; step < num_steps; ++step)
        {
            for (int i = 0; i < num_envs; ++i)
                actions[i] = static_cast<uint8_t>(action_dist(rng));

            env.send(actions);
            env.recv(obs_buffer.data(), reward_buffer.data(), done_buffer.data());

            if constexpr (fps_limit > 0)
            {
                auto frame_duration = std::chrono::duration<double>(1.0 / fps_limit);
                auto end_time = clock::now();
                auto elapsed = end_time - frame_start_time;
                if (elapsed < frame_duration)
                {
                    std::this_thread::sleep_for(frame_duration - elapsed);
                }
            }
            frame_start_time = clock::now();

            if (reward_buffer[0] != 0)
            {
                std::cout << "Rewards this step: "
                          << std::fixed << std::right << std::setw(8) << std::setprecision(4)
                          << reward_buffer[0] << "\r" << std::endl; // std::flush;
            }

            total_reward += reward_buffer[0];
        }

        auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(clock::now() - run_start).count();
        double average_reward = total_reward / num_steps;

        std::cout << "\n--- Run Summary ---\n";
        std::cout << "Total steps: " << num_steps << "\n";
        std::cout << "Total time: " << elapsed << " seconds\n";
        std::cout << "Average FPS: " << (num_steps / elapsed) << "\n";
        std::cout << "Average reward per step: " << average_reward << "\n";
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Exception caught: " << ex.what() << "\n";
        SDL_Quit();
        return 1;
    }

    SDL_Quit();
    return 0;
}
