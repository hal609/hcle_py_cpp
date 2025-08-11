# import gymnasium as gym
# import os

# # Assuming roms are in a sibling 'roms' directory
# ROMS_DIR = os.path.join(os.path.dirname(__file__), 'roms')

# GAME_REGISTRY = {
#     "SuperMarioBros": "smb1.bin",
#     # Add other games here
# }

# def register_hcle_envs():
#     for game_name, rom_file in GAME_REGISTRY.items():
#         rom_path = os.path.join(ROMS_DIR, rom_file)
        
#         gym.register(
#             id=f"HCLE/{game_name}-v0",
#             entry_point="hcle_py.env:HCLEnv",
#             kwargs={
#                 'game': game_name,
#                 'rom_path': rom_path
#             },
#             nondeterministic=True
#         )

#         # Register the vector environment entry point
#         gym.register(
#             id=f"HCLE/{game_name}-v0-vec",
#             entry_point="hcle_py.vector_env:HCLEVectorEnv",
#             kwargs={
#                 'game': game_name, 
#                 'rom_path': rom_path
#                 },
#             nondeterministic=True
#         )

import gymnasium as gym
import os

ROMS_DIR = os.path.join(os.path.dirname(__file__), 'roms')

GAME_REGISTRY = {
    "SuperMarioBros": "smb1.bin",
}

def register_hcle_envs():
    for game_name, rom_file in GAME_REGISTRY.items():
        rom_path = os.path.join(ROMS_DIR, rom_file)
        print(rom_path)
        
        # Register a single ID for the game
        gym.register(
            id=f"HCLE/{game_name}-v0",
            # The entry point for a single environment instance
            entry_point="hcle_py.env:HCLEnv",
            # The entry point for a vectorized environment instance
            vector_entry_point="hcle_py.vector_env:HCLEVectorEnv",
            # Kwargs are passed to BOTH entry points
            kwargs={
                'game': game_name, 
                'rom_path': rom_path
            },
            nondeterministic=True
        )