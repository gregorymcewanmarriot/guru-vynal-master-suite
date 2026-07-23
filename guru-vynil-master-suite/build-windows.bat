@echo off
setlocal

where cmake >nul 2>nul
if errorlevel 1 (
  echo ERROR: CMake was not found in PATH.
  exit /b 1
)

where git >nul 2>nul
if errorlevel 1 (
  echo ERROR: Git was not found in PATH.
  exit /b 1
)

echo Configuring GURU Vynil Master Suite...
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
if errorlevel 1 exit /b 1

echo Building Release VST3 and Standalone targets...
cmake --build build --config Release
if errorlevel 1 exit /b 1

echo.
echo Build complete.
echo Check: build\GuruVynilMasterSuite_artefacts\Release\VST3\
endlocal
