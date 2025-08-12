@echo off
echo --- Cleaning previous build...
if exist build rmdir /s /q build

echo --- Configuring CMake...
mkdir build
cmake -S . -B build -A x64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake
if %errorlevel% neq 0 (
    echo CMake configuration failed.
    exit /b %errorlevel%
)

echo --- Building target...
cmake --build build --config Release --target hcle_test
if %errorlevel% neq 0 (
    echo Build failed.
    exit /b %errorlevel%
)

echo --- Build complete! ---