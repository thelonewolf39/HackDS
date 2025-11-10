# Build HackDS on Windows (No WSL Required!)

## Easiest Method: Docker Desktop

**Time:** 5 minutes setup + 30-60 minutes build
**Difficulty:** ⭐ Easy (just click and run)
**Requirements:** Windows 10/11, 20GB disk space

---

## Step-by-Step Instructions

### 1. Install Docker Desktop (One-Time Setup)

1. **Download Docker Desktop:**
   - Go to: https://www.docker.com/products/docker-desktop
   - Click "Download for Windows"
   - Run the installer

2. **Install Docker:**
   - Accept defaults
   - Restart computer when prompted

3. **Start Docker Desktop:**
   - Open Docker Desktop from Start Menu
   - Wait for "Docker Desktop is running" message
   - You'll see a whale icon in system tray

**That's it! Docker is ready.**

---

### 2. Get HackDS Code

**Option A: Download ZIP** (Easier)
1. Go to your HackDS repository on GitHub
2. Click green "Code" button
3. Click "Download ZIP"
4. Extract to `C:\HackDS` or any folder

**Option B: Use Git**
```cmd
git clone https://github.com/yourusername/HackDS.git
cd HackDS
```

---

### 3. Build HackDS (One Command!)

1. **Open Command Prompt in HackDS folder:**
   - Open File Explorer
   - Navigate to HackDS folder
   - Type `cmd` in address bar and press Enter

2. **Run the build script:**
   ```cmd
   docker-build.bat
   ```

3. **Wait for build to complete:**
   - First time: Downloads Linux (~500MB)
   - Builds kernel (~20 minutes)
   - Builds system (~10 minutes)
   - Creates image (~5 minutes)
   - **Total: 30-60 minutes**

4. **Done!** Output image will be in `output\hackds-0.1.0.img`

---

### 4. Flash to SD Card

**Method 1: balenaEtcher (Recommended)**

1. Download: https://etcher.balena.io/
2. Install and open balenaEtcher
3. Click "Flash from file"
4. Select `output\hackds-0.1.0.img`
5. Click "Select target"
6. Choose your SD card (be careful!)
7. Click "Flash!"
8. Wait 5-10 minutes
9. Done!

**Method 2: Rufus**

1. Download: https://rufus.ie/
2. Open Rufus
3. Select your SD card in "Device"
4. Click "SELECT" and choose `output\hackds-0.1.0.img`
5. Click "START"
6. Click "OK" on any warnings
7. Wait for completion
8. Done!

---

## What the Build Does

The `docker-build.bat` script:

1. ✅ **Checks Docker is installed**
2. ✅ **Creates a Linux container** (Ubuntu)
3. ✅ **Downloads build tools** automatically
4. ✅ **Downloads Linux kernel** for Raspberry Pi
5. ✅ **Compiles everything** (kernel, system, games)
6. ✅ **Creates SD card image** (ready to flash)
7. ✅ **Outputs to** `output\hackds-0.1.0.img`

All of this happens **inside a container** - your Windows system stays clean!

---

## Troubleshooting

### "Docker is not installed or not running"

**Solution:**
1. Make sure Docker Desktop is running
2. Look for whale icon in system tray
3. If not running, open Docker Desktop from Start Menu
4. Wait for "Docker Desktop is running" message

### "Error: Failed to build Docker image"

**Solution:**
1. Check internet connection (needs to download Ubuntu)
2. Make sure Docker Desktop is running
3. Try again - first build downloads a lot

### Build is slow / computer freezes

**Solution:**
Docker uses a lot of CPU. You can:
1. Close other programs
2. Limit CPU cores:
   - Open Docker Desktop
   - Settings → Resources
   - Reduce CPUs to 2-4

### "Out of space" error

**Solution:**
1. Check disk space (need 20GB free)
2. Clean Docker:
   ```cmd
   docker system prune -a
   ```
3. Try build again

### Build fails partway through

**Solution:**
1. Check error message in console
2. Most common: Network timeout
3. Just run `docker-build.bat` again - it will resume!

---

## Advanced: Manual Docker Commands

If you want more control:

```cmd
REM Build the Docker image
docker build -t hackds-builder .

REM Run interactive shell in container
docker run -it --privileged -v "%cd%:/hackds" hackds-builder /bin/bash

REM Inside container, build manually:
cd /hackds
./build-all.sh
```

---

## File Locations

After build completes:

```
HackDS\
├── output\
│   └── hackds-0.1.0.img    ← Flash this to SD card!
├── build\                   ← Temporary build files
├── rootfs\                  ← Root filesystem (for Pi)
├── kernel\                  ← Downloaded kernel source
└── rpi-firmware\            ← Downloaded firmware
```

The important file is: **`output\hackds-0.1.0.img`**

---

## Rebuilding

To build again (after making changes):

```cmd
REM Just run the script again
docker-build.bat
```

It will reuse cached files - much faster!

---

## Cleaning Up

To free disk space after building:

```cmd
REM Delete build files (keep image)
rmdir /s /q build
rmdir /s /q rootfs
rmdir /s /q kernel
rmdir /s /q rpi-firmware

REM Delete Docker cache (frees ~2GB)
docker system prune -a

REM Keep only output\hackds-0.1.0.img
```

---

## Why Docker Desktop?

✅ **No WSL needed** - Runs natively on Windows
✅ **Self-contained** - Doesn't mess with your system
✅ **One command** - Just run `docker-build.bat`
✅ **Repeatable** - Same build every time
✅ **Easy cleanup** - Just delete container

---

## System Requirements

**Minimum:**
- Windows 10 64-bit (version 1903 or higher)
- 8GB RAM
- 20GB free disk space
- Internet connection (first build only)

**Recommended:**
- Windows 11
- 16GB RAM
- SSD with 30GB+ free space
- Fast internet connection

---

## Alternative: Pre-Built Images

If building is too slow, you can also:

1. **Download pre-built image** from GitHub Releases
2. **Flash directly** to SD card
3. **Skip the build** entirely!

Check Releases page: `https://github.com/yourusername/HackDS/releases`

---

## Quick Reference Card

### First Time Setup
```
1. Install Docker Desktop
2. Download HackDS code
3. Run: docker-build.bat
4. Wait 30-60 minutes
5. Flash output\hackds-0.1.0.img to SD card
```

### Rebuilding
```
1. Make changes to code
2. Run: docker-build.bat
3. Wait (much faster with cache)
4. Flash new image
```

### Cleanup
```
docker system prune -a
rmdir /s /q build rootfs kernel
```

---

## Need Help?

1. **Check error message** in console
2. **Read troubleshooting** section above
3. **Search GitHub issues**
4. **Ask in discussions**

Most issues are:
- Docker not running
- Not enough disk space
- Network timeout (just retry)

---

## Summary

**Easiest Windows build method:**

1. Install Docker Desktop (5 minutes)
2. Run `docker-build.bat` (60 minutes)
3. Flash with balenaEtcher (10 minutes)
4. Boot your Raspberry Pi! 🎉

**Total time: ~75 minutes (mostly waiting)**

No Linux knowledge required!
No WSL required!
Just Docker + one command!

---

## What's Next?

After building and flashing:

1. Insert SD card into Raspberry Pi Zero 2W
2. Connect HDMI display
3. Power on
4. See HackDS menu!
5. Connect WiFi: `wifi-manager connect "Network" "pass"`
6. Pair PS5 controller: `bluetooth-manager ps5-setup`
7. Play games! 🎮

See `QUICKSTART.md` for post-install setup.

---

**Happy building!** 🚀
