#!/bin/bash
set -e

source ../build-config.sh

log_info "Building Python ${PYTHON_VERSION} for HackDS..."

PYTHON_DIR="${BUILD_DIR}/python"
PYTHON_SRC="Python-${PYTHON_VERSION}"
PYTHON_URL="https://www.python.org/ftp/python/${PYTHON_VERSION}/${PYTHON_SRC}.tar.xz"

mkdir -p "${PYTHON_DIR}"
cd "${PYTHON_DIR}"

# Download Python if not already downloaded
if [ ! -f "${PYTHON_SRC}.tar.xz" ]; then
    log_info "Downloading Python ${PYTHON_VERSION}..."
    wget "${PYTHON_URL}"
fi

# Extract if not already extracted
if [ ! -d "${PYTHON_SRC}" ]; then
    log_info "Extracting Python..."
    tar xf "${PYTHON_SRC}.tar.xz"
fi

cd "${PYTHON_SRC}"

# Configure for cross-compilation
if [ ! -f Makefile ]; then
    log_info "Configuring Python..."

    # First build for host (needed for cross-compilation)
    mkdir -p build-host
    cd build-host
    ../configure
    make -j${PARALLEL_JOBS}
    cd ..

    # Now configure for target
    mkdir -p build-target
    cd build-target

    ../configure \
        --host=${TARGET} \
        --build=x86_64-linux-gnu \
        --prefix=/system \
        --enable-optimizations \
        --with-ensurepip=no \
        --without-doc-strings \
        --without-pymalloc \
        --disable-ipv6 \
        ac_cv_file__dev_ptmx=yes \
        ac_cv_file__dev_ptc=no

    cd ..
fi

# Build
cd build-target
log_info "Building Python (this will take a while)..."
make -j${PARALLEL_JOBS}

# Install to rootfs
log_info "Installing Python to rootfs..."
make DESTDIR="${ROOTFS_DIR}" install

# Install PyGame
log_info "Installing PyGame..."
"${ROOTFS_DIR}/system/bin/python3" -m pip install --target="${ROOTFS_DIR}/system/lib/python3.11/site-packages" pygame

log_info "Python installation complete"
