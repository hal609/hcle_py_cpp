#include <iostream>
#include <vector>
#include <stdexcept>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <random>
#include <chrono>
#include <limits> // For std::numeric_limits

#include "hcle/environment/preprocessed_env.hpp"
#include "hcle/environment/hcle_vector_environment.hpp"
#include "hcle/common/display.hpp"

int main(int argc, char **argv)
{
    // =========================================================================
    //  1. CONFIGURATION
    // =========================================================================
    constexpr int SEARCH_DEPTH = 10;
    constexpr int ACTION_SPACE_SIZE = 7;
    constexpr int NUM_ENVS = 70;               // Number of parallel environments for simulation
    constexpr int NUM_SAMPLES_PER_ACTION = 10; // Number of random futures to sample for each action
    constexpr int frame_skip = 12;
    constexpr float gamma = 0.99;

    const std::string rom_path = "C:\\Users\\offan\\Documents\\hcle_py_cpp\\src\\hcle\\python\\hcle_py\\data"; // Add your ROM path here
    const std::string game_name = "zelda1";
    constexpr int num_steps = 10000;

    // --- Calculate search space and batching requirements for sampling ---
    const long long total_simulations = static_cast<long long>(ACTION_SPACE_SIZE) * NUM_SAMPLES_PER_ACTION;
    const int num_batches = (total_simulations + NUM_ENVS - 1) / NUM_ENVS; // Ceiling division

    std::cout << "Search Configuration:" << std::endl;
    std::cout << "  - Search Depth: " << SEARCH_DEPTH << std::endl;
    std::cout << "  - Action Space: " << ACTION_SPACE_SIZE << std::endl;
    std::cout << "  - Samples per Action: " << NUM_SAMPLES_PER_ACTION << std::endl;
    std::cout << "  - Total Simulations per Step: " << total_simulations << std::endl;
    std::cout << "  - Parallel Environments: " << NUM_ENVS << std::endl;
    std::cout << "  - Batches per Step: " << num_batches << std::endl;

    // =========================================================================
    //  2. SETUP
    // =========================================================================
    hcle::environment::HCLEVectorEnvironment vec_env(NUM_ENVS, rom_path, game_name, "rgb_array", 256, 240, frame_skip, false, false, 1);
    hcle::environment::PreprocessedEnv env(rom_path, game_name, 256, 240, frame_skip, false, false, 1);
    env.createWindow();

    // Set up a single random number generator for the entire program
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> action_distrib(0, ACTION_SPACE_SIZE - 1);

    const size_t single_obs_size = env.getObservationSize();
    std::vector<uint8_t> obs_buffer(NUM_ENVS * single_obs_size);
    std::vector<double> reward_buffer(NUM_ENVS);
    std::vector<double> total_reward_buffer(NUM_ENVS);
    std::vector<uint8_t> done_buffer(NUM_ENVS);
    std::vector<uint8_t> actions_this_step(NUM_ENVS);

    // Buffer to store the sum of rewards for each initial action
    std::vector<double> sum_of_rewards_for_action(ACTION_SPACE_SIZE);

    env.reset(obs_buffer.data());

    // Initial steps to get into a meaningful game state
    for (int i = 0; i < 1; ++i)
    {
        env.step(0, obs_buffer.data());
    }

    env.saveToState(0);
    double total_reward = 0.0;

    // =========================================================================
    //  3. MAIN SIMULATION LOOP
    // =========================================================================
    for (int step = 0; step < num_steps; ++step)
    {
        // Reset rewards for this planning step
        std::fill(sum_of_rewards_for_action.begin(), sum_of_rewards_for_action.end(), 0.0);

        // --- BEGIN MONTE CARLO SAMPLING ---
        for (int batch = 0; batch < num_batches; ++batch)
        {
            // using clock = std::chrono::steady_clock;
            // auto load_start = clock::now();
            vec_env.loadFromState(0);
            // auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(clock::now() - load_start).count();
            // std::cout << "Loading states took " << elapsed << " seconds" << std::endl;
            std::fill(total_reward_buffer.begin(), total_reward_buffer.end(), 0.0f);

            // Determine the fixed initial action for each simulation in this batch.
            std::vector<int> initial_actions_for_batch(NUM_ENVS);
            for (int i = 0; i < NUM_ENVS; ++i)
            {
                int sim_index = batch * NUM_ENVS + i;
                if (sim_index < total_simulations)
                {
                    initial_actions_for_batch[i] = sim_index / NUM_SAMPLES_PER_ACTION;
                }
            }

            // --- Simulate rollouts to the specified depth ---
            for (int depth = 0; depth < SEARCH_DEPTH; ++depth)
            {
                for (int i = 0; i < NUM_ENVS; ++i)
                {
                    int sim_index = batch * NUM_ENVS + i;
                    if (sim_index < total_simulations)
                    {
                        if (depth == 0)
                        {
                            actions_this_step[i] = initial_actions_for_batch[i];
                        }
                        else
                        {
                            actions_this_step[i] = action_distrib(gen);
                        }
                    }
                    else
                    {
                        actions_this_step[i] = 0; // Padding for the last batch
                    }
                }

                vec_env.send(actions_this_step);
                vec_env.recv(obs_buffer.data(), reward_buffer.data(), done_buffer.data());
                for (int i = 0; i < NUM_ENVS; ++i)
                {
                    total_reward_buffer[i] += std::pow(gamma, depth) * reward_buffer[i];
                }
            }

            // --- Accumulate the rewards for the corresponding initial actions ---
            for (size_t i = 0; i < NUM_ENVS; ++i)
            {
                int sim_index = batch * NUM_ENVS + i;
                if (sim_index < total_simulations)
                {
                    int initial_action = initial_actions_for_batch[i];
                    sum_of_rewards_for_action[initial_action] += total_reward_buffer[i];
                }
            }
        }
        // --- END OF SAMPLING ---

        // =========================================================================
        //  4. TAKE BEST ACTION BASED ON MEAN REWARD
        // =========================================================================

        // --- Calculate mean reward for each action ---
        std::vector<double> mean_rewards(ACTION_SPACE_SIZE);
        for (int i = 0; i < ACTION_SPACE_SIZE; ++i)
        {
            if (NUM_SAMPLES_PER_ACTION > 0)
            {
                // std::cout << "action " << i << " has mean reward " << sum_of_rewards_for_action[i] / NUM_SAMPLES_PER_ACTION << std::endl;
                mean_rewards[i] = sum_of_rewards_for_action[i] / NUM_SAMPLES_PER_ACTION;
            }
            else
            {
                mean_rewards[i] = 0.0;
            }
        }

        // --- Find the highest mean reward ---
        auto max_it = std::max_element(mean_rewards.begin(), mean_rewards.end());
        double max_mean_reward = (max_it != mean_rewards.end()) ? *max_it : -std::numeric_limits<double>::infinity();

        // --- Find all actions that give this best mean reward (to handle ties) ---
        std::vector<int> best_actions;
        for (int i = 0; i < ACTION_SPACE_SIZE; ++i)
        {
            if (std::abs(mean_rewards[i] - max_mean_reward) < 1e-9)
            {
                best_actions.push_back(i);
            }
        }

        // --- Select the best action, breaking ties randomly ---
        int best_first_action = 0; // Default fallback action
        if (!best_actions.empty())
        {
            if (best_actions.size() > 1)
            {
                std::uniform_int_distribution<> tie_distrib(0, best_actions.size() - 1);
                best_first_action = best_actions[tie_distrib(gen)];
            }
            else
            {
                best_first_action = best_actions[0];
            }
        }

        // --- Apply the chosen action to the main environment ---
        env.step(best_first_action, obs_buffer.data());
        double single_step_reward = env.getReward();
        total_reward += single_step_reward;
        std::cout << "Step: " << step << " | Action: " << best_first_action << " | Step Reward: " << single_step_reward << " | Total Reward: " << total_reward << std::endl;

        env.saveToState(0);

        if (env.isDone())
        {
            std::cout << "Episode finished after " << step + 1 << " steps. Final Score: " << total_reward << std::endl;
            // Optionally reset the environment here to start a new episode
            // total_reward = 0.0;
            // env.reset(obs_buffer.data());
            // env.saveToState(0);
            return 0; // Or break the loop
        }
    }

    return 0;
}