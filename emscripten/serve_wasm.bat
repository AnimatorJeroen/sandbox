@echo off
setlocal enabledelayedexpansion

if "%1"=="release" (
    set SERVE_DIR=%~dp0..\build\ReleaseWasm
) else (
    set SERVE_DIR=%~dp0..\build\DebugWasm
)

rem Normalize path to resolve any .. segments
for %%i in ("!SERVE_DIR!") do set "SERVE_DIR=%%~fi"

if not exist "!SERVE_DIR!\sandbox.html" (
    echo ERROR: sandbox.html not found in !SERVE_DIR!
    echo Run build_wasm.bat first.
    pause
    exit /b 1
)

echo Serving WASM build at http://localhost:8080/sandbox.html
echo Press Ctrl+C to stop.
start http://localhost:8080/sandbox.html
cd /d "!SERVE_DIR!"
python -m http.server 8080
