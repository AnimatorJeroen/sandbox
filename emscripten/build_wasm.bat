@echo off
setlocal enabledelayedexpansion

rem Re-launch inside a persistent cmd window so output is never lost
if not defined WASM_LAUNCHED (
    set WASM_LAUNCHED=1
    cmd /k "%~f0" %*
    exit /b
)

rem This script lives in emscripten\ — project root is one level up
set SCRIPT_DIR=%~dp0
if "%SCRIPT_DIR:~-1%"=="\" set "SCRIPT_DIR=%SCRIPT_DIR:~0,-1%"
set PROJECT_DIR=%SCRIPT_DIR%\..

rem ── Check for release or debug argument ───────────────────────────────────
if "%1"=="release" (
    echo Building in Release mode...
    set BUILD_TYPE=Release
    set OUTPUT_DIR=build\ReleaseWasm
) else (
    echo Building in Debug mode...
    set BUILD_TYPE=Debug
    set OUTPUT_DIR=build\DebugWasm
)

rem ── Ensure Git is available (required for emsdk) ───────────────────────────
where git >nul 2>&1
if errorlevel 1 (
    echo ERROR: git is not installed or not in PATH.
    echo Install Git from: https://git-scm.com/download/win
    cmd /k
    exit /b 1
)

rem ── Install / update emsdk ─────────────────────────────────────────────────
set EMSDK_DIR=%USERPROFILE%\emsdk

where em++ >nul 2>&1
if errorlevel 1 (
    echo em++ not found. Setting up Emscripten SDK...

    if not exist "!EMSDK_DIR!" (
        echo Cloning emsdk to !EMSDK_DIR!...
        git clone https://github.com/emscripten-core/emsdk.git "!EMSDK_DIR!"
        if errorlevel 1 ( echo ERROR: Failed to clone emsdk. & cmd /k & exit /b 1 )
    ) else (
        echo emsdk already exists at !EMSDK_DIR!, pulling latest...
        cd /d "!EMSDK_DIR!"
        git pull
        cd /d "%PROJECT_DIR%"
    )

    call "!EMSDK_DIR!\emsdk.bat" install latest
    if errorlevel 1 ( echo ERROR: emsdk install failed. & cmd /k & exit /b 1 )

    call "!EMSDK_DIR!\emsdk.bat" activate latest
    if errorlevel 1 ( echo ERROR: emsdk activate failed. & cmd /k & exit /b 1 )

    call "!EMSDK_DIR!\emsdk_env.bat"
)

rem Source emsdk_env.bat if emsdk exists at the default location (puts emcmake/emmake on PATH)
if exist "!EMSDK_DIR!\emsdk_env.bat" (
    call "!EMSDK_DIR!\emsdk_env.bat"
)

where em++ >nul 2>&1
if errorlevel 1 (
    echo ERROR: em++ still not found after install. Try restarting this script.
    echo If emsdk is installed in a non-default location, add it to PATH manually.
    cmd /k
    exit /b 1
)

rem ── Install CMake via winget if not available ──────────────────────────────
where cmake >nul 2>&1
if errorlevel 1 (
    echo cmake not found. Installing via winget...
    winget install --id Kitware.CMake --silent --accept-source-agreements --accept-package-agreements
    if errorlevel 1 (
        echo ERROR: winget install of CMake failed.
        echo Please install CMake manually from: https://cmake.org/download/
        cmd /k
        exit /b 1
    )
    rem Refresh PATH so cmake is visible in this session
    for /f "tokens=*" %%i in ('where cmake 2^>nul') do set CMAKE_EXE=%%i
    if not defined CMAKE_EXE (
        echo CMake was installed but is not on PATH yet.
        echo Please restart this script or open a new terminal.
        cmd /k
        exit /b 1
    )
    echo CMake installed successfully.
)

rem ── Install Ninja via winget if not available ────────────────────────────────
where ninja >nul 2>&1
if errorlevel 1 (
    echo ninja not found. Installing via winget...
    winget install --id Ninja-build.Ninja --silent --accept-source-agreements --accept-package-agreements
    if errorlevel 1 (
        echo ERROR: winget install of Ninja failed.
        echo Please install Ninja manually from: https://ninja-build.org/
        cmd /k
        exit /b 1
    )
    rem Refresh PATH for this session
    for /f "tokens=*" %%i in ('where ninja 2^>nul') do set NINJA_EXE=%%i
    if not defined NINJA_EXE (
        echo Ninja was installed but is not on PATH yet. Please restart this script.
        cmd /k
        exit /b 1
    )
    echo Ninja installed successfully.
)

rem ── Configure with emcmake (only when build dir is missing or forced) ──────
set BUILD_DIR=%PROJECT_DIR%\%OUTPUT_DIR%

if not exist "%BUILD_DIR%\build.ninja" (
    echo Configuring CMake for WebAssembly...
    emcmake cmake -S "%SCRIPT_DIR%" -B "%BUILD_DIR%" -G Ninja -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
    if errorlevel 1 (
        echo.
        echo CMake configuration failed. See errors above.
        cmd /k
        exit /b 1
    )
) else (
    echo CMake already configured. Skipping configure step.
    echo [Tip] Delete %OUTPUT_DIR% to force a full reconfigure.
)

rem ── Build with emmake (incremental, parallel) ──────────────────────────────
echo.
echo Building...
emmake cmake --build "%BUILD_DIR%" --parallel
if errorlevel 1 (
    echo.
    echo Build failed. See errors above.
    cmd /k
    exit /b 1
)

rem ── Serve ──────────────────────────────────────────────────────────────────
echo.
echo Build succeeded. Serving at http://localhost:8080/sandbox.html
start http://localhost:8080/sandbox.html
cd /d "%BUILD_DIR%"
python -m http.server 8080
echo.
echo Server stopped. Press any key to close...
pause > nul
