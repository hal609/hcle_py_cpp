cmake_minimum_required(VERSION 3.19)
project(hcle CXX)

cmake_policy(SET CMP0148 NEW)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Python COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(OpenCV CONFIG REQUIRED COMPONENTS core imgproc)
find_package(ZLIB REQUIRED)

set(HCLE_SOURCES
    src/hcle/emucore/nes.cpp
    src/hcle/emucore/cpu.cpp
    src/hcle/emucore/ppu.cpp
    src/hcle/emucore/apu.cpp
    src/hcle/emucore/mapper.cpp
    src/hcle/environment/preprocessed_env.cpp
    src/hcle/environment/hcle_environment.cpp
    src/hcle/common/display.cpp
)

pybind11_add_module(_hcle_py
    ${HCLE_SOURCES}
    src/hcle/python/python_interface.cpp
    src/hcle/python/python_vector_interface.cpp
)

target_link_libraries(_hcle_py PRIVATE
    pybind11::module
    # ${SDL2_LIBS}
    SDL2::SDL2
    SDL2::SDL2main
    ${OpenCV_LIBS}
    ZLIB::ZLIB
)

target_include_directories(_hcle_py PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# INSTALL
install(TARGETS _hcle_py
        LIBRARY DESTINATION hcle_py)
        
install(FILES $<TARGET_RUNTIME_DLLS:_hcle_py>
        DESTINATION hcle_py
        COMPONENT RuntimeLibraries)