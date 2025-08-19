#include <iostream>
#include <vector>
#include <stdexcept>
#include <numeric>   // For std::iota
#include <algorithm> // For std::max_element
#include <cmath>     // For std::pow (only used once for initialization)

#include "hcle/environment/preprocessed_env.hpp"
#include "hcle/environment/hcle_vector_environment.hpp"
#include "hcle/common/display.hpp"

// Helper function to render the environment screen
void render(const uint8_t *frame_ptr, std::unique_ptr<hcle::common::Display> &display)
{
    display->update(frame_ptr);
    if (display->processEvents())
    {
        throw hcle::common::WindowClosedException();
    }
}

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
    constexpr int SEARCH_DEPTH = 7;
    constexpr int NUM_ENVS = 64;
    constexpr int ACTION_SPACE_SIZE = 2;

    const std::string rom_path = "C:\\Users\\offan\\Downloads\\hcle_py_cpp\\src\\hcle\\python\\hcle_py\\roms\\smb1.bin";
    const std::string game_name = "smb1";
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
    constexpr int frame_skip = 4;
    hcle::environment::HCLEVectorEnvironment vec_env(NUM_ENVS, rom_path, game_name, "rgb_array", 256, 240, frame_skip, true, true, 1);
    hcle::environment::PreprocessedEnv env(rom_path, game_name, 256, 240, frame_skip, false, false, 1);

    const uint8_t *frame_pointer = env.getFramePointer();
    std::unique_ptr<hcle::common::Display> display = std::make_unique<hcle::common::Display>("HCLEnvironment", 256, 240, 3);
    const size_t single_obs_size = env.getObservationSize();
    std::vector<uint8_t> obs_buffer(NUM_ENVS * single_obs_size);
    std::vector<float> reward_buffer(NUM_ENVS);
    std::vector<float> total_reward_buffer(NUM_ENVS, 0.0f);
    std::vector<uint8_t> done_buffer(NUM_ENVS);
    std::vector<int> actions_this_step(NUM_ENVS);
    std::vector<float> all_path_rewards(total_paths);
    env.reset(obs_buffer.data());
    for (int i = 0; i < 32; ++i)
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
                    total_reward_buffer[i] += reward_buffer[i];
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

        auto max_it = std::max_element(all_path_rewards.begin(), all_path_rewards.end());
        long long best_path_id = std::distance(all_path_rewards.begin(), max_it);

        int best_first_action = (best_path_id / first_action_divisor) % ACTION_SPACE_SIZE;

        std::cout << "Step " << step << ": Best " << SEARCH_DEPTH << "-step reward was " << *max_it
                  << ". Taking action: " << best_first_action << std::endl;

        env.step(best_first_action, obs_buffer.data());
        render(frame_pointer, display);
        total_reward += env.getReward();
        env.saveToState(0);
    }

    return 0;
}