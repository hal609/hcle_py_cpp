#include "hcle/environment/hcle_vector_environment.hpp"
#include "hcle/environment/preprocessed_env.hpp" // Needed for the factory

namespace hcle::environment
{

    // The constructor's job is to create the factory and then the vectorizer.
    HCLEVectorEnvironment::HCLEVectorEnvironment(
        const int num_envs,
        const std::string &rom_path,
        const std::string &game_name,
        const std::string &render_mode,
        const uint8_t obs_height,
        const uint8_t obs_width,
        const uint8_t frame_skip,
        const bool maxpool,
        const bool grayscale,
        const uint8_t stack_num)
    {
        // 1. Create the factory lambda that knows how to build a PreprocessedEnv.
        auto env_factory = [=](int env_id)
        {
            std::string current_render_mode = (env_id == 0) ? render_mode : "rgb_array";
            return std::make_unique<PreprocessedEnv>(
                rom_path, game_name, current_render_mode, obs_height, obs_width,
                frame_skip, maxpool, grayscale, stack_num);
        };

        // 2. Create the vectorizer engine, passing it the factory.
        vectorizer_ = std::make_unique<AsyncVectorizer>(num_envs, env_factory);

        // const auto temp_env = env_factory(0);
        action_set_ = std::vector<uint8_t>({0}); // temp_env->getActionSet()
    }

    void HCLEVectorEnvironment::reset()
    {
        vectorizer_->reset();
        printf("HCLEVectorEnvironment finished calling vectorizer_->reset().\n");
    }

    void HCLEVectorEnvironment::send(const std::vector<int> &action_ids)
    {
        vectorizer_->send(action_ids);
    }

    std::vector<hcle::vector::Timestep> HCLEVectorEnvironment::recv()
    {
        return vectorizer_->recv();
    }

    std::vector<uint8_t> HCLEVectorEnvironment::getActionSet() const
    {
        return vectorizer_->getActionSet();;
    }
}