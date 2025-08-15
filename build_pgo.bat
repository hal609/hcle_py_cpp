@echo off
set BUILD_DIR=build_pgo
set BUILD_TYPE=Release
set TARGET_EXE=hcle_test

echo --- Cleaning previous PGO build...
if exist %BUILD_DIR% rmdir /s /q %BUILD_DIR%
mkdir %BUILD_DIR%

echo.
echo === PHASE 1: INSTRUMENT ===
echo.

echo --- Configuring PGO-Instrument build...
cmake -S . -B %BUILD_DIR% -A x64 -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
      -DCMAKE_TOOLCHAIN_FILE=C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake ^
      -DCMAKE_CXX_FLAGS_RELEASE="/GL" ^
      -DCMAKE_EXE_LINKER_FLAGS_RELEASE="/LTCG:PGI /GENPROFILE"

if %errorlevel% neq 0 (echo CMake PGO-Instrument configure failed. & exit /b %errorlevel%)

echo --- Building instrumented target...
cmake --build %BUILD_DIR% --config %BUILD_TYPE% --target %TARGET_EXE%
if %errorlevel% neq 0 (echo PGO-Instrument build failed. & exit /b %errorlevel%)

echo.
echo === PHASE 2: TRAIN ===
echo.

echo --- Running training workload...
rem The working directory must be where the PGD/PGC files are located
pushd %BUILD_DIR%\%BUILD_TYPE%
.\%TARGET_EXE%.exe
popd
if %errorlevel% neq 0 (echo Training run failed. & exit /b %errorlevel%)


echo.
echo === PHASE 3: OPTIMIZE ===
echo.

echo --- Configuring PGO-Optimize build...
rem We don't need to re-run the full CMake configure, just change flags and rebuild.
rem The crucial flag is /USEPROFILE, which tells the linker to use the PGD data.
cmake -S . -B %BUILD_DIR% -A x64 -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
      -DCMAKE_TOOLCHAIN_FILE=C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake ^
      -DCMAKE_CXX_FLAGS_RELEASE="/O2 /GL /fp:fast" ^
      -DCMAKE_EXE_LINKER_FLAGS_RELEASE="/LTCG:PGO /USEPROFILE"

if %errorlevel% neq 0 (echo CMake PGO-Optimize configure failed. & exit /b %errorlevel%)

@REM echo --- Cleaning intermediate files before final build...
@REM cmake --build %BUILD_DIR% --config %BUILD_TYPE% --target clean
@REM if %errorlevel% neq 0 (echo PGO clean step failed. & exit /b %errorlevel%)

echo --- Building final optimized target...
cmake --build %BUILD_DIR% --config %BUILD_TYPE% --target %TARGET_EXE% 
if %errorlevel% neq 0 (echo PGO-Optimize build failed. & exit /b %errorlevel%)

echo.
echo --- PGO build complete! The optimized executable is in %BUILD_DIR%\%BUILD_TYPE% ---