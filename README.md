The Home Console Learning Environment
<a href="#home-console-learning-environment">
  <img alt="Home Console Learning Environment" align="right" width=120 src="docs/img/hcle_icon_v0.png" />
</a>
===============================


<!-- ![Static Badge](https://img.shields.io/pypi/pyversions/hcle?logo=python) -->
![Static Badge](https://img.shields.io/badge/Python-3.10%20|%203.11%20|%203.12%20|%203.13-blue?logo=python)
![PyPI - Format](https://img.shields.io/pypi/format/hcle_py)
![PyPI - Version](https://img.shields.io/pypi/v/hcle_py)
![GitHub commit activity](https://img.shields.io/github/commit-activity/t/hal609/hcle_py_cpp)
![CodeFactor Grade](https://img.shields.io/codefactor/grade/github/hal609/hcle_py_cpp)
![Code Lines](https://img.shields.io/endpoint?url=https://ghloc.vercel.app/api/Jonius7/SteamUI-OldGlory/badge?filter=.py$,.scss$,.rs$&style=flat&logoColor=white&label=Lines%20of%20Code)
<!-- ![PyPI - Wheel](https://img.shields.io/pypi/wheel/hcle_py) -->
<!-- ![PyPI - Python Version](https://img.shields.io/pypi/pyversions/hcle_py) -->



<!-- [![Python](https://img.shields.io/pypi/pyversions/hcle-py.svg)](https://badge.fury.io/py/ale-py)
[![PyPI Version](https://img.shields.io/pypi/v/ale-py)](https://pypi.org/project/ale-py) -->

**The Home Console Learning Environment (HCLE) is a framework inspired by the Arcade Learning Environment (ALE) that allows for the development of AI agents for Nintendo Entertainment System games, as ALE does for Atari 2600 games.**
It is built on top of the NES emulator [cynes](https://github.com/Youlixx/cynes) and separates the details of emulation from agent design.

HCLE currently supports 16 titles, the full list of which can be viewed [here](https://github.com/hal609/hcle_py_cpp/supported_titles.md). A full write up of this project is available [here](https://hkolb.co.uk/hcle.pdf).

Features
--------
The project aims for feature parity with ALE and as such supports the following standard ALE features:

- Object-oriented framework with support to add agents and games.
- Automatic extraction/calculation of game score and end-of-game signal.
- Multi-platform code (compiled and tested under macOS, Windows, and several Linux distributions).
- Python bindings through [pybind11](https://github.com/pybind/pybind11).
- Native support for [Gymnasium](http://github.com/farama-Foundation/gymnasium), a maintained fork of OpenAI Gym.
- C++ based vectorizer for acting in multiple ROMs at the same time.

Quick Start
===========

HCLE currently supports three different interfaces: C++, Python, and Gymnasium.

Python
------

You simply need to install the `hcle-py` package distributed via PyPI:

```shell
pip install hcle-py
```
You can now import HCLE in your Python projects as you would with ALE
```python
from hcle_py.env import HCLEEnv

env = HCLEEnv("smb1",
	render_mode="human",
	img_height=240,
	img_width=256,
	frame_skip=4,
	maxpool=False,
	grayscale=False,
	stack_num=1,
	render_fps_limit=0
)

obs, info = env.reset()

for i in range(1000):
	action = env.action_space.sample()
	obs, reward, terminated, truncated, info = env.step(action)

env.close()
```

A vectorized environment with preprocessing, written in C++, is also available and can be used like so:

```py
from hcle_py import NESVectorEnv

# Create a vector environment with 4 parallel instances of Super Mario Bros. 1
envs = NESVectorEnv(
    game="smb1",  # The ROM id not name, i.e., camel case compared to `gymnasium.make` name versions
    num_envs=4,
)
# Reset all environments
observations, info = envs.reset()

num_steps = 1000
for step in range(num_steps):
    # Take random actions in all environments
    actions = envs.action_space.sample()
    observations, rewards, terminations, truncations, infos = envs.step(actions)

# Close the environment when done
envs.close()
```

## Gymnasium

```py
import gymnasium as gym
import hcle_py

gym.register_envs(hcle_py)  # unnecessary but helpful for IDEs

env = gym.make('HCLE/TheLegendOfZelda-v0', render_mode="human")  # remove render_mode in training
obs, info = env.reset()
episode_over = False
while not episode_over:
    action = policy(obs)  # to implement - use `env.action_space.sample()` for a random policy
    obs, reward, terminated, truncated, info = env.step(action)

    episode_over = terminated or truncated

env.close()
```


C++
---

The following instructions will assume you have a valid C++20 compiler and [`vcpkg`](https://github.com/microsoft/vcpkg) installed.

To compile HCLE you can run:

```sh
mkdir build && cd build
cmake ../ -DCMAKE_BUILD_TYPE=Release
```

```cpp
#include "hcle/environment/hcle_vector_environment.hpp"

const int num_envs = 72;
const std::string rom_path = "C:\\Users\\example\\Documents\\data";
const std::string game_name = "arkanoid";
const std::string render_mode = "rgb_array";

std::vector<uint8_t> obs_buffer(num_envs * single_obs_size);
std::vector<double> reward_buffer(num_envs);
std::vector<uint8_t> done_buffer(num_envs);
std::vector<uint8_t> actions(num_envs);

hcle::environment::HCLEVectorEnvironment env(num_envs, rom_path, game_name, render_mode, 84, 84, 1, true, false, 4);

env.reset(obs_buffer.data(), reward_buffer.data(), done_buffer.data());

for (int step = 0; step < num_steps; ++step)
{
    for (int i = 0; i < num_envs; ++i)
        actions[i] = static_cast<uint8_t>(action_dist(rng));

    env.send(actions);
    env.recv(obs_buffer.data(), reward_buffer.data(), done_buffer.data());
}
```

<!-- Available Parameters
------ -->

