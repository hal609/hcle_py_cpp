"""Rom module with functions for collecting individual and all ROMS files."""

from __future__ import annotations

import functools
import hashlib
import json
import os
import warnings
from pathlib import Path

__all__ = ["get_rom_path", "get_all_rom_ids"]


@functools.lru_cache(maxsize=1)
def _get_expected_bin_hashes() -> dict[str, str]:
    # this is a map of {rom.bin : md5 checksum}
    with open(Path(__file__).parent / "md5.json") as f:
        return json.load(f)
        
@functools.lru_cache(maxsize=1)
def get_all_rom_ids() -> list[str]:
    return [key.split(".")[0] for key in _get_expected_bin_hashes().keys()]

def get_rom_path(name: str) -> Path:
    """
    Expects name as a snake_case name, returns the full path of the .bin file
    if it's valid, otherwise raises an error.
    """
    current_dir = os.path.dirname(os.path.abspath(__file__))
    rom_path = os.path.join(current_dir, f'{name}.bin')
    return rom_path