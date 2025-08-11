import os
import re
import subprocess
import sys
from pathlib import Path

import pybind11
from setuptools import Extension, setup, find_packages
from setuptools.command.build_ext import build_ext


class CMakeExtension(Extension):
    def __init__(self, name: str, sourcedir: str = "") -> None:
        super().__init__(name, sources=[])
        self.sourcedir = os.fspath(Path(sourcedir).resolve())


class CMakeBuild(build_ext):
    PLAT_TO_CMAKE = {
        "win32": "Win32",
        "win-amd64": "x64",
        "win-arm32": "ARM",
        "win-arm64": "ARM64",
    }

    def build_extension(self, ext: CMakeExtension) -> None:
        ext_fullpath = Path.cwd() / self.get_ext_fullpath(ext.name)
        extdir = ext_fullpath.parent.resolve()
        debug = int(os.environ.get("DEBUG", 0)) if self.debug is None else self.debug
        cfg = "Debug" if debug else "Release"

        cmake_generator = os.environ.get("CMAKE_GENERATOR", "")
        
        # --- THIS IS THE KEY MODIFICATION ---
        # We get the pybind11 CMake directory and pass it to the command
        cmake_args = [
            f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}{os.sep}",
            f"-DPYTHON_EXECUTABLE={sys.executable}",
            f"-Dpybind11_DIR={pybind11.get_cmake_dir()}", # Add this line
            f"-DCMAKE_BUILD_TYPE={cfg}",
        ]
        # ------------------------------------
        
        build_args = []

        if "CMAKE_ARGS" in os.environ:
            cmake_args += [item for item in os.environ["CMAKE_ARGS"].split(" ") if item]

        if self.compiler.compiler_type == "msvc":
            single_config = any(x in cmake_generator for x in {"NMake", "Ninja"})
            contains_arch = any(x in cmake_generator for x in {"ARM", "Win64"})

            if not single_config and not contains_arch:
                cmake_args += ["-A", self.PLAT_TO_CMAKE[self.plat_name]]

            if not single_config:
                cmake_args += [
                    f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{cfg.upper()}={extdir}"
                ]
                build_args += ["--config", cfg]

        if "CMAKE_BUILD_PARALLEL_LEVEL" not in os.environ:
            if hasattr(self, "parallel") and self.parallel:
                build_args += [f"-j{self.parallel}"]

        build_temp = Path(self.build_temp) / ext.name
        if not build_temp.exists():
            build_temp.mkdir(parents=True)

        subprocess.run(
            ["cmake", ext.sourcedir, *cmake_args], cwd=build_temp, check=True
        )
        subprocess.run(
            ["cmake", "--build", ".", *build_args], cwd=build_temp, check=True
        )
# Replace the entire 'if __name__ == "__main__":' block in your setup.py

if __name__ == "__main__":
    setup(
        name="hcle_py",
        version="0.1.0",
        author="Hal Kolb",
        author_email="hal@kolb.co.uk",
        description="Gymnasium environment for the NES",
        license="GPL-3.0",
        license_file="LICENSE", # Use the modern argument

        # --- This is the fix ---
        # Tells setuptools that the root of our Python packages is in this directory:
        package_dir={"": "src/hcle/python"},
        # Automatically find all packages (like 'hcle_py') inside that directory
        packages=find_packages(where="src/hcle/python"),
        # ------------------------

        # install_requires=["numpy", "gymnasium"], # Added gymnasium

        # This name must match the package structure and the module name in CMakeLists.txt
        ext_modules=[CMakeExtension("hcle_py._hcle_py")],

        cmdclass={"build_ext": CMakeBuild},
        zip_safe=False,
        python_requires=">=3.8",
        classifiers=[
            "Development Status :: 3 - Alpha",
            "Programming Language :: C++",
            "Programming Language :: Python :: 3.8",
            "Programming Language :: Python :: 3.9",
            "Programming Language :: Python :: 3.10",
            "Programming Language :: Python :: 3.11",
            "Programming Language :: Python :: 3.12",
        ],
    )