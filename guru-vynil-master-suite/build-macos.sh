#!/usr/bin/env bash
set -euo pipefail

cmake -S . -B build -G Xcode
cmake --build build --config Release

echo "Build complete. Check build/GuruVynilMasterSuite_artefacts/Release/."
