from typing import Any, TypeVar
import gymnasium as gym
from gymnasium.vector import VectorEnv
from gymnasium.spaces import Box, Discrete
import numpy as np
import math
from hcle_py import roms
from . import _hcle_py

ObsType = TypeVar("ObsType")

class HCLEVectorEnv(VectorEnv):
    """
    Gymnasium VectorEnv wrapper for the C++ HCLEVectorEnvironment.

    This wrapper manages the lifecycle of the observation, reward, and done
    buffers, passing them to the C++ backend to be filled in-place,
    which avoids memory allocation overhead in the main loop.
    """

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
    ):
        # Initialize the C++ vectorized environment
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
            stack_num=stack_num,
        )

        # --- Define observation and action spaces based on C++ env properties ---
        channels = 1 if grayscale else 3
        
        # The shape of a single environment's observation
        single_obs_shape = (stack_num, obs_height, obs_width) if grayscale else (stack_num, obs_height, obs_width, channels)
        
        self.single_observation_space = Box(
            low=0, high=255, shape=single_obs_shape, dtype=np.uint8
        )
        
        action_space_size = len(self.vec_hcle.getActionSet())
        self.single_action_space = Discrete(action_space_size)

        self.num_envs = num_envs
        self.batch_size = num_envs
        self.observation_space = gym.vector.utils.batch_space(self.single_observation_space, self.batch_size)
        self.action_space = gym.vector.utils.batch_space(self.single_action_space, self.batch_size) 

        # --- Pre-allocate shared memory buffers ---
        # These NumPy arrays will be passed to C++ to be filled directly.
        self.obs_buffer = np.zeros(self.observation_space.shape, dtype=self.observation_space.dtype)
        self.rewards_buffer = np.zeros(self.num_envs, dtype=np.float32)
        # Use uint8 for dones to match C++ bool size and avoid vector<bool> issues
        self.dones_buffer = np.zeros(self.num_envs, dtype=np.uint8)

    def reset(self, *, seed: int | None = None, options: dict[str, Any] | None = None) -> tuple[ObsType, dict[str, Any]]:
        """Resets all environments and returns the initial observations."""

        self.vec_hcle.reset(self.obs_buffer)

        # Return a copy of the observation buffer to prevent users from
        # accidentally modifying the internal state.
        return np.copy(self.obs_buffer), {}
    
    
    def step_async(self, actions: np.ndarray):
        """
        Sends actions to the environments without waiting for the results.
        """
        actions = np.asarray(actions, dtype=np.uint8)
        self.vec_hcle.send(actions)

    def step_wait(self) -> tuple[ObsType, np.ndarray, np.ndarray, np.ndarray, dict[str, Any]]:
        """
        Waits for the asynchronous step to complete and returns the results.
        """
        self.vec_hcle.recv(self.obs_buffer, self.rewards_buffer, self.dones_buffer)
        
        dones_bool = self.dones_buffer.astype(np.bool_)
        truncateds = np.zeros(self.num_envs, dtype=np.bool_)
        infos = {}

        return np.copy(self.obs_buffer), self.rewards_buffer, dones_bool, truncateds, infos


    def step(self, actions: np.ndarray) -> tuple[ObsType, np.ndarray, np.ndarray, np.ndarray, dict[str, Any]]:
        """
        Convenience method that performs a full synchronous step.
        """
        self.step_async(actions)
        return self.step_wait()

    def close(self, **kwargs):
        """Cleans up the C++ environment."""
        if hasattr(self, 'vec_hcle'):
            # The C++ destructor will handle shutting down threads.
            del self.vec_hcle
