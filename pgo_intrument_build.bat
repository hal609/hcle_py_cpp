@echo off
cmake -S . -B build -A x64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_CXX_FLAGS_RELEASE="/GL" -DCMAKE_EXE_LINKER_FLAGS_RELEASE="/GENPROFILE /LTCG:PGI"

if %errorlevel% neq 0 (
    echo CMake configuration failed.
    exit /b %errorlevel%
)

echo --- Building Release target...
cmake --build build --config Release --target hcle_test

if %errorlevel% neq 0 (
    echo Build failed.
    exit /b %errorlevel%
)

echo --- Release build complete! ---

echo --- 2. Running training scenario ---
.\build\Release\hcle_test.exe