@echo off
REM Download latest HackDS release (no building required!)

title HackDS Downloader

color 0B
cls

echo.
echo  ============================================
echo   HackDS Image Downloader
echo  ============================================
echo.
echo  This downloads the latest pre-built image.
echo  No building, no Docker, no waiting!
echo.
echo  Just download and flash to SD card.
echo.
echo  ============================================
echo.

REM Set your repository here
set REPO=yourusername/HackDS

echo Fetching latest release from GitHub...
echo Repository: %REPO%
echo.

REM Create output directory
if not exist "output" mkdir output

echo.
echo Opening GitHub releases page in browser...
echo.
echo Please download the .img.zip file and extract it.
echo Then use balenaEtcher to flash to SD card.
echo.

start https://github.com/%REPO%/releases/latest

echo.
echo ============================================
echo  Next Steps:
echo ============================================
echo.
echo  1. Download hackds-vX.X.X.img.zip from browser
echo  2. Extract the .img file from the ZIP
echo  3. Download balenaEtcher:
echo     https://etcher.balena.io/
echo  4. Flash the .img to your SD card
echo  5. Boot your Raspberry Pi!
echo.
echo ============================================
echo.
pause
