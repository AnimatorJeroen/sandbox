@echo off
setlocal enabledelayedexpansion

rem Re-launch inside a persistent cmd window so output is never lost
if not defined WASM_LAUNCHED (
    set WASM_LAUNCHED=1
    cmd /k "%~f0" %*
    exit /b
)

rem Check if em++ is available, install emsdk if not
where em++ >nul 2>&1
if errorlevel 1 (
    echo em++ not found. Installing Emscripten SDK...

    where git >nul 2>&1
    if errorlevel 1 (
        echo ERROR: git is not installed or not in PATH. Cannot install emsdk.
        echo Install Git from: https://git-scm.com/download/win
        exit /b 1
    )

    set EMSDK_DIR=%USERPROFILE%\emsdk

    if not exist "!EMSDK_DIR!" (
        echo Cloning emsdk to !EMSDK_DIR!...
        git clone https://github.com/emscripten-core/emsdk.git "!EMSDK_DIR!"
        if errorlevel 1 (
            echo ERROR: Failed to clone emsdk.
            exit /b 1
        )
    ) else (
        echo emsdk already exists at !EMSDK_DIR!, pulling latest...
        cd /d "!EMSDK_DIR!"
        git pull
        cd /d "%~dp0"
    )

    echo Installing and activating latest Emscripten...
    call "!EMSDK_DIR!\emsdk.bat" install latest
    if errorlevel 1 (
        echo ERROR: emsdk install failed.
        exit /b 1
    )
    call "!EMSDK_DIR!\emsdk.bat" activate latest
    if errorlevel 1 (
        echo ERROR: emsdk activate failed.
        exit /b 1
    )
    call "!EMSDK_DIR!\emsdk_env.bat"

    where em++ >nul 2>&1
    if errorlevel 1 (
        echo ERROR: em++ still not found after install. Try restarting this script.
        exit /b 1
    )
    echo Emscripten installed successfully.
)

rem Check for release or debug argument
if "%1"=="release" (
    echo Building in Release mode...
    set BUILD_TYPE=Release
    set COMPILER_OPTS=-O3 -std=c++17
    set OUTPUT_DIR=build\ReleaseWasm
) else (
    echo Building in Debug mode...
    set BUILD_TYPE=Debug
    set COMPILER_OPTS=-O0 -g -std=c++17
    set OUTPUT_DIR=build\DebugWasm
)

rem Include directories
set INCLUDE_OPTS=-I source -I vendor\include -I vendor\include\imgui

rem Create output folder
if not exist build (
    mkdir build
)
if not exist %OUTPUT_DIR% (
    mkdir %OUTPUT_DIR%
)

rem Collect all source .cpp files (excluding pch.cpp and MeshImporter which uses Assimp)
set SRC_FILES=
for /R source %%f in (*.cpp) do (
    echo %%f | findstr /i "pch.cpp" >nul
    if errorlevel 1 (
        echo %%f | findstr /i "MeshImporter" >nul
        if errorlevel 1 (
            set "SRC_FILES=!SRC_FILES! %%f"
        )
    )
)

rem Collect vendor .cpp files
for /R vendor %%f in (*.cpp) do (
    set "SRC_FILES=!SRC_FILES! %%f"
)

echo Compiling project for WebAssembly...
em++ %COMPILER_OPTS% %INCLUDE_OPTS% ^
    -DPLATFORM_WASM -DGLEW_STATIC ^
    !SRC_FILES! ^
    -o %OUTPUT_DIR%\sandbox.html ^
    -s USE_GLFW=3 -s USE_WEBGL2=1 -s FULL_ES3=1 ^
    -s ALLOW_MEMORY_GROWTH=1 -s WASM_MEM_MAX=512MB ^
    -s ASYNCIFY=1 ^
    --preload-file Assets@/Assets

if errorlevel 1 (
    echo.
    echo Build failed. See errors above.
    cmd /k
)

echo Build succeeded. Serving at http://localhost:8080/sandbox.html
start http://localhost:8080/sandbox.html
cd %OUTPUT_DIR%
python -m http.server 8080
echo.
echo Server stopped. Press any key to close...
pause > nul
