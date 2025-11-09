# Building HackDS

## Prerequisites

### On Linux (Ubuntu/Debian)

```bash
sudo apt-get update
sudo apt-get install -y \
    git bc bison flex libssl-dev make \
    gcc-aarch64-linux-gnu g++-aarch64-linux-gnu \
    device-tree-compiler \
    python3 python3-pip \
    qemu-user-static debootstrap \
    parted dosfstools
```

### On Windows (via WSL2)

1. Install WSL2 with Ubuntu
2. Follow Linux prerequisites above

## Build Steps

### 1. Clone Required Repositories

```bash
# Linux kernel
git clone --depth=1 --branch rpi-6.1.y https://github.com/raspberrypi/linux.git kernel

# Raspberry Pi firmware
git clone --depth=1 https://github.com/raspberrypi/firmware.git rpi-firmware

# Build tools
git clone --depth=1 https://github.com/raspberrypi/tools.git rpi-tools
```

### 2. Configure and Build Kernel

```bash
cd kernel

# Set up cross-compilation
export ARCH=arm64
export CROSS_COMPILE=aarch64-linux-gnu-

# Use Pi Zero 2W default config as base
make bcm2711_defconfig

# Customize for HackDS (minimal, gaming-focused)
make menuconfig

# Build kernel, modules, and device trees
make -j$(nproc) Image modules dtbs

# Install modules to staging directory
mkdir -p ../rootfs/lib/modules
make INSTALL_MOD_PATH=../rootfs modules_install
```

### 3. Build Root Filesystem

```bash
# Create directory structure
cd ..
mkdir -p rootfs/{boot,dev,proc,sys,tmp,games,mods,settings,hacks}
mkdir -p rootfs/system/{bin,lib,etc}

# Build HackDS system components
cd src
make all
make DESTDIR=../rootfs install
```

### 4. Create SD Card Image

```bash
# Run the build script
./scripts/build-image.sh

# Output will be: hackds.img
```

### 5. Flash to SD Card

```bash
# Linux
sudo dd if=hackds.img of=/dev/sdX bs=4M status=progress

# Windows (use Win32DiskImager or Rufus)
# Or via WSL: sudo dd if=hackds.img of=/dev/sdX bs=4M
```

## Build Configuration

Edit `build-config.sh` to customize:

```bash
# Target architecture
ARCH=arm64
TARGET=aarch64-linux-gnu

# System size (MB)
BOOT_SIZE=128
ROOT_SIZE=1024

# Kernel options
KERNEL_VERSION=6.1
ENABLE_DEBUG=0

# Python version
PYTHON_VERSION=3.11

# Graphics backend
GRAPHICS_BACKEND=drm  # or 'fb' for framebuffer
```

## Quick Build

For a complete build from scratch:

```bash
./build-all.sh
```

This will:
1. Download all dependencies
2. Build kernel
3. Build system components
4. Create root filesystem
5. Generate SD card image

## Development Build

For faster iteration during development:

```bash
# Build only system components
make -C src clean all install

# Rebuild just the game loader
make -C src/gameloader clean all install

# Update image without rebuilding kernel
./scripts/update-rootfs.sh
```

## Testing

### QEMU Emulation

```bash
# Test the image in QEMU
./scripts/test-qemu.sh hackds.img
```

### Serial Console

Connect via UART for debugging:
```bash
screen /dev/ttyUSB0 115200
```

## Troubleshooting

### Kernel won't boot
- Check bootloader config in boot/config.txt
- Verify kernel=kernel8.img in config.txt
- Check cmdline.txt for correct root partition

### Graphics not working
- Ensure DRM/KMS drivers enabled in kernel
- Check /dev/dri/card0 exists
- Verify GPU firmware in /boot

### Python games won't run
- Verify Python libraries in rootfs/usr/lib/python3.11
- Check LD_LIBRARY_PATH includes system libraries
- Test with simple Python script first
