@echo off

REM Create build directory if it doesn't exist
if not exist build (
    mkdir build
)

REM Change into build directory
cd build

REM Run CMake configure step with Visual Studio 17 2022 generator
cmake -G "Visual Studio 17 2022" ..

REM Check if configuration succeeded
if errorlevel 1 (
    echo CMake configuration failed!
    exit /b 1
)

REM Build the project
cmake --build .

REM Check if build succeeded
if errorlevel 1 (
    echo Build failed!
    exit /b 1
)

echo Build succeeded!
pause
