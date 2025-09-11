cmake_minimum_required(VERSION 3.15)

project(hcle CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

if(WIN32)
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Oi /Ot /Ob2 /fp:fast /arch:AVX2 /W4")
    set(CMAKE_CXX_FLAGS_DEBUG "/W4 /diagnostics:caret")

    # Add the /PROFILE flag ONLY for RelWithDebInfo builds
    set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} /PROFILE")
    set(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO} /PROFILE")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -ffast-math")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra -Wpedantic -fsanitize=address")
endif()

find_package(Python COMPONENTS Interpreter Development)
find_package(SDL2 CONFIG REQUIRED)
find_package(pybind11 CONFIG REQUIRED)
find_package(OpenCV REQUIRED COMPONENTS core imgproc)
find_package(ZLIB REQUIRED)


set(HCLE_CORE_SOURCES
src/hcle/emucore/nes.cpp
src/hcle/emucore/cpu.cpp
src/hcle/emucore/ppu.cpp
src/hcle/emucore/apu.cpp
src/hcle/emucore/mapper.cpp
src/hcle/environment/preprocessed_env.cpp
src/hcle/environment/hcle_environment.cpp
src/hcle/common/display.cpp
)

# CORE FILES
add_library(hcle_core STATIC ${HCLE_CORE_SOURCES})
target_include_directories(hcle_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(hcle_core
    PUBLIC
        SDL2::SDL2
        ${OpenCV_LIBS} 
        ZLIB::ZLIB
)

# PYTHON MODULE
pybind11_add_module(_hcle_py
    src/hcle/python/python_interface.cpp
    src/hcle/python/python_vector_interface.cpp
)

target_link_libraries(_hcle_py PRIVATE hcle_core)

set_target_properties(_hcle_py PROPERTIES DEBUG_POSTFIX "_d")

install(TARGETS _hcle_py
        LIBRARY DESTINATION hcle_py)

install(RUNTIME_DEPENDENCY_SET hcle_py_deps
        POST_EXCLUDE_FILES
            "python*.dll"
        LIBRARY DESTINATION hcle_py
        RUNTIME DESTINATION hcle_py
)

install(FILES $<TARGET_RUNTIME_DLLS:_hcle_py> DESTINATION hcle_py) 

# TEST CPP RUNNER
# add_executable(hcle_test src/apps/test_runner.cpp)
# target_link_libraries(hcle_test PRIVATE hcle_core)