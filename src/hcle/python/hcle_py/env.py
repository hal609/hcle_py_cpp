import gymnasium as gym
from gymnasium import spaces 
import numpy as np
from . import _hcle_py # Import the C++ module built by pybind11

class HCLEnv(gym.Env):
    metadata = {"render_modes": ["human", "rgb_array"], "render_fps": 60}

    def __init__(self, game: str, rom_path: str, render_mode: str = "rgb_array", img_height = 240, img_width=256, frame_skip=4, maxpool=False, grayscale=False, stack_num=1):
        # 1. Instantiate the C++ environment
        self.hcle = _hcle_py.PreprocessedEnv(rom_path, game, img_height, img_width, frame_skip, maxpool, grayscale, stack_num)

        # 3. Get the available actions from C++ to build the action space
        self._action_set = self.hcle.get_action_set()
        self.action_space = spaces.Discrete(len(self._action_set))

        # 4. Define the observation space (dimensions are fixed for NES)
        self.observation_space = spaces.Box(low=0, high=255, shape=(img_height, img_width, (3 if grayscale else 1)), dtype=np.uint8)

        self.obs_buffer = np.zeros(self.observation_space.shape, dtype=self.observation_space.dtype)
    
    def step(self, action_index: int):
        self.hcle.step(action_index, self.obs_buffer)

        reward = self.hcle.get_reward()
        done = self.hcle.is_done()
        truncated = False
        info = {}

        return np.copy(self.obs_buffer), reward, done, truncated, info

    def reset(self, *, seed=None, options=None):
        super().reset(seed=seed) # Important for seeding in Gym
        
        self.hcle.reset(self.obs_buffer)
        info = {}
        return np.copy(self.obs_buffer), info
        
    def close(self):
        # Add any C++ cleanup here if necessary
        pass

    def save_to_state(self, state_num:int):
        self.hcle.save_to_state(state_num)

    def load_from_state(self, state_num:int):
        self.hcle.load_from_state(state_num)