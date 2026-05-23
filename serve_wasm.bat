@echo off
echo Serving WASM build at http://localhost:8080/sandbox.html
echo Press Ctrl+C to stop.
start http://localhost:8080/sandbox.html
cd build\DebugWasm
python -m http.server 8080
