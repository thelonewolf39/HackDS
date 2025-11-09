#!/bin/bash
set -e

source ../build-config.sh

log_info "Creating root filesystem structure..."

# Create directory structure
mkdir -p "${ROOTFS_DIR}"/{boot,dev,proc,sys,tmp,run}
mkdir -p "${ROOTFS_DIR}"/{games,mods,settings,hacks}
mkdir -p "${ROOTFS_DIR}"/system/{bin,lib,etc,share/fonts}
mkdir -p "${ROOTFS_DIR}"/usr/{bin,lib}
mkdir -p "${ROOTFS_DIR}"/var/{log,tmp}

# Set permissions
chmod 1777 "${ROOTFS_DIR}/tmp"
chmod 1777 "${ROOTFS_DIR}/var/tmp"
chmod 755 "${ROOTFS_DIR}/games"
chmod 755 "${ROOTFS_DIR}/mods"
chmod 755 "${ROOTFS_DIR}/settings"
chmod 755 "${ROOTFS_DIR}/hacks"

# Create essential device nodes
log_info "Creating device nodes..."
sudo mknod -m 666 "${ROOTFS_DIR}/dev/null" c 1 3 || true
sudo mknod -m 666 "${ROOTFS_DIR}/dev/zero" c 1 5 || true
sudo mknod -m 666 "${ROOTFS_DIR}/dev/random" c 1 8 || true
sudo mknod -m 666 "${ROOTFS_DIR}/dev/urandom" c 1 9 || true
sudo mknod -m 666 "${ROOTFS_DIR}/dev/tty" c 5 0 || true
sudo mknod -m 666 "${ROOTFS_DIR}/dev/console" c 5 1 || true

# Create /etc files
log_info "Creating system configuration files..."

# /etc/hostname
echo "hackds" > "${ROOTFS_DIR}/system/etc/hostname"

# /etc/hosts
cat > "${ROOTFS_DIR}/system/etc/hosts" << EOF
127.0.0.1   localhost
127.0.1.1   hackds

::1         localhost ip6-localhost ip6-loopback
ff02::1     ip6-allnodes
ff02::2     ip6-allrouters
EOF

# /etc/fstab
cat > "${ROOTFS_DIR}/system/etc/fstab" << EOF
# <file system>  <mount point>  <type>  <options>         <dump> <pass>
proc             /proc          proc    defaults          0      0
sysfs            /sys           sysfs   defaults          0      0
devtmpfs         /dev           devtmpfs mode=0755       0      0
tmpfs            /tmp           tmpfs   mode=1777        0      0
tmpfs            /run           tmpfs   mode=0755        0      0
/dev/mmcblk0p1   /boot          vfat    defaults          0      2
/dev/mmcblk0p2   /              ext4    defaults,noatime  0      1
EOF

# /etc/profile
cat > "${ROOTFS_DIR}/system/etc/profile" << 'EOF'
export PATH=/system/bin:/usr/bin:/bin
export LD_LIBRARY_PATH=/system/lib:/usr/lib:/lib
export HOME=/
export TERM=linux
export HACKDS_VERSION=0.1.0

# Game development paths
export PYTHONPATH=/system/lib/python3.11
export SDL_VIDEODRIVER=kmsdrm

# Set prompt
export PS1='\u@\h:\w\$ '
EOF

# Create README in games directory
cat > "${ROOTFS_DIR}/games/README.txt" << EOF
HackDS Games Directory

Place your .hdsg game files here.

To add games:
1. Copy .hdsg files to this directory
2. Press 'R' in the menu to refresh

Example:
  cp my-game.hdsg /games/

For more information, visit the HackDS documentation.
EOF

# Create example settings file
cat > "${ROOTFS_DIR}/settings/system.hdss.example" << 'EOF'
{
  "graphics": {
    "resolution": [1280, 720],
    "fullscreen": true,
    "vsync": true
  },
  "audio": {
    "master_volume": 0.8,
    "enabled": true
  },
  "menu": {
    "auto_scan": true,
    "sort_by": "name"
  }
}
EOF

# Copy system libraries (you would need to actually cross-compile these)
log_info "Note: You need to install SDL2, zlib, and other libraries for ARM64"
log_info "These should be copied from your cross-compilation toolchain or built separately"

# Install a basic font (you'd need to download this)
log_info "Note: Download a TrueType font and place it at:"
log_info "  ${ROOTFS_DIR}/system/share/fonts/default.ttf"

# Create inittab for init system
cat > "${ROOTFS_DIR}/system/etc/inittab" << EOF
# HackDS init configuration
# This file is read by hackds-init

# Default runlevel
id:1:initdefault:

# System initialization
si::sysinit:/system/bin/hackds-menu

# Spawn menu on tty1
1:1:respawn:/system/bin/hackds-menu

# Shutdown sequence
::shutdown:/bin/umount -a -r
EOF

log_info "Root filesystem structure created"
log_info "Next steps:"
log_info "  1. Install cross-compiled libraries to ${ROOTFS_DIR}/system/lib"
log_info "  2. Install Python and PyGame"
log_info "  3. Install a TrueType font"
log_info "  4. Run build-image.sh to create SD card image"
