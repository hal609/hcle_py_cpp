// src/apps/test_runner.cpp
#include <iostream>
#include <random>
#include <vector>
#include <thread>
#include <chrono>
#include <fstream> // For writing to a file

#include "hcle/environment/hcle_vector_environment.hpp"

int main(int argc, char **argv)
{
    int num_envs = 1;
    std::string rom_path = "C:\\Users\\offan\\Downloads\\hcle_py_cpp\\src\\hcle\\python\\hcle_py\\roms\\smb1.bin";
    std::string game_name = "smb1";
    std::string render_mode = "rgb_array";
    int num_steps = 1000;

    std::vector<uint8_t> backup_state_;

    try
    {
        std::cout << "Creating HCLEVectorEnvironment (num_envs=" << num_envs << ")...\n";

        hcle::environment::HCLEVectorEnvironment env(num_envs, rom_path, game_name, render_mode);

        std::vector<hcle::vector::Timestep> timesteps;

        timesteps = env.reset();

        size_t action_space = env.getActionSet().size();
        printf("Action space size: %zu\n", action_space);

        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> action_dist(0, (int)action_space - 1);

        std::vector<int> actions(num_envs);

        double total_reward = 0.0;
        using clock = std::chrono::steady_clock;
        auto run_start = clock::now();

        for (int step = 0; step < num_steps; ++step)
        {
            for (int i = 0; i < num_envs; ++i)
                actions[i] = static_cast<int>(action_dist(rng));

            env.send(actions);
            timesteps = env.recv();

            total_reward += timesteps[0].reward / (double)num_envs;

            if ((step + 1) % 20 == 0)
            {
                std::cout << "Step " << (step + 1) << " step reward = " << timesteps[0].reward
                          << " total reward = " << total_reward << "\n";
            }
        }
        auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(clock::now() - run_start).count();

        std::cout << "Run complete in " << elapsed << " seconds. Average fps = " << ((num_steps * num_envs) / elapsed) << ". Closing.\n";
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Exception: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}
