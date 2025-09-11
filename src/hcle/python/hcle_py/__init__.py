"""Python module for HCLE."""

import re
import difflib
import warnings

import gymnasium as gym
from gymnasium.error import Error as GymError

from .env import HCLEEnv
from .vector_env import NESVectorEnv

__all__ = ["HCLEEnv", "NESVectorEnv"]
_original_make = gym.make

try:
    from .registration import register_hcle_envs, GAME_REGISTRY

    register_hcle_envs()
except ImportError as e:
    warnings.warn(f"Could not register HCLE environments with Gymnasium: {e}")


def _custom_make(id, **kwargs):
    """
    A wrapper for gym.make that provides helpful error messages for
    the HCLE namespace.
    """
    try:
        return _original_make(id, **kwargs)
    except GymError as e:
        if id.startswith("HCLE/"):
            # Extract game name from env ID
            match = re.search(r"HCLE/(.+?)-v\d+", id)
            if match:
                unsupported_game = match.group(1)

                suggestions = difflib.get_close_matches(
                    unsupported_game, GAME_REGISTRY.keys(), n=1, cutoff=0.6
                )

                error_message = (
                    f"'{unsupported_game}' is not a currently supported HCLE game."
                )
                if suggestions:
                    error_message += f"\nPerhaps you meant '{suggestions[0]}'?"

                error_message += (
                    "\nA full list of supported titles is available at: "
                    "https://github.com/hal609/hcle_py_cpp/supported_games.md"
                )

                raise GymError(error_message) from e

        raise e


# Apply the patch by replacing the original function with our wrapper
gym.make = _custom_make
