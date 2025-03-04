@echo off
cd /d "%~dp0"
echo Running Premake5...
premake5 vs2022
echo Done!
pause