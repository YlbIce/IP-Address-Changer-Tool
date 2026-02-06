@echo off
echo ========================================
echo Building ChangeIPTool
echo ========================================

REM Create build directory
if not exist build mkdir build
cd build

REM Configure with CMake
echo Configuring project...
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..
if errorlevel 1 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

REM Build the project
echo Building project...
cmake --build . --config Release
if errorlevel 1 (
    echo Build failed!
    pause
    exit /b 1
)

echo ========================================
echo Build completed successfully!
echo ========================================

REM Create deploy directory
if not exist deploy mkdir deploy
cd deploy

REM Copy executable
copy ..\Release\ChangeIPTool.exe .

REM Copy Qt DLLs (adjust paths based on your Qt installation)
echo Deploying Qt libraries...
windeployqt ChangeIPTool.exe --release --no-translations

echo ========================================
echo Package created in build\deploy directory
echo ========================================
pause
