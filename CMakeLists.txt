cmake_minimum_required(VERSION 3.15)
set(CMAKE_TOOLCHAIN_FILE C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake)
project(hcle CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2")
    set(CMAKE_CXX_FLAGS_DEBUG "/W4 /diagnostics:caret")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "-Ofast")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

find_package(Python COMPONENTS Interpreter Development)
find_package(pybind11 CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(pybind11 CONFIG REQUIRED)
find_package(OpenCV REQUIRED COMPONENTS core imgproc)
find_package(zlib REQUIRED)


set(HCLE_CORE_SOURCES
    src/hcle/emucore/nes.cpp
    src/hcle/emucore/cpu.cpp
    src/hcle/emucore/ppu.cpp
    src/hcle/emucore/apu.cpp
    src/hcle/emucore/mapper.cpp
    src/hcle/environment/preprocessed_env.cpp
    src/hcle/environment/hcle_environment.cpp
    src/hcle/common/display.cpp
    src/hcle/games/smb1.cpp
)

# CORE FILES
add_library(hcle_core STATIC ${HCLE_CORE_SOURCES})
target_include_directories(hcle_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(hcle_core PUBLIC SDL2::SDL2 SDL2::SDL2main ${OpenCV_LIBS})

# PYTHON MODULE
pybind11_add_module(_hcle_py
    src/hcle/python/python_interface.cpp
    src/hcle/python/python_vector_interface.cpp
)
target_link_libraries(_hcle_py PRIVATE hcle_core)
set_target_properties(_hcle_py PROPERTIES DEBUG_POSTFIX "_d")
install(TARGETS _hcle_py
        LIBRARY DESTINATION hcle_py)

install(FILES $<TARGET_RUNTIME_DLLS:_hcle_py> DESTINATION hcle_py)
        
# TEST CPP RUNNER
add_executable(hcle_test src/apps/test_runner.cpp)
target_link_libraries(hcle_test PRIVATE hcle_core)

# COPY SDL2.dll TEST DIR
add_custom_command(
    TARGET hcle_test
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:SDL2::SDL2>" 
        "$<TARGET_FILE_DIR:hcle_test>"
    COMMENT "Copying SDL2.dll to output directory"
)