@echo off
echo ========================================
echo Building ChangeIPTool with qmake
echo ========================================

REM Set Qt path - modify this to match your Qt installation
set QT_PATH=C:\Qt\6.x\mingw_64

REM Add Qt tools to PATH
set PATH=%QT_PATH%\bin;%QT_PATH%\Tools\mingw\bin;%PATH%

echo Step 1: Cleaning previous build...
if exist Makefile del Makefile
if exist Makefile.Debug del Makefile.Debug
if exist Makefile.Release del Makefile.Release
if exist debug rmdir /s /q debug
if exist release rmdir /s /q release

echo Step 2: Running qmake...
qmake ChangeIPTool.pro
if errorlevel 1 (
    echo qmake failed!
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

copy release\ChangeIPTool.exe deploy\

cd deploy
windeployqt ChangeIPTool.exe --release --no-translations
cd ..

echo ========================================
echo Build completed successfully!
echo ========================================
echo Output location: deploy\ChangeIPTool.exe
pause
