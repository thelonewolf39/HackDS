#!/bin/bash
set -e

# Load configuration
source build-config.sh

log_info "HackDS Build System v${IMAGE_VERSION}"
log_info "Building for ${BOARD} (${ARCH})"

# Create directory structure
log_info "Creating build directories..."
mkdir -p "${BUILD_DIR}"/{kernel,rootfs,tools}
mkdir -p "${ROOTFS_DIR}"/{boot,dev,proc,sys,tmp,run}
mkdir -p "${ROOTFS_DIR}"/{games,mods,settings,hacks}
mkdir -p "${ROOTFS_DIR}"/system/{bin,lib,etc,share}
mkdir -p "${OUTPUT_DIR}"

# Step 1: Download dependencies
if [ ! -d "kernel" ]; then
    log_info "Cloning Linux kernel..."
    git clone --depth=1 --branch ${KERNEL_BRANCH} \
        https://github.com/raspberrypi/linux.git kernel
fi

if [ ! -d "rpi-firmware" ]; then
    log_info "Cloning Raspberry Pi firmware..."
    git clone --depth=1 \
        https://github.com/raspberrypi/firmware.git rpi-firmware
fi

if [ ! -d "rpi-tools" ]; then
    log_info "Cloning Raspberry Pi tools..."
    git clone --depth=1 \
        https://github.com/raspberrypi/tools.git rpi-tools
fi

# Step 2: Build kernel
log_info "Building Linux kernel..."
cd kernel

if [ ! -f .config ]; then
    log_info "Configuring kernel..."
    make bcm2711_defconfig

    # Apply HackDS-specific configurations
    scripts/config --disable CONFIG_WIRELESS
    scripts/config --disable CONFIG_BT
    scripts/config --enable CONFIG_DRM
    scripts/config --enable CONFIG_DRM_VC4
    scripts/config --disable CONFIG_MODULES
    scripts/config --enable CONFIG_EMBEDDED
fi

log_info "Compiling kernel (this may take a while)..."
make -j${PARALLEL_JOBS} Image.gz dtbs

# Copy kernel to boot
cp arch/arm64/boot/Image.gz "${ROOTFS_DIR}/boot/kernel8.img"
cp arch/arm64/boot/dts/broadcom/bcm2710-rpi-zero-2-w.dtb "${ROOTFS_DIR}/boot/"

cd ..

# Step 3: Build system components
log_info "Building HackDS system components..."
make -C src all
make -C src DESTDIR="${ROOTFS_DIR}" install

# Step 4: Create boot configuration
log_info "Creating boot configuration..."
./scripts/create-boot-config.sh

# Step 5: Build Python minimal
log_info "Building Python ${PYTHON_VERSION}..."
./scripts/build-python.sh

# Step 6: Create filesystem structure
log_info "Creating filesystem structure..."
./scripts/create-rootfs.sh

# Step 7: Generate SD card image
log_info "Generating SD card image..."
./scripts/build-image.sh

log_info "Build complete! Image: ${OUTPUT_DIR}/${IMAGE_NAME}-${IMAGE_VERSION}.img"
log_info "Flash with: sudo dd if=${OUTPUT_DIR}/${IMAGE_NAME}-${IMAGE_VERSION}.img of=/dev/sdX bs=4M status=progress"
