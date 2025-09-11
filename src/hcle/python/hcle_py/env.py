import gymnasium as gym
from gymnasium import spaces
import numpy as np
import os

from . import _hcle_py


class HCLEEnv(gym.Env):
    metadata = {"render_modes": ["human", "rgb_array"], "render_fps": 60}

    def __init__(
        self,
        game: str,
        data_root_dir: str = os.path.join(os.path.dirname(__file__), "data"),
        render_mode: str = "rgb_array",
        img_height=240,
        img_width=256,
        frame_skip=4,
        maxpool=False,
        grayscale=False,
        stack_num=1,
        render_fps_limit=0,
    ):
        self.hcle = _hcle_py.PreprocessedEnv(
            data_root_dir,
            game,
            img_height,
            img_width,
            frame_skip,
            maxpool,
            grayscale,
            stack_num,
        )

        self._action_set = self.hcle.get_action_set()
        self.action_space = spaces.Discrete(len(self._action_set))

        single_obs_shape = (
            (stack_num, img_height, img_width)
            if grayscale
            else (stack_num, img_height, img_width, (1 if grayscale else 3))
        )
        self.observation_space = spaces.Box(
            low=0, high=255, shape=single_obs_shape, dtype=np.uint8
        )

        self.obs_buffer = np.zeros(
            self.observation_space.shape, dtype=self.observation_space.dtype
        )

        self.render_mode = render_mode
        if self.render_mode == "human":
            self.hcle.create_window(render_fps_limit)

    def step(self, action_index: int):
        self.hcle.step(action_index, self.obs_buffer)
        reward = self.hcle.get_reward()
        done = self.hcle.is_done()
        if done:
            self.hcle.reset(self.obs_buffer)
        truncated = False
        info = {}

        return self.obs_buffer, reward, done, truncated, info

    def reset(self, *, seed=None, options=None):
        super().reset(seed=seed)  # Needed for seeding in Gym

        self.hcle.reset(self.obs_buffer)
        info = {}
        return self.obs_buffer, info

    def save_to_state(self, state_num: int):
        self.hcle.save_to_state(state_num)

    def load_from_state(self, state_num: int):
        self.hcle.load_from_state(state_num)
