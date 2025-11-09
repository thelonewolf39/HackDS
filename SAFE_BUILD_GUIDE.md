# Safe Build Guide for Windows Users

## Overview

This guide ensures you can safely build HackDS on Windows without any risk to your system.

## Prerequisites

1. **Install WSL2** (if not already installed):
   ```powershell
   # In PowerShell (Admin)
   wsl --install
   ```

2. **Install Ubuntu in WSL2:**
   ```powershell
   wsl --install -d Ubuntu
   ```

3. **Reboot Windows** after installation

## Safety Checklist

Before building, verify:

✅ You're running in WSL2 (isolated environment)
✅ Build directory is in WSL2 filesystem (`/home/user/...`)
✅ You have at least 20GB free space in WSL2
✅ You understand `dd` command risks (see below)

## Safe Build Process

### Step 1: Open WSL2

```powershell
# From Windows, open WSL2 terminal
wsl
```

You should see a Linux prompt like:
```
user@DESKTOP-XXX:~$
```

### Step 2: Clone HackDS (Inside WSL2)

```bash
# Navigate to home directory
cd ~

# Clone repository
git clone https://github.com/yourusername/HackDS.git
cd HackDS

# Verify you're in WSL2
pwd
# Should show: /home/yourusername/HackDS
```

**Important:** This path is INSIDE WSL2, completely separate from Windows!

### Step 3: Review Build Scripts (Optional but Recommended)

```bash
# See what the build script does
cat build-all.sh

# Check if any suspicious commands
grep -n "rm -rf" build-all.sh
grep -n "dd" build-all.sh
```

Look for:
- ❌ Any commands that target `/mnt/c/...` (Windows drives)
- ❌ Any `dd` commands with hardcoded devices
- ✅ Commands should only affect local directories

### Step 4: Safe Build (No Sudo Required for Most Steps)

```bash
# Install dependencies (needs sudo, but safe)
sudo apt-get update
sudo apt-get install -y \
    git bc bison flex libssl-dev make \
    gcc-aarch64-linux-gnu g++-aarch64-linux-gnu \
    device-tree-compiler python3 python3-pip \
    qemu-user-static debootstrap parted dosfstools

# Build the system (NO SUDO NEEDED)
# Comment out the image creation step first
```

### Step 5: Modified Build Script (Extra Safe)

Create a safe version of the build script:

```bash
# Copy original
cp build-all.sh build-safe.sh

# Edit the safe version
nano build-safe.sh
```

Comment out the image creation line:
```bash
# Step 7: Generate SD card image
log_info "Generating SD card image..."
# ./scripts/build-image.sh    # <-- COMMENT THIS OUT
```

Save and run:
```bash
chmod +x build-safe.sh
./build-safe.sh
```

This builds everything EXCEPT the SD card image (which requires sudo and loop devices).

### Step 6: Create Image Separately (When Ready)

Only when you're ready to create the SD card image:

```bash
# Review the image creation script FIRST
cat scripts/build-image.sh

# Make sure it only uses loop devices, not real devices
grep "dd if=" scripts/build-image.sh
```

Should show something like:
```bash
dd if=/dev/zero of="${IMAGE_FILE}" ...
```

NOT:
```bash
dd if=something of=/dev/sda    # ← This would be BAD
```

Then run:
```bash
sudo ./scripts/build-image.sh
```

## Understanding the Risks

### What's Safe ✅

- Building in WSL2 home directory (`/home/user/...`)
- Installing packages with `apt-get`
- Compiling code with `make`
- Creating files in `build/`, `rootfs/`, `output/`
- Running Python/bash scripts in WSL2

### What's Risky ⚠️

- **`dd` commands to block devices:**
  ```bash
  # DANGEROUS if device is wrong
  sudo dd if=image.img of=/dev/sdX
  ```

- **Operating on `/mnt/c/...` paths:**
  ```bash
  # This accesses Windows filesystem
  rm -rf /mnt/c/...  # ← DON'T DO THIS
  ```

### What's VERY Risky ❌

- Running `dd` on `/dev/sda` or `/dev/nvme0n1` (main system drives)
- Deleting from `/mnt/c/Windows` or `/mnt/c/Program Files`
- Running unknown scripts as sudo without reviewing

## Flashing SD Card Safely

When you're ready to flash:

### Step 1: Identify SD Card

```bash
# BEFORE inserting SD card
lsblk

# INSERT SD card

# AFTER inserting SD card
lsblk
```

Look for the NEW device (usually `/dev/sdX` or `/dev/mmcblk0`).

