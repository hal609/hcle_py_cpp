// src/apps/test_runner.cpp
#include <iostream>
#include <random>
#include <vector>
#include <thread>

#include "hcle/environment/preprocessed_env.hpp"

int main(int argc, char **argv)
{
    int num_envs = 1;
    std::string rom_path = "C:\\Users\\offan\\Downloads\\hcle_py_cpp\\src\\hcle\\python\\hcle_py\\roms\\smb1.bin";
    std::string game_name = "smb1";
    std::string render_mode = "human";
    int num_steps = 1000;
    std::vector<uint8_t> obs(240 * 256 * 3);

    std::vector<uint8_t> backup_state_;

    try
    {
        std::cout << "Creating HCLEEnvironment (num_envs=" << num_envs << ")...\n";
        hcle::environment::PreprocessedEnv env(rom_path, game_name, render_mode);

        const int H = 240, W = 256, C = 3;
        size_t obs_size = static_cast<size_t>(num_envs) * H * W * C;
        std::vector<uint8_t> obs(obs_size);

        env.reset();
        std::cout << "Initial environment reset complete.\n";

        size_t action_space = env.getActionSet().size();
        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> action_dist(0, (int)action_space - 1);

        uint8_t action;
        float reward;
        bool done;

        double total_reward = 0.0;

        uint8_t step = 0;
        while (true)
        {
            action = static_cast<uint8_t>(action_dist(rng));

            reward = env.step(action);
            done = env.isDone();
            env.getScreenRGB(obs.data());

            total_reward += reward;

            if ((step + 1) % 20 == 0)
            {
                std::cout << "Step " << (step + 1) << " mean reward = " << total_reward / (step + 1)
                          << " total reward = " << total_reward << "\n";
            }
            ++step;
        }

        std::cout << "Run complete. Closing.\n";
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Exception: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}
