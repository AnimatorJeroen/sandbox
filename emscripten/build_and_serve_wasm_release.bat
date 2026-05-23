@echo off
setlocal

rem Set WASM_LAUNCHED to skip the re-launch in build_wasm.bat so it blocks until done
set WASM_LAUNCHED=1
call "%~dp0build_wasm.bat" release
if errorlevel 1 ( pause & exit /b 1 )

call "%~dp0serve_wasm.bat" release
