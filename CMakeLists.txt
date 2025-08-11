# --- Project Definition ---
cmake_minimum_required(VERSION 3.12)
project(hcle CXX)

# --- Compiler Settings ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(MSVC)
    add_compile_options(/w)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_PREFIX_PATH "C:/dev/SDL2-2.30.3" CACHE PATH "Root of SDL2 library")

# --- Find Dependencies ---
find_package(SDL2 CONFIG REQUIRED)

## MODIFICATION 1: Find Python and pybind11 ##
# This is the modern way to find pybind11, assuming it was installed via pip
find_package(pybind11 CONFIG REQUIRED)
# If the above line fails, you might need to add pybind11 as a subdirectory.
# Example: add_subdirectory(extern/pybind11)

# --- Define Shared Source Files ---
## MODIFICATION 2: Define sources in a variable for reuse ##
# It's good practice to list all shared C++ source files in one place.
# These files contain the core logic for both the player and the Python module.
set(HCLE_CORE_SOURCES
    src/hcle/emucore/nes.cpp
    src/hcle/emucore/cpu.cpp
    src/hcle/emucore/ppu.cpp
    src/hcle/emucore/apu.cpp
    src/hcle/emucore/mapper.cpp
    src/hcle/environment/hcle_environment.cpp
    src/hcle/common/display.cpp
    src/hcle/environment/game_logic.cpp
    src/hcle/common/thread_pool.cpp
    src/hcle/environment/hcle_vector_environment.cpp
    src/hcle/games/smb1.cpp
)

# # --- Define the Player Executable (Optional) ---
# # You can keep this if you still want to build the standalone player.
# add_executable(hcle_player
#     src/apps/player.cpp
#     ${HCLE_CORE_SOURCES} ## Add the core sources directly
# )

# target_include_directories(hcle_player PRIVATE
#     ${CMAKE_CURRENT_SOURCE_DIR}/src
# )

# target_link_libraries(hcle_player PRIVATE
#     SDL2::SDL2
#     SDL2::SDL2main
# )

# --- Define the Python Interface ---
## MODIFICATION 3: Define the Python module ##
# --- Define the Python Interface ---
pybind11_add_module(_hcle_py
    # This module is built from the interface file AND all the core logic files.
    src/hcle/python/python_interface.cpp
    src/hcle/python/python_vector_interface.cpp
    ${HCLE_CORE_SOURCES}
)

## MODIFICATION: Add this block ##
# The Python module also needs to know where to find headers, just like hcle_player.
target_include_directories(_hcle_py PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# The Python module also needs to link against its dependencies.
target_link_libraries(_hcle_py PRIVATE
    SDL2::SDL2
    SDL2::SDL2main
)

# --- Output Configuration ---
# Sets the output directory for the final executables and modules.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})