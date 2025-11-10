# Build HackDS on Windows - Super Easy Method

## TL;DR

1. Install Docker Desktop
2. Double-click `docker-build-simple.bat`
3. Wait 30-60 minutes
4. Flash `output\hackds-0.1.0.img` to SD card
5. Done!

---

## Step 1: Install Docker Desktop (One Time)

### Download and Install

1. Go to: **https://www.docker.com/products/docker-desktop**
2. Click **"Download for Windows"**
3. Run the installer
4. Accept all defaults
5. Restart computer when prompted
6. Open Docker Desktop from Start Menu
7. Wait for **"Docker Desktop is running"** message

**Done!** Docker icon should appear in system tray.

---

## Step 2: Build HackDS

### Download HackDS

1. Download ZIP from your GitHub repository
2. Extract to any folder (e.g., `C:\HackDS`)

### Run the Builder

1. Open the `HackDS` folder
2. **Double-click `docker-build-simple.bat`**
3. Press any key to start build
4. **Wait 30-60 minutes** (first time)
5. Window will show "BUILD COMPLETE!" when done

**That's it!** The image is ready.

---

## Step 3: Flash to SD Card

### Download balenaEtcher

1. Go to: **https://etcher.balena.io/**
2. Download and install

### Flash the Image

1. Insert SD card (8GB+) into computer
2. Open balenaEtcher
3. Click **"Flash from file"**
4. Select `HackDS\output\hackds-0.1.0.img`
5. Click **"Select target"** → Choose your SD card
6. Click **"Flash!"**
7. Wait 5-10 minutes
8. Done!

---

## Step 4: Boot HackDS

1. Remove SD card from computer
2. Insert into Raspberry Pi Zero 2W
3. Connect HDMI to display
4. Connect power
5. **HackDS boots!** 🎉

---

## Troubleshooting

### Docker Desktop won't start

**Error:** "WSL 2 installation is incomplete"

**Fix:**
```powershell
# Run in PowerShell (Admin)
wsl --install --no-distribution
```
Restart computer.

**Or:** In Docker Desktop settings:
- Uncheck "Use WSL 2 based engine"
- Click "Apply & Restart"

---

### Build script says "Docker not found"

**Fix:**
1. Make sure Docker Desktop is running
2. Look for whale icon in system tray
3. If not there, open Docker Desktop
4. Wait for "running" message
5. Try script again

---

### Build fails with "no space left"

**Fix:**
1. Delete old Docker images:
   ```cmd
   docker system prune -a
   ```
2. Free up disk space (need 20GB)
3. Try again

---

### Build is very slow

**Fix:**
1. Close other programs
2. First build downloads a lot - be patient
3. Subsequent builds are much faster (cached)

**Note:** It's normal to take 30-60 minutes first time!

---

## What Gets Built

The script builds:

✅ Linux kernel for Raspberry Pi Zero 2W
✅ HackDS system (init, menu, settings)
✅ WiFi & Bluetooth managers
✅ Game loader
✅ Example games
✅ Bootable SD card image

All in one `.img` file ready to flash!

---

## After Building

Your `HackDS` folder will have:

```
HackDS\
├── output\
│   └── hackds-0.1.0.img   ← This is what you flash!
├── docker-build-simple.bat ← Double-click to build
├── Dockerfile              ← Build configuration
└── ... (source code)
```

**The important file:** `output\hackds-0.1.0.img`

---

## Rebuilding After Changes

1. Make changes to code
2. Double-click `docker-build-simple.bat` again
3. Much faster (uses cache)
4. Flash new image to SD card

---

## Cleaning Up

To free disk space:

```cmd
REM Clean Docker cache
docker system prune -a

REM Delete build files (optional)
rmdir /s /q build
rmdir /s /q rootfs
rmdir /s /q kernel
```

---

## No Docker? Alternative Methods

If you can't install Docker:

### Option 1: GitHub Actions (Coming Soon)
- Push code to GitHub
- GitHub builds automatically
- Download pre-built image

### Option 2: Virtual Machine
- Install VirtualBox
- Create Ubuntu VM
- Follow Linux build instructions

### Option 3: Pre-Built Images
- Download from Releases page
- Skip building entirely
- Just flash and go!

---

## System Requirements

**Minimum:**
- Windows 10 64-bit (1903+)
- 8GB RAM
- 20GB free space
- Internet connection

**Recommended:**
- Windows 11
- 16GB RAM
- SSD with 30GB free
- Fast internet

---

## Why This Method?

✅ **No WSL** - Pure Windows solution
✅ **No Linux knowledge** needed
✅ **One click** - Double-click and wait
✅ **Safe** - Doesn't touch your Windows system
✅ **Clean** - Everything in container
✅ **Repeatable** - Same result every time

---

## Common Questions

**Q: Do I need WSL?**
A: No! Docker can use Hyper-V instead.

**Q: Will this slow down my PC?**
A: During build, yes. After build, no - you can close Docker.

**Q: Can I pause the build?**
A: Not easily. Let it finish or start over.

**Q: How much disk space do I need?**
A: 20GB for build, 2GB for final image.

**Q: Can I delete Docker after building?**
A: Yes! Once you have the `.img` file, you can uninstall Docker.

**Q: Do I need to rebuild for every SD card?**
A: No! Build once, flash many times.

---

## Quick Start Summary

**Total time: ~75 minutes**

| Step | Time | Action |
|------|------|--------|
| 1 | 5 min | Install Docker Desktop |
| 2 | 2 min | Download HackDS |
| 3 | 60 min | Run build script (automated) |
| 4 | 8 min | Flash with balenaEtcher |
| **Total** | **75 min** | **Most is waiting!** |

---

## Next Steps

After flashing and booting HackDS:

1. **Connect WiFi:**
   ```bash
   wifi-manager connect "YourNetwork" "password"
   ```

2. **Pair PS5 Controller:**
   ```bash
   bluetooth-manager ps5-setup
   ```

3. **Check for updates:**
   ```bash
   hackds-updater check
   ```

4. **Play games!** 🎮

See `QUICKSTART.md` for complete setup guide.

---

## Need Help?

1. Read `WINDOWS_BUILD_GUIDE.md` (detailed guide)
2. Check GitHub Issues
3. Ask in Discussions
4. Most issues = Docker not running or disk space

---

**You've got this!** Building HackDS on Windows is easy with Docker. Just install, double-click, and wait! 🚀
