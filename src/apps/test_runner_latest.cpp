#include <iostream>
#include <random>
#include <vector>
#include <thread>
#include <chrono>
#include <fstream>
#include <stdexcept>

#include "hcle/environment/hcle_vector_environment.hpp"

void saveObsToRaw(std::vector<uint8_t> *obs_buffer)
{
    std::cout << "\n--- Saving observation at step 100 to test_output.raw ---\n";
    std::ofstream raw_image("test_output.raw", std::ios::binary);
    if (raw_image)
    {
        // Write the entire batch of observations from the buffer
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
    const int num_envs = 1;
    const std::string rom_path = "C:\\Users\\offan\\Downloads\\hcle_py_cpp\\src\\hcle\\python\\hcle_py\\roms\\smb1.bin";
    const std::string game_name = "smb1";
    const std::string render_mode = "human"; // Use "human" for window
    const int num_steps = 1000;

    try
    {
        std::cout << "Creating HCLEVectorEnvironment (num_envs=" << num_envs << ")...\n";
        hcle::environment::HCLEVectorEnvironment env(num_envs, rom_path, game_name, render_mode, 84, 84, 4, true, false, 4);
        printf("HCLEVectorEnvironment created.\n");

        // --- Pre-allocate memory buffers for results ---
        const size_t single_obs_size = env.getObservationSize();
        std::vector<uint8_t> obs_buffer(num_envs * single_obs_size);
        std::vector<float> reward_buffer(num_envs);
        // Use uint8_t for the done buffer to ensure it has a .data() method.
        std::vector<uint8_t> done_buffer(num_envs);

        // --- Reset environments to get initial state ---
        std::cout << "Resetting environments...\n";
        env.reset(obs_buffer.data(), reward_buffer.data(), done_buffer.data());
        std::cout << "Environments reset.\n";

        // --- Setup for the main loop ---
        const size_t action_space_size = env.getActionSet().size();
        std::cout << "Action space size: " << action_space_size << "\n";

        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> action_dist(0, static_cast<int>(action_space_size) - 1);
        std::vector<int> actions(num_envs);

        // --- Performance tracking ---
        double total_reward = 0.0;
        using clock = std::chrono::steady_clock;
        auto run_start = clock::now();

        std::cout << "Starting main loop for " << num_steps << " steps...\n";
        for (int step = 0; step < num_steps; ++step)
        {
            // Generate a batch of random actions
            for (int i = 0; i < num_envs; ++i)
            {
                actions[i] = action_dist(rng);
            }

            // Asynchronously send actions and synchronously wait for results
            env.send(actions);
            env.recv(obs_buffer.data(), reward_buffer.data(), done_buffer.data());

            if (step == 100)
            {
                // Save the observation at step 100 to a raw file
                saveObsToRaw(&obs_buffer);
            }
            // Accumulate rewards for performance metric (optional)
            for (int i = 0; i < num_envs; ++i)
            {
                total_reward += reward_buffer[i];
            }

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
        return 1;
    }

    return 0;
}
