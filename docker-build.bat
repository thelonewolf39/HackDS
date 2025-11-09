@echo off
REM HackDS Docker Build Script for Windows
REM No WSL required - just Docker Desktop!

echo ====================================
echo HackDS Docker Build System
echo ====================================
echo.

REM Check if Docker is installed
docker --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: Docker is not installed or not running!
    echo.
    echo Please install Docker Desktop from:
    echo https://www.docker.com/products/docker-desktop
    echo.
    pause
    exit /b 1
)

echo Docker detected!
echo.

REM Build the Docker image
echo Step 1/3: Building Docker container...
docker build -t hackds-builder .
if errorlevel 1 (
    echo ERROR: Failed to build Docker image
    pause
    exit /b 1
)

echo.
echo Step 2/3: Building HackDS in container...
echo This will take 30-60 minutes...
echo.

REM Run the build inside Docker
docker run --rm --privileged ^
    -v "%cd%:/hackds" ^
    hackds-builder ^
    /bin/bash -c "cd /hackds && chmod +x build-all.sh && ./build-all.sh"

if errorlevel 1 (
    echo ERROR: Build failed
    pause
    exit /b 1
)

echo.
echo Step 3/3: Build complete!
echo.
echo Your HackDS image is ready:
echo Location: output\hackds-0.1.0.img
echo.
echo To flash to SD card:
echo 1. Download Rufus or balenaEtcher
echo 2. Select the .img file from output folder
echo 3. Select your SD card
echo 4. Flash!
echo.
pause
