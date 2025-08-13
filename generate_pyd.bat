@echo off
echo --- Cleaning previous build...
if exist build rmdir /s /q build

cmake -S . -B build -A x64 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake

if %errorlevel% neq 0 (
    echo CMake configuration failed.
    exit /b %errorlevel%
)

echo --- Building Debug target...
cmake --build build --config Debug --target _hcle_py

if %errorlevel% neq 0 (
    echo Build failed.
    exit /b %errorlevel%
)

echo --- Debug build complete! ---