#pragma once

#include <vector>
#include <memory>
#include <string>

#include "hcle/environment/async_vectorizer.hpp"
// #include "hcle/environment/utils.hpp"

namespace hcle::environment
{

    // The user-facing class with the desired API
    class HCLEVectorEnvironment
    {
    public:
        HCLEVectorEnvironment(
            const int num_envs,
            const std::string &rom_path,
            const std::string &game_name,
            const std::string &render_mode,
            const uint8_t obs_height = 84,
            const uint8_t obs_width = 84,
            const uint8_t frame_skip = 4,
            const bool maxpool = true,
            const bool grayscale = true,
            const uint8_t stack_num = 4);

        void reset();
        void send(const std::vector<int> &action_ids);
        std::vector<hcle::vector::Timestep> recv();
        std::vector<uint8_t> getActionSet() const;


    private:
        std::unique_ptr<AsyncVectorizer> vectorizer_;
        std::vector<uint8_t> action_set_;
    };
}