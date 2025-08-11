import gymnasium as gym
import time
import matplotlib.pyplot as plt
import hcle_py # This import registers the environments
from hcle_py.vector_env import HCLEVectorEnv

def run_test():
    print("--- HCLE Test Script ---")

    env_id = "HCLE/SuperMarioBros-v0"
    num_envs = 64
    try:
        print(f"Creating env: {env_id}")
        envs = HCLEVectorEnv(game="smb1", num_envs=num_envs)
        print("Environment created successfully. ✅")
    except Exception as e:
        print(f"❌ Error creating environment: {e}")

    print("Resetting environment...")
    obs = envs.reset()

    total_reward = 0.0
    num_steps = 1000
    start_time = time.time()

    print(f"\nRunning for {num_steps} steps with random actions...")
    for i in range(num_steps):
        action = envs.action_space.sample()
        obs, reward, terminated, truncated, info = envs.step(action)

        total_reward += reward.mean()

        if (i + 1) % 20 == 0:
            print(f"Step {i+1:3d}: Reward={reward.mean(): 6.2f}, Total Reward={total_reward: 8.2f}")

        # # If the episode ends (Mario dies), reset the environment.
        # if terminated or truncated:
        #     # print(f"\nEpisode finished after {i + 1} steps. Resetting.")
        #     envs.reset()
        #     total_reward = 0.0

    end_time = time.time()
    duration = end_time - start_time
    fps = (num_steps*num_envs) / duration

    # Clean up the environment resources.
    envs.close()
    print("\n--- Test Complete ---")
    print(f"Executed {num_steps*num_envs} steps in {duration:.2f} seconds ({fps:.2f} FPS).")


if __name__ == "__main__":
    run_test()
