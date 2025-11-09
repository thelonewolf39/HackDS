# HackDS Operating System

A custom gaming operating system for Raspberry Pi Zero 2W

## Overview

HackDS is a minimal, custom-built operating system designed specifically for running games on the Raspberry Pi Zero 2W. It features a lightweight graphical interface and supports games written in Python and C/C++.

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

## Building

See `docs/BUILD.md` for build instructions.

## Target Hardware

- **Device**: Raspberry Pi Zero 2W
- **CPU**: Quad-core ARM Cortex-A53 @ 1GHz
- **RAM**: 512MB
- **Architecture**: ARMv8-A (64-bit)
