@echo off
if "%1"=="release" (
    set SERVE_DIR=%~dp0..\build\ReleaseWasm
) else (
    set SERVE_DIR=%~dp0..\build\DebugWasm
)

echo Serving WASM build at http://localhost:8080/sandbox.html
echo Press Ctrl+C to stop.
start http://localhost:8080/sandbox.html
cd /d "%SERVE_DIR%"
python -m http.server 8080
