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
    int num_envs = 5;
    std::string rom_path = "C:\\Users\\offan\\Downloads\\hcle_py_cpp\\src\\hcle\\python\\hcle_py\\roms\\smb1.bin";
    std::string game_name = "smb1";
    std::string render_mode = "rgb_array";
    int fps_limit = -1;
    int num_steps = 100; // 1000;

    std::vector<uint8_t> backup_state_;

    try
    {
        std::cout << "Creating HCLEVectorEnvironment (num_envs=" << num_envs << ")...\n";
        hcle::environment::HCLEVectorEnvironment env(num_envs, rom_path, game_name, render_mode);

        const int H = 240, W = 256, C = 3;
        size_t obs_bytes = static_cast<size_t>(num_envs) * H * W * C;
        std::vector<uint8_t> obs(obs_bytes);

        std::vector<hcle::vector::Timestep> timesteps;

        // reset (fills the obs buffer)
        std::cout << "!!!!!!!!!!!!!!!!!! About to call initial reset. !!!!!!!!!!!!!!!!!!\n";
        env.reset();
        std::cout << "Environment reset complete.\n";

        printf("About to get action space from vector env.\n");
        std::vector<uint8_t> action_set = env.getActionSet();
        printf("Returned to test_runner with action set of type %s with size %zu\n", typeid(action_set).name(), action_set.size());
        size_t action_space = action_set.size();
        action_space = 2;
        printf("Action space size: %zu\n", action_space);
        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> action_dist(0, (int)action_space - 1);

        std::vector<int> actions(num_envs);
        std::vector<float> rewards(num_envs);
        std::vector<uint8_t> dones_u8(num_envs);
        bool *dones_ptr = reinterpret_cast<bool *>(dones_u8.data());

        double total_reward = 0.0;
        using clock = std::chrono::steady_clock;
        auto run_start = clock::now();

        printf("!!!!!!!!!!!! About to enter step loop in test_runner !!!!!!!!!!!!!!\n");
        for (int step = 0; step < num_steps; ++step)
        {
            // random actions
            for (int i = 0; i < num_envs; ++i)
                actions[i] = static_cast<int>(action_dist(rng));

            // call step (fills obs, rewards, dones)
            env.send(actions);
            timesteps = env.recv();

            // unsigned int state_size = env.nes_->size();
            // backup_state_.resize(state_size);
            // env.nes_->save(backup_state_.data());
            // printf("Backup size: %u\n", state_size);

            // simple reporting
            double mean_reward = 0.0;
            for (float r : rewards)
                mean_reward += r;
            mean_reward /= (double)num_envs;
            total_reward += mean_reward;

            if ((step + 1) % 20 == 0)
            {
                std::cout << "Step " << (step + 1) << " mean reward = " << mean_reward
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
