import gymnasium as gym
from gymnasium.spaces import Box, Discrete
import numpy as np
from . import _hcle_py # Import the C++ module built by pybind11

class HCLEnv(gym.Env):
    metadata = {"render_modes": ["human", "rgb_array"], "render_fps": 60}

    def __init__(self, game: str, rom_path: str, render_mode: str = "rgb_array", **kwargs):
        # 1. Instantiate the C++ environment
        self.hcle = _hcle_py.HCLEnvironment()
        
        # 2. Load the ROM, which also initializes the correct game logic in C++
        self.hcle.load_rom(rom_path, game, render_mode)
        
        # 3. Get the available actions from C++ to build the action space
        self._action_set = self.hcle.get_action_set()
        self.action_space = Box(low=0, high=255, shape=(len(self._action_set)), dtype=np.uint8)

        # 4. Define the observation space (dimensions are fixed for NES)
        self.observation_space = Box(low=0, high=255, shape=(240, 256, 3), dtype=np.uint8)

    def step(self, action_index: np.uint8):
        # Pass the action index to C++, get reward back
        reward = self.hcle.act(action_index)
        
        # Get other values from C++
        obs = self.hcle.get_screen_rgb()
        terminated = self.hcle.is_done()
        truncated = False
        info = {} # You can add RAM values to info if needed

        return np.copy(obs), reward, terminated, truncated, info

    def reset(self, *, seed=None, options=None):
        super().reset(seed=seed) # Important for seeding in Gym
        
        self.hcle.reset()
        obs = self.hcle.get_screen_rgb()
        info = {}
        
        return np.copy(obs), info
        
    def close(self):
        # Add any C++ cleanup here if necessary
        pass