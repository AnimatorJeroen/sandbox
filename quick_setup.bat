@echo off
REM Quick setup script - runs dependency installation and project generation
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
cd premake
call premake5 vs2022
cd ..

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Project generation failed
    pause
    exit /b 1
)

echo.
echo ======================================
echo   Setup Complete!
echo ======================================
echo.
echo You can now:
echo   1. Open Sandbox.sln
echo   2. Build and run (F5)
echo   3. Import FBX models with Ctrl+I
echo.
pause
