# HackDS Build Container
# Builds HackDS OS for Raspberry Pi Zero 2W

FROM ubuntu:22.04

# Prevent interactive prompts
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=UTC

# Install build dependencies
RUN apt-get update && apt-get install -y \
    git bc bison flex libssl-dev make \
    gcc-aarch64-linux-gnu g++-aarch64-linux-gnu \
    device-tree-compiler \
    python3 python3-pip \
    qemu-user-static debootstrap \
    parted dosfstools wget rsync \
    libsdl2-dev libsdl2-ttf-device-tree-compiler \
    kpartx fdisk \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /hackds

# Build configuration
ENV ARCH=arm64
ENV CROSS_COMPILE=aarch64-linux-gnu-

# Default command
CMD ["/bin/bash"]
