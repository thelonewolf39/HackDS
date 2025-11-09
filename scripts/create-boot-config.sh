#!/bin/bash
set -e

source build-config.sh

log_info "Creating boot configuration files..."

# Create config.txt
cat > "${ROOTFS_DIR}/boot/config.txt" << 'EOF'
# HackDS Boot Configuration

# Kernel
kernel=kernel8.img
arm_64bit=1

# GPU Memory (256MB for graphics)
gpu_mem=256

# Display
hdmi_force_hotplug=1
hdmi_drive=2
framebuffer_width=1280
framebuffer_height=720

# VideoCore IV (VC4) DRM driver
dtoverlay=vc4-kms-v3d
max_framebuffers=2

# Disable unused features for performance
dtparam=audio=off
camera_auto_detect=0
display_auto_detect=0

# Overclock (safe settings for Pi Zero 2W)
arm_freq=1200
gpu_freq=500
over_voltage=2

# Disable boot delay
boot_delay=0
disable_splash=1

# Enable UART for debugging (optional)
enable_uart=1
EOF

# Create cmdline.txt
cat > "${ROOTFS_DIR}/boot/cmdline.txt" << 'EOF'
console=serial0,115200 console=tty1 root=/dev/mmcblk0p2 rootfstype=ext4 elevator=deadline fsck.repair=yes rootwait quiet loglevel=1 logo.nologo vt.global_cursor_default=0
EOF

log_info "Boot configuration created"