**Example output:**
```
NAME        SIZE TYPE MOUNTPOINT
sda         1TB  disk            ← Your WSL2 virtual disk (DON'T TOUCH!)
└─sda1      1TB  part /
sdb         8GB  disk            ← This is your SD card!
└─sdb1      8GB  part
```

### Step 2: Unmount SD Card (If Mounted)

```bash
sudo umount /dev/sdb1  # Replace with your device
```

### Step 3: Flash Carefully

```bash
# Triple-check the device name!
echo "I am about to write to /dev/sdb"
echo "Is this correct? (yes/no)"
read answer

if [ "$answer" = "yes" ]; then
    sudo dd if=output/hackds-0.1.0.img of=/dev/sdb bs=4M status=progress
    sync
else
    echo "Cancelled. Check device name and try again."
fi
```

### Step 4: Verify

```bash
# Check if image was written
sudo fdisk -l /dev/sdb

# You should see two partitions:
# /dev/sdb1 - Boot partition (FAT32)
# /dev/sdb2 - Root partition (ext4)
```

## What Files Are Created

All files stay in WSL2:

```
/home/yourusername/HackDS/
├── build/           ← Temporary build files (~5GB)
│   ├── kernel/      ← Compiled kernel
│   ├── rootfs/      ← Root filesystem
│   └── python/      ← Python build
├── rootfs/          ← Final filesystem (~2GB)
├── output/          ← SD card image (~2GB)
├── kernel/          ← Kernel source (~1GB)
├── rpi-firmware/    ← Pi firmware (~200MB)
└── rpi-tools/       ← Build tools (~500MB)
```

**Total space needed:** ~15-20GB in WSL2

**Windows impact:** ZERO (unless you run out of disk space)

## Accessing Files from Windows

WSL2 files are accessible from Windows:

```
\\wsl$\Ubuntu\home\yourusername\HackDS\
```

You can open this in Windows Explorer to see the built files!

## Rollback / Cleanup

If something goes wrong:

```bash
# Delete build artifacts (safe)
rm -rf build/ output/ rootfs/

# Delete downloaded sources (safe)
rm -rf kernel/ rpi-firmware/ rpi-tools/

# Start over
git pull
./build-all.sh
```

## Emergency Stop

If build is running and you want to stop:

```bash
# Press Ctrl+C in terminal

# If stuck, kill all make processes
killall make

# Clean up
make -C src clean
```

## Verifying WSL2 Isolation

Test that Windows is protected:

```bash
# This is your WSL2 home (safe)
echo $HOME
# Output: /home/yourusername

# This is where Windows C: drive is mounted
ls /mnt/c/
# Output: Users/ Windows/ Program Files/ ...

# Build should NEVER touch /mnt/c/
grep -r "/mnt/c" build-all.sh scripts/
# Should return NOTHING
```

## Common Questions

### Q: Will this slow down my Windows PC?

A: The build process is CPU-intensive, but:
- Runs in WSL2 (isolated)
- Can limit cores: `make -j2` instead of `make -j$(nproc)`
- Close other programs during build
- Takes 30-60 minutes total

### Q: Can I pause the build?

A: Yes!
- Press `Ctrl+Z` to pause
- Run `fg` to resume
- Or just close terminal and restart later

### Q: What if I run out of space?

A: WSL2 uses Windows disk space:
- Check space: `df -h`
- Free space: Delete `build/` and `kernel/` directories
- Expand WSL2 disk: Google "WSL2 resize disk"

### Q: Can I build on Windows directly?

A: No, bash scripts require Linux. Options:
1. Use WSL2 (recommended, safest)
2. Use Docker Desktop for Windows
3. Use a Linux VM (VirtualBox)
4. Use a dedicated Linux machine

## Summary: Is It Safe?

**YES**, if you:
✅ Build in WSL2 (isolated environment)
✅ Review scripts before running
✅ Don't use `sudo` unnecessarily
✅ Double-check device names before `dd`
✅ Keep builds in `/home/...` not `/mnt/c/...`

**NO**, if you:
❌ Run `dd` on wrong device
❌ Modify scripts to target Windows paths
❌ Run random commands as sudo without understanding

## Final Checklist

Before running `build-all.sh`:

- [ ] Running in WSL2 (not PowerShell or CMD)
- [ ] In directory `/home/username/HackDS/`
- [ ] Reviewed build scripts
- [ ] Have 20GB free space
- [ ] No `/mnt/c/` references in scripts
- [ ] Understand `dd` command risks
- [ ] SD card is backed up (if already in use)

If all checked, you're good to go!

```bash
./build-all.sh
```

---

**Remember:** WSL2 provides strong isolation. Your Windows system is safe as long as you stay within WSL2 and don't explicitly target Windows paths!
