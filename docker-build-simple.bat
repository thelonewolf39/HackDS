@echo off
REM Simple HackDS Builder for Windows
REM Double-click to build!

title HackDS Builder

color 0A
cls

echo.
echo  ============================================
echo   HackDS Build System for Windows
echo  ============================================
echo.
echo  This will build HackDS using Docker.
echo  Time required: 30-60 minutes
echo.
echo  Requirements:
echo  - Docker Desktop installed and running
echo  - 20GB free disk space
echo  - Internet connection
echo.
echo  ============================================
echo.
pause

REM Check Docker
echo Checking Docker...
docker --version >nul 2>&1
if errorlevel 1 (
    color 0C
    echo.
    echo  [ERROR] Docker not found!
    echo.
    echo  Please install Docker Desktop:
    echo  https://www.docker.com/products/docker-desktop
    echo.
    echo  Then restart this script.
    echo.
    pause
    exit /b 1
)

color 0B
echo Docker found!
echo.
echo Starting build...
echo.
echo Please be patient - this takes a while!
echo You can minimize this window.
echo.

REM Build and run
docker build -t hackds-builder . 2>&1
if errorlevel 1 goto error

docker run --rm --privileged -v "%cd%:/hackds" hackds-builder /bin/bash -c "cd /hackds && chmod +x build-all.sh && ./build-all.sh" 2>&1
if errorlevel 1 goto error

color 0A
cls
echo.
echo  ============================================
echo   BUILD COMPLETE!
echo  ============================================
echo.
echo  Your HackDS image is ready:
echo  Location: output\hackds-0.1.0.img
echo.
echo  Next steps:
echo  1. Download balenaEtcher from:
echo     https://etcher.balena.io/
echo.
echo  2. Flash output\hackds-0.1.0.img to SD card
echo.
echo  3. Insert SD card into Raspberry Pi Zero 2W
echo.
echo  4. Power on and enjoy!
echo.
echo  ============================================
echo.
echo  Press any key to open output folder...
pause >nul
explorer output
exit /b 0

:error
color 0C
echo.
echo  ============================================
echo   BUILD FAILED
echo  ============================================
echo.
echo  Check the error messages above.
echo.
echo  Common issues:
echo  - Docker not running
echo  - Not enough disk space
echo  - Network connection lost
echo.
echo  Try running again - it might work!
echo.
pause
exit /b 1
