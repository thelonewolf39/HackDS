#!/bin/bash
set -e

source ../build-config.sh

log_info "Creating SD card image..."

IMAGE_FILE="${OUTPUT_DIR}/${IMAGE_NAME}-${IMAGE_VERSION}.img"
BOOT_SIZE_MB=${BOOT_SIZE}
ROOT_SIZE_MB=${ROOT_SIZE}
TOTAL_SIZE_MB=$((BOOT_SIZE_MB + ROOT_SIZE_MB + 16))

# Create empty image
log_info "Creating ${TOTAL_SIZE_MB}MB image file..."
dd if=/dev/zero of="${IMAGE_FILE}" bs=1M count=${TOTAL_SIZE_MB} status=progress

# Create partition table
log_info "Creating partition table..."
parted -s "${IMAGE_FILE}" mklabel msdos
parted -s "${IMAGE_FILE}" mkpart primary fat32 4MiB ${BOOT_SIZE_MB}MiB
parted -s "${IMAGE_FILE}" mkpart primary ext4 ${BOOT_SIZE_MB}MiB 100%

# Set up loop device
log_info "Setting up loop device..."
LOOP_DEV=$(sudo losetup -f --show "${IMAGE_FILE}")

# Create partitions
sudo partprobe "${LOOP_DEV}"

# Format boot partition
log_info "Formatting boot partition (FAT32)..."
sudo mkfs.vfat -F 32 -n BOOT "${LOOP_DEV}p1"

# Format root partition
log_info "Formatting root partition (ext4)..."
sudo mkfs.ext4 -L rootfs "${LOOP_DEV}p2"

# Mount partitions
MOUNT_BOOT="/tmp/hackds-boot"
MOUNT_ROOT="/tmp/hackds-root"

mkdir -p "${MOUNT_BOOT}" "${MOUNT_ROOT}"

sudo mount "${LOOP_DEV}p1" "${MOUNT_BOOT}"
sudo mount "${LOOP_DEV}p2" "${MOUNT_ROOT}"

# Copy boot files
log_info "Copying boot files..."
sudo cp "${ROOTFS_DIR}/boot"/* "${MOUNT_BOOT}/"

# Copy firmware files
sudo cp "${PWD}/rpi-firmware/boot/start4.elf" "${MOUNT_BOOT}/"
sudo cp "${PWD}/rpi-firmware/boot/fixup4.dat" "${MOUNT_BOOT}/"

# Copy root filesystem
log_info "Copying root filesystem..."
sudo rsync -a --exclude boot "${ROOTFS_DIR}/" "${MOUNT_ROOT}/"

# Set permissions
log_info "Setting permissions..."
sudo chown -R root:root "${MOUNT_ROOT}"

# Unmount
log_info "Unmounting..."
sudo umount "${MOUNT_BOOT}"
sudo umount "${MOUNT_ROOT}"

# Cleanup loop device
sudo losetup -d "${LOOP_DEV}"

# Cleanup mount points
rmdir "${MOUNT_BOOT}" "${MOUNT_ROOT}"

log_info "Image created successfully: ${IMAGE_FILE}"
log_info "Flash to SD card with:"
log_info "  sudo dd if=${IMAGE_FILE} of=/dev/sdX bs=4M status=progress"
