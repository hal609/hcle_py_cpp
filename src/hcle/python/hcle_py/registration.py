import gymnasium as gym
import os

DATA_DIR = os.path.join(os.path.dirname(__file__), "data")

GAME_REGISTRY = {
    "Arkanoid": "arkanoid",
    "Baseball": "baseball",
    "BubbleBobble": "bubblebobble",
    "DrMario": "drmario",
    "Excitebike": "excitebike",
    "Golf": "golf",
    "KungFu": "kungfu",
    "AdventuresOfLolo1": "lolo1",
    "MarioBros": "mariobros",
    "MikeTysonsPunchOut": "mtpo",
    "SuperMarioBros1": "smb1",
    "SuperMarioBros2": "smb2",
    "SuperMarioBros3": "smb3",
    "Tetris": "tetris",
    "TeenageMutantNinjaTurtles": "tmnt",
    "TheLegendOfZelda": "zelda1",
}


def register_hcle_envs():
    for game_name, data_file in GAME_REGISTRY.items():
        # Register a single ID for the game
        gym.register(
            id=f"HCLE/{game_name}-v0",
            # The entry point for a single environment instance
            entry_point="hcle_py.env:HCLEEnv",
            # The entry point for a vectorized environment instance
            vector_entry_point="hcle_py.vector_env:NESVectorEnv",
            # Kwargs are passed to BOTH entry points
            kwargs={"game": data_file, "data_root_dir": DATA_DIR},
            nondeterministic=True,
        )
