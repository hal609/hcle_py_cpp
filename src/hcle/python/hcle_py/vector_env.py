from typing import Any, TypeVar
import gymnasium as gym
from gymnasium.vector import VectorEnv
from gymnasium.spaces import Box, Discrete
from gymnasium.spaces.space import MaskNDArray, Space
import numpy as np
import time
from hcle_py import roms
from . import _hcle_py

ObsType = TypeVar("ObsType")

class HCLEVectorEnv(VectorEnv):
    """Gymnasium VectorEnv wrapper for the C++ HCLEVectorEnvironment."""

    def __init__(
            self, 
            game: str, 
            num_envs: int = 2, 
            render_mode: str = "rgb_array", 
            obs_height: int = 84,
            obs_width: int = 84,
            frame_skip: int = 4,
            maxpool: bool = True,
            grayscale: bool = True,
            stack_num: int = 4,
            fps_limit:int = -1):
        
        self.vec_hcle = _hcle_py.HCLEVectorEnvironment(
            num_envs=num_envs,
            rom_path=roms.get_rom_path(game),
            game_name=game,
            render_mode=render_mode,
            obs_height=obs_height,
            obs_width=obs_width,
            frame_skip=frame_skip,
            maxpool=maxpool,
            grayscale=grayscale,
            stack_num=stack_num
        )
        
        self.fps_limit = fps_limit

        self.single_observation_space = Box(low=0, high=255, shape=(240, 256, 3), dtype=np.uint8)
        
        action_space_size = self.vec_hcle.get_action_space_size()
        self.single_action_space = Discrete(action_space_size)

        self.num_envs = num_envs
        self.batch_size = num_envs
        self.observation_space = gym.vector.utils.batch_space(
            self.single_observation_space, self.batch_size
        )
        self.action_space = gym.vector.utils.batch_space(
            self.single_action_space, self.batch_size
        )

    def reset(self, *, seed=None, options=None)  -> tuple[ObsType, dict[str, Any]]:
        return self.vec_hcle.reset()
    
    def step_async(self, actions: np.ndarray):
        """Asynchronously sends actions to the environments."""
        self.vec_hcle.step_async(actions)

    def step_wait(self) -> tuple[np.ndarray, np.ndarray, np.ndarray, np.ndarray, dict[str, Any]]:
        """Waits for and receives the results from the environments."""
        obs, rewards, dones = self.vec_hcle.step_wait()
        truncateds = np.zeros(self.num_envs, dtype=np.bool_)
        infos = {}
        return obs, rewards, dones, truncateds, infos

    def step(self, actions: np.ndarray):
        self.step_async(actions)
        return self.step_wait()

    def close(self, **kwargs):
        if hasattr(self, 'vec_hcle'):
            del self.vec_hcle