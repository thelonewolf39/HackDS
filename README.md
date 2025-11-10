# HackDS Operating System

A custom gaming operating system for Raspberry Pi Zero 2W

## Overview

HackDS is a minimal, custom-built operating system designed specifically for running games on the Raspberry Pi Zero 2W. It features a lightweight graphical interface and supports games written in Python and C/C++.

## Features

- **Custom File Formats**: Specialized formats for games, mods, settings, and hacks
- **Multi-Language Support**: Run Python and C++ games
- **GUI Menu System**: Easy-to-use graphical interface (no cursor!)
- **PS5 Controller Support**: Full DualSense wireless controller support
- **Bluetooth & WiFi**: Built-in wireless connectivity management
- **Auto-Update System**: Automatically check and install updates from GitHub
- **Settings Menu**: Configure WiFi, Bluetooth, and system settings
- **Lightweight**: Optimized for Pi Zero 2W's limited resources
- **Safe Updates**: Automatic backup and rollback support
- **Game Packaging**: Easy-to-use tools for creating game packages

## File Formats

- **.hdsg** - HackDS Game files
- **.hdsm** - HackDS Mod files
- **.hdss** - HackDS Settings files
- **.hdsh** - HackDS Hacks files

All formats use custom binary structures for efficiency.

## Architecture

- **Kernel**: Custom Linux kernel configured for Pi Zero 2W
- **Graphics**: Direct framebuffer/DRM for lightweight rendering
- **Init System**: Minimal custom init
- **Game Runtime**: Python 3 + C/C++ support
- **UI**: Custom GUI menu system

## Directory Structure

```
/
├── boot/           - Boot files and kernel
├── system/         - Core system files
├── games/          - Installed games (.hdsg)
├── mods/           - Game modifications (.hdsm)
├── settings/       - System and game settings (.hdss)
└── hacks/          - Game hacks/cheats (.hdsh)
```

## Quick Start

### 🎯 Easiest Way: Download Pre-Built Image (Recommended!)

**Total time: 15 minutes** - No building required!

1. **Download** latest release: [Releases Page](https://github.com/thelonewolf39/HackDS/releases/latest)
2. **Extract** the ZIP file to get the `.img` file
3. **Flash** with balenaEtcher: https://etcher.balena.io/
4. **Boot** your Raspberry Pi Zero 2W and enjoy! 🎮

📖 **Full guide**: See `NO_BUILD_GUIDE.md` - Just download, flash, and play!

---

### 🔨 Build It Yourself (Advanced)

**Windows Users (No WSL):**

1. Install Docker Desktop: https://www.docker.com/products/docker-desktop
2. Double-click `docker-build-simple.bat` and wait 30-60 minutes
3. Flash `output\hackds-0.1.0.img` to SD card
4. Boot and enjoy!

📖 **Full guide**: See `BUILD_ON_WINDOWS.md`

**Linux/Mac Users:**

1. Follow `QUICKSTART.md` for step-by-step instructions
2. Flash the built image to your SD card
3. Boot and play!

📖 **Full guide**: See `docs/BUILD.md`

## Building Options

| Method | Time | Difficulty | Guide |
|--------|------|------------|-------|
| **Download Pre-Built** | 15 min | ⭐ Easy | `NO_BUILD_GUIDE.md` |
| Windows (Docker) | 60 min | ⭐⭐ Medium | `BUILD_ON_WINDOWS.md` |
| Linux/WSL | 60 min | ⭐⭐⭐ Advanced | `docs/BUILD.md` |

💡 **Recommendation**: Use pre-built images unless you're modifying the code!

## Controller & Settings

### PS5 DualSense Controller

Full wireless support for PS5 controllers:

```bash
# Quick setup
bluetooth-manager ps5-setup
```

**Controller mapping:**
- **D-Pad**: Navigate menus
- **X Button**: Launch game/Select
- **Triangle**: Check for updates
- **Options**: Open settings

See `docs/SETTINGS_GUIDE.md` for complete guide.

### WiFi & Bluetooth

Manage wireless connections easily:

```bash
# WiFi
wifi-manager scan
wifi-manager connect "NetworkName" "password"

# Bluetooth
bluetooth-manager scan
bluetooth-manager pair <MAC_ADDRESS>
```

**Settings Menu:**
- Press `F1` or `Tab` in main menu
- Or press `Options` button on controller
- Configure WiFi, Bluetooth, and system settings

## Auto-Updates

HackDS can automatically update from your GitHub repository:

- Check for updates: Press `U` in the menu
- Install updates: Press `I` when available
- Configure in: `/settings/update-settings.json`
- Full docs: See `docs/AUTO_UPDATE.md`

## Target Hardware

- **Device**: Raspberry Pi Zero 2W
- **CPU**: Quad-core ARM Cortex-A53 @ 1GHz
- **RAM**: 512MB
- **Architecture**: ARMv8-A (64-bit)
