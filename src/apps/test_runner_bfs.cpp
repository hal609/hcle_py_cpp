#include <iostream>
#include <vector>
#include <stdexcept>
#include <numeric>   // For std::iota
#include <algorithm> // For std::max_element
#include <cmath>     // For std::pow (only used once for initialization)
#include <fstream>
#include <random>

#include "hcle/environment/preprocessed_env.hpp"
#include "hcle/environment/hcle_vector_environment.hpp"
#include "hcle/common/display.hpp"

int getActionForPath(long long path_id, int depth, int action_space_size, const std::vector<long long> &powers_lookup)
{
    // The exponent is SEARCH_DEPTH - 1 - depth. The lookup table is indexed by depth for simplicity.
    return (path_id / powers_lookup[depth]) % action_space_size;
}

int main(int argc, char **argv)
{
    // =========================================================================
    //  1. CONFIGURATION
    // =========================================================================
    constexpr int SEARCH_DEPTH = 3;
    constexpr int ACTION_SPACE_SIZE = 5;
    int max_envs = pow(ACTION_SPACE_SIZE, SEARCH_DEPTH);
    int env_counter = max_envs;
    int divisor = 1;
    while (env_counter > 300)
    {
        divisor++;
        env_counter = max_envs / divisor;
    }
    int NUM_ENVS = max_envs / divisor;
    constexpr int frame_skip = 15;
    constexpr float gamma = 0.99;

    const std::string rom_path = "";
    const std::string game_name = "tetris";
    constexpr int num_steps = 1000;

    // --- Calculate search space and batching requirements ---
    const long long total_paths = static_cast<long long>(std::pow(ACTION_SPACE_SIZE, SEARCH_DEPTH));
    const int num_batches = (total_paths + NUM_ENVS - 1) / NUM_ENVS; // Ceiling division

    // OPTIMIZATION: Pre-calculate powers to avoid std::pow in the loop
    std::vector<long long> powers_lookup(SEARCH_DEPTH);
    for (int d = 0; d < SEARCH_DEPTH; ++d)
    {
        powers_lookup[d] = static_cast<long long>(std::pow(ACTION_SPACE_SIZE, SEARCH_DEPTH - 1 - d));
    }
    const long long first_action_divisor = powers_lookup[0]; // Divisor for the first action (depth 0)

    std::cout << "Search Configuration:" << std::endl;
    std::cout << "  - Search Depth: " << SEARCH_DEPTH << std::endl;
    std::cout << "  - Action Space: " << ACTION_SPACE_SIZE << std::endl;
    std::cout << "  - Total Paths to Search: " << total_paths << std::endl;
    std::cout << "  - Parallel Environments: " << NUM_ENVS << std::endl;
    std::cout << "  - Batches per Step: " << num_batches << std::endl;

    // =========================================================================
    //  2. SETUP
    // =========================================================================
    hcle::environment::HCLEVectorEnvironment vec_env(NUM_ENVS, rom_path, game_name, "rgb_array", 256, 240, frame_skip, false, false, 1);
    hcle::environment::PreprocessedEnv env(rom_path, game_name, 256, 240, frame_skip, false, false, 1);
    env.createWindow();

    std::mt19937 rng(std::random_device{}());

    const size_t single_obs_size = env.getObservationSize();
    std::vector<uint8_t> obs_buffer(NUM_ENVS * single_obs_size);
    std::vector<double> reward_buffer(NUM_ENVS);
    std::vector<double> total_reward_buffer(NUM_ENVS, 0.0f);
    std::vector<uint8_t> done_buffer(NUM_ENVS);
    std::vector<int> actions_this_step(NUM_ENVS);
    std::vector<double> all_path_rewards(total_paths);
    env.reset(obs_buffer.data());
    for (int i = 0; i < (3); ++i)
    {
        env.step(0, obs_buffer.data());
    }
    env.saveToState(0);
    double total_reward = 0.0;

    // --- Open log file and write header ---
    std::string filename = std::format("bfs_logs/{}_skip{}_gamma{}_depth{}_nactions{}.csv", game_name, frame_skip, gamma, SEARCH_DEPTH, ACTION_SPACE_SIZE);
    std::ofstream log_file(filename);
    if (!log_file.is_open())
    {
        std::cerr << "Error: Unable to open log_file.csv for writing." << std::endl;
        return 1; // Exit if the file can't be opened
    }
    log_file << "step,action,cumulative_reward\n";

    // =========================================================================
    //  3. MAIN SIMULATION LOOP
    // =========================================================================
    for (int step = 0; step < num_steps; ++step)
    {
        // --- BEGIN BREADTH-FIRST SEARCH ---
        for (int batch = 0; batch < num_batches; ++batch)
        {
            vec_env.loadFromState(0);
            std::fill(total_reward_buffer.begin(), total_reward_buffer.end(), 0.0f);

            for (int depth = 0; depth < SEARCH_DEPTH; ++depth)
            {
                for (int i = 0; i < NUM_ENVS; ++i)
                {
                    long long path_id = static_cast<long long>(batch) * NUM_ENVS + i;
                    if (path_id < total_paths)
                    {
                        actions_this_step[i] = getActionForPath(path_id, depth, ACTION_SPACE_SIZE, powers_lookup);
                    }
                    else
                    {
                        actions_this_step[i] = 0; // Padding
                    }
                }

                vec_env.send(actions_this_step);
                vec_env.recv(obs_buffer.data(), reward_buffer.data(), done_buffer.data());
                for (int i = 0; i < NUM_ENVS; ++i)
                {
                    // total_reward_buffer[i] += reward_buffer[i];
                    total_reward_buffer[i] += std::pow(gamma, depth) * reward_buffer[i];
                }
            }

            for (size_t i = 0; i < NUM_ENVS; ++i)
            {
                long long path_id = static_cast<long long>(batch) * NUM_ENVS + i;
                if (path_id < total_paths)
                {
                    all_path_rewards[path_id] = total_reward_buffer[i];
                }
            }
        }
        // --- END OF SEARCH ---

        // double max_reward = -std::numeric_limits<double>::infinity();

        // =========================================================================
        //  4. TAKE BEST ACTION
        // =========================================================================

        auto best_reward = std::max_element(all_path_rewards.begin(), all_path_rewards.end());

        // --- Find all paths which give the best reward ---
        std::vector<long long> best_path_indices;
        for (size_t i = 0; i < all_path_rewards.size(); ++i)
        {
            if (all_path_rewards[i] == *best_reward)
            {
                best_path_indices.push_back(i);
            }
        }

        long long best_path_id = 0;
        if (best_path_indices.size() > 1)
        {
            std::random_device rd;                                                    // Obtain a random number from hardware
            std::mt19937 gen(rd());                                                   // Seed the generator
            std::uniform_int_distribution<> distrib(0, best_path_indices.size() - 1); // Define the range

            // Pick a random index from our list of ties
            best_path_id = best_path_indices[distrib(gen)];
        }
        else
        {
            // If there's only one best path, just take it
            best_path_id = best_path_indices[0];
        }

        // best_path_id = std::distance(all_path_rewards.begin(), best_reward);

        double sequence_reward = 0;
        std::cout << "DEBUG: Best path ID is " << best_path_id << " with total reward " << *best_reward << std::endl;
        std::cout << "DEBUG: Full action sequence: ";
        for (int d = 0; d < SEARCH_DEPTH; ++d)
        {
            int action_step = getActionForPath(best_path_id, d, ACTION_SPACE_SIZE, powers_lookup);
            // env.step(action_step, obs_buffer.data());
            std::cout << action_step << " ";
            // sequence_reward += env.getReward();
        }
        std::cout << "Step " << step << ": Best " << SEARCH_DEPTH << "-step reward was " << *best_reward << std::endl;

        int best_first_action = (best_path_id / first_action_divisor) % ACTION_SPACE_SIZE;
        best_first_action = getActionForPath(best_path_id, 0, ACTION_SPACE_SIZE, powers_lookup);
        std::cout << "Step " << step << ": Best " << SEARCH_DEPTH << "-step reward was " << *best_reward
                  << ". Taking action: " << best_first_action << ". Total reward so far: " << total_reward << std::endl;

        env.step(best_first_action, obs_buffer.data());
        double single_step_reward = env.getReward();
        total_reward += single_step_reward;
        std::cout << " Step reward: " << single_step_reward << std::endl;
        log_file << step << "," << best_first_action << "," << total_reward << "\n";
        env.saveToState(0);
        if (env.isDone())
        {
            return 0;
        }
    }

    return 0;
}