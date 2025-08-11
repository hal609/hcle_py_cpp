"""Python module for HCLE."""

import warnings

# # --- Public API ---
# # Import the core C++ interface. The 'C' prefix is a convention to
# # indicate it's the raw C++ binding. This is the class you'll create with pybind11.
# # The name '_cynes_py' corresponds to the `pybind11_add_module(_cynes_py ...)` in CMake.
# # from ._cynes_py import NesInterface as CNesInterface

# # Import the user-friendly Python Gymnasium wrapper from your `env.py` file.
# from .env import HCLEnv

# __all__ = ["CNesInterface", "NesEnv"]
# __all__ = ["HCLEnv"]

try:
    from .registration import register_hcle_envs
    register_hcle_envs()
except ImportError as e:
    warnings.warn(f"Could not register HCLE environments with Gymnasium: {e}")