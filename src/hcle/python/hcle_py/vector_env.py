import gymnasium as gym
from gymnasium.vector import VectorEnv
from gymnasium.spaces import Box, Discrete
from gymnasium.spaces.space import MaskNDArray, Space
import numpy as np
import time
from hcle_py import roms
from . import _hcle_py

class HCLEVectorEnv(VectorEnv):
    """Gymnasium VectorEnv wrapper for the C++ HCLEVectorEnvironment."""

    def __init__(self, game: str, num_envs: int = 2, render_mode: str = "human", fps_limit:int = -1):
        self.vec_hcle = _hcle_py.HCLEVectorEnvironment(
            num_envs=num_envs,
            rom_path=roms.get_rom_path(game),
            game_name=game,
            render_mode=render_mode
        )
        
        self.fps_limit = fps_limit

        self.single_observation_space = Box(low=0, high=255, shape=(240, 256, 3), dtype=np.uint8)
        
        action_space_size = self.vec_hcle.get_action_space_size()
        self.single_action_space = Box(low=0, high=action_space_size-1 , shape=(), dtype=np.uint8)

        self.num_envs = num_envs
        self.batch_size = num_envs
        self.observation_space = gym.vector.utils.batch_space(
            self.single_observation_space, self.batch_size
        )
        self.action_space = gym.vector.utils.batch_space(
            self.single_action_space, self.batch_size
        )

    def reset(self, *, seed=None, options=None):
        self.vec_hcle.reset()
        return #np.copy(obs), {}

    def step(self, actions):
        actions = np.asarray(actions, dtype=np.uint8)
        start_time = time.time()
        obs, rewards, dones = self.vec_hcle.step(actions)
        if self.fps_limit > 0:
            time.sleep(max(0, 1/self.fps_limit - (time.time() - start_time)))
        truncateds = np.zeros(self.num_envs, dtype=np.bool_)
        infos = {}
        return np.copy(obs), rewards, dones, truncateds, infos

    def close(self, **kwargs):
        if hasattr(self, 'vec_hcle'):
            del self.vec_hcle