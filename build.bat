@echo off

rem Set up CMake variables
set GENERATOR="Visual Studio 17 2022"
set BUILD_DIR=build

rem Create the build directory if it doesn't exist
if not exist %BUILD_DIR% mkdir %BUILD_DIR%

rem Change directory to the build directory
cd %BUILD_DIR%

rem Configure the project using CMake
cmake -G %GENERATOR% ..

rem Build the project
cmake --build .

rem Optionally install the project
rem cmake --install .

rem Display build output
echo Build completed. Press any key to exit.
pause > nul
