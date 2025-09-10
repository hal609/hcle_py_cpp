// src/apps/test_runner.cpp
#include <iostream>
#include <random>
#include <vector>
#include <thread>
#include <fstream>

#include "hcle/environment/preprocessed_env.hpp"

int main(int argc, char **argv)
{
    int num_envs = 1;
    std::string rom_path = "C:\\Users\\offan\\Documents\\hcle_py_cpp\\src\\hcle\\python\\hcle_py\\data";
    std::string game_name = "bubblebobble";
    std::string render_mode = "human";
    int num_steps = 1000;
    const int H = 240, W = 256, C = 3;
    size_t obs_size = static_cast<size_t>(num_envs) * H * W * C;
    std::vector<uint8_t> obs(obs_size);

    std::vector<uint8_t> backup_state_;
    int step = 0;

    try
    {
        std::cout << "Creating HCLEEnvironment (num_envs=" << num_envs << ")...\n";
        hcle::environment::PreprocessedEnv env(rom_path, game_name, 84, 84, 4, false, false, 1, 2500);
        env.createWindow();
        env.reset(obs.data());
        std::cout << "Initial environment reset complete.\n";

        size_t action_space_size = env.getActionSet().size();
        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> action_dist(0, static_cast<int>(action_space_size) - 1);

        uint8_t action;
        double reward;
        bool done;

        std::ofstream log_file;

        std::string filename = std::format("{}_random_actions.csv", game_name);
        std::ifstream myFile(filename);
        if (myFile.is_open())
        {
            log_file.open(filename, std::ios::app);
        }
        else
        {
            log_file.open(filename);
            log_file << "total_reward,num_steps_in_episode\n";
        }

        for (int run = 0; run < 20; run++)
        {
            env.reset(obs.data());
            double total_reward = 0.0;

            step = 0;
            while (true)
            {
                action = static_cast<uint8_t>(action_dist(rng));

                env.step(action, obs.data());
                done = env.isDone();
                reward = env.getReward();

                total_reward += reward;

                // if ((step + 1) % 20 == 0)
                // {
                //     std::cout << "Step " << (step + 1) << " mean reward = " << total_reward / (step + 1)
                //               << " total reward = " << total_reward << "\n";
                // }
                ++step;

                if (done)
                {
                    break;
                }
            }

            std::cout << "Run " << run << " complete. Total reward: " << total_reward << " Num steps: " << step << std::endl;
            log_file << total_reward << "," << step << "\n";
        }
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Exception: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}
