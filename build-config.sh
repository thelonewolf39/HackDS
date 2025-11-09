#!/bin/bash
# HackDS Build Configuration

# Target architecture
export ARCH=arm64
export CROSS_COMPILE=aarch64-linux-gnu-
export TARGET=aarch64-linux-gnu

# Raspberry Pi Zero 2W specific
export BOARD=pi0-2w
export SOC=bcm2710

# Partition sizes (MB)
export BOOT_SIZE=128
export ROOT_SIZE=2048

# Kernel configuration
export KERNEL_VERSION=6.1
export KERNEL_BRANCH=rpi-6.1.y
export ENABLE_DEBUG=0
export ENABLE_MODULES=1

# Bootloader
export BOOTLOADER=uboot  # or 'firmware' for standard RPi boot

# Graphics backend
export GRAPHICS_BACKEND=drm  # Direct Rendering Manager
export ENABLE_VC4=1          # VideoCore IV driver

# Python configuration
export PYTHON_VERSION=3.11
export PYTHON_MINIMAL=1      # Minimal Python build

# C/C++ runtime
export ENABLE_CPP=1
export CPP_STDLIB=libstdc++

# System libraries
export ENABLE_SDL2=1
export ENABLE_OPENGL=1
export ENABLE_ALSA=1

# Build options
export PARALLEL_JOBS=$(nproc)
export VERBOSE=0

# Directories
export BUILD_DIR="${PWD}/build"
export ROOTFS_DIR="${PWD}/rootfs"
export OUTPUT_DIR="${PWD}/output"
export TOOLS_DIR="${PWD}/tools"

# Image name
export IMAGE_NAME="hackds"
export IMAGE_VERSION="0.1.0"

# Colors for output
export COLOR_RED='\033[0;31m'
export COLOR_GREEN='\033[0;32m'
export COLOR_YELLOW='\033[1;33m'
export COLOR_NC='\033[0m'

# Helper functions
log_info() {
    echo -e "${COLOR_GREEN}[INFO]${COLOR_NC} $1"
}

log_warn() {
    echo -e "${COLOR_YELLOW}[WARN]${COLOR_NC} $1"
}

log_error() {
    echo -e "${COLOR_RED}[ERROR]${COLOR_NC} $1"
}
