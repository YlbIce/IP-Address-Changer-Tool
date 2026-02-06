@echo off
echo ========================================
echo ChangeIPTool - Build and Package Script
echo ========================================

REM Set Qt path - modify this to match your Qt installation
set QT_PATH=C:\Qt\6.x\mingw_64

REM Add Qt tools to PATH
set PATH=%QT_PATH%\bin;%QT_PATH%\Tools\mingw\bin;%PATH%

echo Step 1: Creating build directory...
if not exist build mkdir build
cd build

echo Step 2: Configuring with CMake...
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..
if errorlevel 1 (
    echo CMake configuration failed!
    echo Please ensure Qt and CMake are properly installed.
    pause
    exit /b 1
)

echo Step 3: Building project...
mingw32-make -j4
if errorlevel 1 (
    echo Build failed!
    pause
    exit /b 1
)

echo Step 4: Creating deployment package...
if not exist deploy mkdir deploy
cd deploy

copy ..\ChangeIPTool.exe .

echo Step 5: Deploying Qt libraries...
windeployqt ChangeIPTool.exe --release --no-translations

echo Step 6: Creating installer...
cd ..
cpack -G NSIS

echo ========================================
echo Build and packaging completed!
echo ========================================
echo.
echo Output files:
echo - Executable: build\deploy\ChangeIPTool.exe
echo - Installer: build\ChangeIPTool-1.0.0-win64.exe
echo - Portable ZIP: build\ChangeIPTool-1.0.0-win64.zip
echo.
pause
