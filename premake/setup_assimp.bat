@echo off
REM ==================================================
REM Assimp Setup Script for Sandbox Project
REM ==================================================
REM This script will:
REM 1. Check if vcpkg is installed
REM 2. Install vcpkg if needed
REM 3. Install Assimp via vcpkg
REM 4. Integrate vcpkg with Visual Studio
REM ==================================================

cd /d "%~dp0"
cd ..

echo.
echo ======================================
echo   Sandbox Project - Assimp Setup
echo ======================================
echo.

REM Check if vcpkg is already in PATH or exists locally
where vcpkg >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo [INFO] vcpkg found in PATH
    goto :install_assimp
)

REM Check if vcpkg exists in common locations
if exist "%USERPROFILE%\vcpkg\vcpkg.exe" (
    echo [INFO] vcpkg found at %USERPROFILE%\vcpkg
    set "VCPKG_ROOT=%USERPROFILE%\vcpkg"
    goto :install_assimp
)

if exist "C:\vcpkg\vcpkg.exe" (
    echo [INFO] vcpkg found at C:\vcpkg
    set "VCPKG_ROOT=C:\vcpkg"
    goto :install_assimp
)

if exist "%CD%\vcpkg\vcpkg.exe" (
    echo [INFO] vcpkg found in current directory
    set "VCPKG_ROOT=%CD%\vcpkg"
    goto :install_assimp
)

REM vcpkg not found, offer to install
echo [WARN] vcpkg not found on this system
echo.
echo vcpkg is required to install Assimp automatically.
echo.
set /p INSTALL_VCPKG="Would you like to install vcpkg now? (Y/N): "

if /i "%INSTALL_VCPKG%" NEQ "Y" (
    echo.
    echo [INFO] Installation cancelled. Please install vcpkg manually:
    echo   1. git clone https://github.com/Microsoft/vcpkg.git
    echo   2. cd vcpkg
    echo   3. bootstrap-vcpkg.bat
    echo   4. vcpkg install assimp:x64-windows
    echo   5. vcpkg integrate install
    echo.
    pause
    exit /b 1
)

:download_vcpkg
echo.
echo [INFO] Installing vcpkg...
echo.

REM Clone vcpkg to user directory
cd /d "%USERPROFILE%"
if exist "vcpkg" (
    echo [WARN] vcpkg directory already exists, using existing installation
    cd vcpkg
) else (
    echo [INFO] Cloning vcpkg from GitHub...
    git clone https://github.com/Microsoft/vcpkg.git
    if %ERRORLEVEL% NEQ 0 (
        echo [ERROR] Failed to clone vcpkg. Make sure git is installed.
        pause
        exit /b 1
    )
    cd vcpkg
)

echo [INFO] Bootstrapping vcpkg...
call bootstrap-vcpkg.bat
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Failed to bootstrap vcpkg
    pause
    exit /b 1
)

set "VCPKG_ROOT=%USERPROFILE%\vcpkg"
echo [SUCCESS] vcpkg installed successfully!
echo.

:install_assimp
echo.
echo [INFO] Installing Assimp via vcpkg...
echo This may take several minutes on first install...
echo.

REM Add vcpkg to PATH temporarily if not already there
if defined VCPKG_ROOT (
    set "PATH=%VCPKG_ROOT%;%PATH%"
)

REM Install Assimp for x64-windows
vcpkg install assimp:x64-windows
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Failed to install Assimp
    pause
    exit /b 1
)

echo.
echo [SUCCESS] Assimp installed successfully!
echo.

:integrate_vcpkg
echo [INFO] Integrating vcpkg with Visual Studio...
vcpkg integrate install
if %ERRORLEVEL% NEQ 0 (
    echo [WARN] Failed to integrate vcpkg with Visual Studio
    echo You may need to run this script as Administrator
    echo.
) else (
    echo [SUCCESS] vcpkg integrated with Visual Studio!
    echo.
)

:finalize
echo.
echo ======================================
echo   Setup Complete!
echo ======================================
echo.
echo Assimp has been installed successfully.
echo.
echo Next steps:
echo   1. Run: runPremake_visualStudio.bat
echo   2. Open Sandbox.sln in Visual Studio
echo   3. Build the project (F7)
echo   4. Press Ctrl+I in the editor to import FBX models!
echo.
echo See ASSIMP_SETUP.md for more information.
echo.
pause
