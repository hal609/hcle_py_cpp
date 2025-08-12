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
set(PYTHON_EXECUTABLE "C:/Users/offan/anaconda3/python.exe" CACHE PATH "Path to the Python executable")
set(pybind11_DIR "C:/Users/offan/anaconda3/Lib/site-packages/pybind11/share/cmake/pybind11" CACHE PATH "Path to pybind11's CMake directory")
    
# --- Find Deps ---
find_package(Python COMPONENTS Interpreter Development REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(pybind11 CONFIG REQUIRED)

# --- Define Source Files ---
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

# Build static library for core
add_library(hcle_core STATIC ${HCLE_CORE_SOURCES})
target_include_directories(hcle_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(hcle_core PUBLIC SDL2::SDL2 SDL2::SDL2main)

# Python module
pybind11_add_module(_hcle_py
    src/hcle/python/python_interface.cpp
    src/hcle/python/python_vector_interface.cpp)

target_include_directories(_hcle_py PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(_hcle_py PRIVATE hcle_core)

# Build small cpp testing code
add_executable(hcle_test src/apps/test_runner.cpp)
target_include_directories(hcle_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(hcle_test PRIVATE hcle_core SDL2::SDL2 SDL2::SDL2main)

# Copy SDL2.dll to output dir
add_custom_command(
    TARGET hcle_test
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:SDL2::SDL2>" 
        "$<TARGET_FILE_DIR:hcle_test>"
    COMMENT "Copying SDL2.dll to output directory"
)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})