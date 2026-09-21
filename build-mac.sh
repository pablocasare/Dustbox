#!/usr/bin/env bash
# One-shot build for macOS. First run downloads JUCE and takes a while.
set -e
cd "$(dirname "$0")"
command -v cmake >/dev/null || { echo "CMake not found. Install it: brew install cmake"; exit 1; }
cmake -B build -G Xcode
cmake --build build --config Release
echo
echo "Done. Installed to:"
echo "  ~/Library/Audio/Plug-Ins/VST3/Dust Box.vst3"
echo "  ~/Library/Audio/Plug-Ins/Components/Dust Box.component"
echo "Rescan plugins in your DAW to pick it up."
