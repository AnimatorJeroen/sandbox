@echo off
REM ==================================================
REM Quick Setup Script for Sandbox Project
REM Installs dependencies and generates project files
REM ==================================================

cd /d "%~dp0"

echo.
echo ======================================
echo   Sandbox Project - Quick Setup
echo ======================================
echo.

REM Step 1: Install Assimp if needed
echo [1/2] Checking dependencies...
call setup_assimp.bat
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Dependency installation failed
    pause
    exit /b 1
)

REM Step 2: Generate project files
echo.
echo [2/2] Generating Visual Studio project...
call runPremake_visualStudio.bat

echo.
echo ======================================
echo   Setup Complete!
echo ======================================
echo.
echo You can now:
echo   1. Open Sandbox.sln (in parent directory)
echo   2. Build and run (F5)
echo   3. Import FBX models with Ctrl+I
echo.
pause
