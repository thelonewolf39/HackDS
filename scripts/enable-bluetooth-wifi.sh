#!/bin/bash
#
# Enable Bluetooth and WiFi support in kernel configuration
#

set -e

cd "$(dirname "$0")/.."

if [ ! -d "kernel" ]; then
    echo "Error: Kernel source not found. Run build-all.sh first."
    exit 1
fi

cd kernel

export ARCH=arm64
export CROSS_COMPILE=aarch64-linux-gnu-

echo "Enabling Bluetooth and WiFi support in kernel..."

# Enable Bluetooth
scripts/config --module CONFIG_BT
scripts/config --module CONFIG_BT_BREDR
scripts/config --module CONFIG_BT_LE
scripts/config --module CONFIG_BT_RFCOMM
scripts/config --module CONFIG_BT_BNEP
scripts/config --module CONFIG_BT_HIDP
scripts/config --module CONFIG_BT_HCIBTUSB
scripts/config --module CONFIG_BT_HCIUART
scripts/config --module CONFIG_BT_HCIUART_BCM
scripts/config --module CONFIG_BT_HCIBCM203X
scripts/config --module CONFIG_BT_BCM

# Enable HID for controllers
scripts/config --enable CONFIG_HID
scripts/config --enable CONFIG_HID_GENERIC
scripts/config --module CONFIG_HID_SONY  # For PS5 DualSense
scripts/config --module CONFIG_SONY_FF   # Force feedback
scripts/config --module CONFIG_HID_PLAYSTATION  # PS5 specific
scripts/config --module CONFIG_HID_MICROSOFT  # Xbox controllers
scripts/config --module CONFIG_HID_NINTENDO  # Switch controllers

# Enable WiFi
scripts/config --module CONFIG_CFG80211
scripts/config --module CONFIG_MAC80211
scripts/config --module CONFIG_BRCMFMAC  # Broadcom WiFi (Pi Zero 2W)
scripts/config --module CONFIG_BRCMUTIL

# Enable wireless extensions
scripts/config --enable CONFIG_WIRELESS_EXT
scripts/config --enable CONFIG_WEXT_CORE
scripts/config --enable CONFIG_WEXT_PROC
scripts/config --enable CONFIG_WEXT_SPY

# Enable regulatory database
scripts/config --module CONFIG_CFG80211_WEXT

# Enable input devices
scripts/config --enable CONFIG_INPUT
scripts/config --enable CONFIG_INPUT_EVDEV
scripts/config --enable CONFIG_INPUT_JOYSTICK
scripts/config --enable CONFIG_INPUT_JOYDEV

# Enable USB HID
scripts/config --enable CONFIG_USB_HID
scripts/config --module CONFIG_USB_HIDDEV

echo ""
echo "Bluetooth and WiFi support enabled!"
echo ""
echo "Kernel configuration updated with:"
echo "  ✓ Bluetooth (including BCM support for Pi)"
echo "  ✓ WiFi (Broadcom FMAC for Pi Zero 2W)"
echo "  ✓ PS5 DualSense controller support"
echo "  ✓ Xbox, Nintendo controller support"
echo "  ✓ HID and input device support"
echo ""
echo "Next steps:"
echo "  1. Rebuild kernel: make -j\$(nproc) Image.gz dtbs"
echo "  2. Install modules: make INSTALL_MOD_PATH=../rootfs modules_install"
echo ""
