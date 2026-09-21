@echo off
REM One-shot build for Windows. First run downloads JUCE and takes a while.
cd /d "%~dp0"
where cmake >nul 2>nul || (echo CMake not found - install it from cmake.org & exit /b 1)
cmake -B build -G "Visual Studio 17 2022" -A x64 || exit /b 1
cmake --build build --config Release || exit /b 1
echo.
echo Done. VST3 installed to C:\Program Files\Common Files\VST3\Dust Box.vst3
echo Rescan plugins in your DAW to pick it up.
