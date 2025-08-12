@echo off
cmake -S . -B build -A x64 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake

if %errorlevel% neq 0 (
    echo CMake configuration failed.
    exit /b %errorlevel%
)

echo --- Building DEBUG target...
cmake --build build --config Debug --target hcle_test

if %errorlevel% neq 0 (
    echo Build failed.
    exit /b %errorlevel%
)

echo --- Debug build complete! ---