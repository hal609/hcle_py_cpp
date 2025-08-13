import time
import numpy as np
from hcle_py.vector_env import HCLEVectorEnv

def simulate_agent_work(delay_ms: int):
    """This function simulates the time an agent would spend 'thinking' (e.g., a GPU forward pass)."""
    if delay_ms > 0:
        time.sleep(delay_ms / 1000.0)

def run_performance_test():
    num_envs = 64
    num_steps = 500
    agent_think_time_ms = 16  # Simulate a 16ms delay, ~60 FPS agent

    print("--- HCLE Performance Test ---")
    print(f"Envs: {num_envs}, Steps: {num_steps}, Agent Think Time: {agent_think_time_ms}ms")

    envs = HCLEVectorEnv(game="smb1", num_envs=num_envs, render_mode="rgb_array")
    
    # --- 1. Synchronous Test (Your current method) ---
    print("\nRunning SYNCHRONOUS test...")
    envs.reset()
    start_time = time.time()
    for _ in range(num_steps):
        actions = envs.action_space.sample()
        # step() waits for C++ to finish before this call returns
        obs, reward, terminated, truncated, info = envs.step(actions)
        # Agent "thinks" *after* waiting for the simulation
        simulate_agent_work(agent_think_time_ms)
    
    sync_duration = time.time() - start_time
    sync_fps = (num_steps * num_envs) / sync_duration

    # --- 2. Asynchronous Test (Pipeline method) ---
    print("Running ASYNCHRONOUS test...")
    obs, info = envs.reset()
    actions = envs.action_space.sample()
    
    # Prime the pipeline by sending the first set of actions
    envs.step_async(actions)
    
    start_time = time.time()
    for _ in range(num_steps):
        # Agent "thinks" about the observations it just received
        simulate_agent_work(agent_think_time_ms)
        
        # Now, get the results from the *previous* step (which were being simulated in the background)
        obs, reward, terminated, truncated, info = envs.step_wait()
        
        # Immediately send the actions for the *next* step to keep the pipeline full
        actions = envs.action_space.sample()
        envs.step_async(actions)
        
    async_duration = time.time() - start_time
    async_fps = (num_steps * num_envs) / async_duration

    envs.close()
    
    # --- 3. Results ---
    print("\n--- Test Complete ---")
    print(f"Synchronous: {sync_duration:.2f} seconds ({sync_fps:.2f} FPS)")
    print(f"Asynchronous: {async_duration:.2f} seconds ({async_fps:.2f} FPS)")
    print(f"\nPerformance Improvement: {async_fps / sync_fps:.2f}x")

if __name__ == "__main__":
    run_performance_test()