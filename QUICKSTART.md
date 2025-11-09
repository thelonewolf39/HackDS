# HackDS Quick Start Guide

## Overview

HackDS is a custom gaming operating system for Raspberry Pi Zero 2W. This guide will help you build the OS, create games, and deploy to your device.

## Prerequisites

- Linux development machine (Ubuntu 20.04+ recommended) or WSL2
- Raspberry Pi Zero 2W
- MicroSD card (8GB minimum, 16GB+ recommended)
- SD card reader

## Quick Build

### 1. Install Dependencies

```bash
sudo apt-get update
sudo apt-get install -y \
    git bc bison flex libssl-dev make \
    gcc-aarch64-linux-gnu g++-aarch64-linux-gnu \
    device-tree-compiler \
    python3 python3-pip \
    qemu-user-static debootstrap \
    parted dosfstools wget rsync \
    libsdl2-dev libsdl2-ttf-dev
```

### 2. Build HackDS

```bash
cd HackDS
./build-all.sh
```

This will:
- Download Linux kernel and firmware
- Build kernel for Pi Zero 2W
- Build system components (init, game loader, menu)
- Create bootable SD card image

Build time: 30-60 minutes (depending on your machine)

### 3. Flash to SD Card

```bash
# Find your SD card device (e.g., /dev/sdb)
lsblk

# Flash the image (CAREFUL: This will erase the card!)
sudo dd if=output/hackds-0.1.0.img of=/dev/sdX bs=4M status=progress
sync
```

### 4. Boot Your Pi

1. Insert SD card into Raspberry Pi Zero 2W
2. Connect HDMI display
3. Connect USB keyboard
4. Power on

You should see the HackDS menu!

## Creating Your First Game

### Python Game

1. Create a game directory:

```bash
mkdir my-game
cd my-game
```

2. Write your game in `main.py`:

```python
import pygame

pygame.init()
screen = pygame.display.set_mode((1280, 720))
clock = pygame.time.Clock()

running = True
while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        elif event.type == pygame.KEYDOWN:
            if event.key == pygame.K_ESCAPE:
                running = False

    screen.fill((0, 0, 0))
    # Your game logic here

    pygame.display.flip()
    clock.tick(60)

pygame.quit()
```

3. Create `metadata.json`:

```json
{
  "name": "My First Game",
  "version": "1.0.0",
  "author": "Your Name",
  "description": "My awesome game",
  "engine": "python",
  "entrypoint": "main.py",
  "resolution": [1280, 720],
  "requires": {
    "python_version": ">=3.7",
    "libraries": ["pygame"]
  }
}
```

4. Package your game:

```bash
python3 ../tools/hdsg-packager.py game . my-game.hdsg metadata.json
```

5. Copy to Pi:

```bash
# Mount the SD card's root partition
sudo mount /dev/sdX2 /mnt

# Copy game
sudo cp my-game.hdsg /mnt/games/

# Unmount
sudo umount /mnt
```

### C++ Game

1. Create game directory and source:

```bash
mkdir my-cpp-game
cd my-cpp-game
```

2. Write `main.cpp`:

```cpp
#include <SDL2/SDL.h>

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow(
        "My Game",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1280, 720,
        SDL_WINDOW_SHOWN
    );

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window, -1,
        SDL_RENDERER_ACCELERATED
    );

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT ||
                (event.type == SDL_KEYDOWN &&
                 event.key.keysym.sym == SDLK_ESCAPE)) {
                running = false;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Your game logic here

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
```

3. Compile for ARM64:

```bash
aarch64-linux-gnu-g++ -o game main.cpp -lSDL2
```

4. Create `metadata.json` and package:

```json
{
  "name": "My C++ Game",
  "version": "1.0.0",
  "author": "Your Name",
  "description": "My C++ game",
  "engine": "cpp",
  "entrypoint": "game",
  "resolution": [1280, 720],
  "requires": {
    "libraries": ["SDL2"]
  }
}
```

```bash
python3 ../tools/hdsg-packager.py game . my-cpp-game.hdsg metadata.json
```

## Menu Controls

- **Up/Down** or **W/S**: Navigate games
- **Enter** or **Space**: Launch selected game
- **R**: Refresh game list
- **ESC** or **Q**: Shutdown

## Example Games

Two example games are included:

### Package Examples

```bash
# Python example
cd examples/simple-game
python3 ../../tools/hdsg-packager.py game . bouncing-balls.hdsg metadata.json

# C++ example
cd examples/cpp-game
make arm64
python3 ../../tools/hdsg-packager.py game . particles.hdsg metadata.json
```

## File Formats

### .hdsg - Game Files
Complete game packages with code, assets, and metadata.

### .hdsm - Mod Files
Modifications for existing games.

### .hdss - Settings Files
System and game configuration.

### .hdsh - Hack Files
Game cheats and trainers.

See `docs/FILE_FORMATS.md` for detailed specifications.

## Troubleshooting

### Display Issues

Edit `/boot/config.txt` on the SD card:
- Change resolution: `framebuffer_width=1920` and `framebuffer_height=1080`
- Force HDMI: `hdmi_force_hotplug=1`

### Games Won't Load

- Check game is in `/games/` directory
- Verify file has `.hdsg` extension
- Press **R** in menu to refresh
- Connect via serial console for error messages

### Serial Console Debugging

Connect USB-UART adapter:
- TX (Pi) -> RX (Adapter)
- RX (Pi) -> TX (Adapter)
- GND -> GND

```bash
screen /dev/ttyUSB0 115200
```

### Performance Issues

- Reduce game resolution
- Disable VSync in game code
- Check `over_voltage` and `arm_freq` in `/boot/config.txt`

## Development Tips

1. **Test on Desktop First**: Develop games on your PC, then cross-compile for Pi
2. **Use Version Control**: Keep your games in git
3. **Start Simple**: Begin with basic examples, add features gradually
4. **Profile Performance**: Pi Zero 2W has limited resources
5. **Asset Optimization**: Compress images, use appropriate formats

## Community

- Report bugs: Create an issue in the repository
- Share games: Tag your releases with `#HackDS`
- Contribute: Pull requests welcome!

## Next Steps

- Read `docs/FILE_FORMATS.md` for format details
- Check `docs/BUILD.md` for advanced build options
- Explore the source code in `src/`
- Join the community and share your games!

Happy game development!
