#include <vector>
#include <atomic>
#include <mutex>
#include <memory>

namespace hcle::vector
{
    struct Timestep
    {
        int env_id;                       // ID of the environment this observation is from
        std::vector<uint8_t> observation; // Screen pixel data
        float reward;                     // Reward received in this step
        bool done;
        std::vector<uint8_t> *final_observation; // Screen pixel data for previous episode last observation with Autoresetmode == SameStep
    };

    struct Action
    {
        int env_id;
        uint8_t action_value;
        bool force_reset = false;
    };

    // struct Result
    // {
    //     int env_id;
    //     float reward;
    //     bool done;
    // };
}