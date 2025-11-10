# HackDS - No Build Required! 🎉

## Easiest Way: Just Download & Flash

**Total time: 15 minutes** (no building!)

Skip all the building complexity - just download a pre-built image and flash it to your SD card.

---

## Quick Start (3 Steps)

### Step 1: Download Pre-Built Image (5 min)

**Option A: From GitHub Releases (Recommended)**

1. Go to: `https://github.com/yourusername/HackDS/releases/latest`
2. Download `hackds-vX.X.X.img.zip` (replace with actual version)
3. Extract the ZIP file to get `hackds-vX.X.X.img`

**Option B: Run Download Script**

1. Double-click `download-latest.bat`
2. Opens browser to releases page
3. Download and extract

### Step 2: Flash to SD Card (8 min)

1. **Download balenaEtcher**: https://etcher.balena.io/
2. Install and open balenaEtcher
3. Click **"Flash from file"**
4. Select the extracted `.img` file
5. Click **"Select target"** → Choose SD card
6. Click **"Flash!"**
7. Wait for completion

### Step 3: Boot (2 min)

1. Insert SD card into Raspberry Pi Zero 2W
2. Connect HDMI display
3. Connect power
4. **HackDS boots!** 🎮

---

## That's It!

No building, no Docker, no Linux, no WSL, no compiling.

Just: **Download → Flash → Boot**

---

## First Boot Setup

After HackDS boots, you'll see the menu. Connect wirelessly:

### Connect to WiFi

```bash
# Press F1 for settings, or run:
wifi-manager scan
wifi-manager connect "YourNetwork" "password"
```

### Pair PS5 Controller

```bash
# Put controller in pairing mode (PS + Share)
bluetooth-manager ps5-setup
```

### Check for Updates

```bash
# In menu, press U, or run:
hackds-updater check
```

---

## File Details

### Download Size

- **Compressed (ZIP)**: ~500MB - 1GB
- **Extracted (IMG)**: ~2GB
- **SD Card needed**: 8GB minimum

### What's Included

Pre-built image includes:

✅ Raspberry Pi Zero 2W kernel
✅ HackDS operating system
✅ GUI menu system
✅ Settings menu
✅ WiFi manager
✅ Bluetooth manager
✅ PS5 controller support
✅ Auto-update system
✅ Game loader
✅ Example games
✅ All documentation

Everything ready to go!

---

## Where Images Come From

### Automatic Builds

Every time new code is pushed to GitHub:

1. GitHub Actions automatically builds HackDS
2. Compiles kernel, system, everything
3. Creates bootable SD card image
4. Uploads to Releases page
5. **You just download the result!**

No building on your computer needed!

### Build Status

Check if latest image is ready:

1. Go to repository on GitHub
2. Click "Actions" tab
3. See latest workflow run
4. Green checkmark = image ready
5. Download from Releases

---

## Verifying Downloads

Each release includes a SHA256 checksum file.

### On Windows

```powershell
# In PowerShell, in download folder:
CertUtil -hashfile hackds-v0.1.0.img SHA256

# Compare with hackds-v0.1.0.sha256 file
```

### On Linux/Mac

```bash
sha256sum -c hackds-v0.1.0.sha256
```

Should show "OK" if file is valid.

---

## Flashing Tools

### balenaEtcher (Recommended)

**Pros:**
- Easy to use
- Works on Windows, Mac, Linux
- Verifies after flashing
- Free and open source

**Download:** https://etcher.balena.io/

### Rufus (Windows Only)

**Pros:**
- Fast
- Portable (no install)
- Advanced options

**Download:** https://rufus.ie/

### Win32DiskImager (Windows)

**Pros:**
- Simple
- Lightweight
- Classic tool

**Download:** https://sourceforge.net/projects/win32diskimager/

### dd Command (Linux/Mac)

```bash
# Find SD card device
lsblk

# Flash (be careful with device name!)
sudo dd if=hackds-v0.1.0.img of=/dev/sdX bs=4M status=progress
sync
```

---

## Versions & Updates

### Stable Releases

Tagged versions (v0.1.0, v0.2.0, etc.):
- Fully tested
- Stable
- Recommended for most users

### Development Builds

Latest from main branch:
- Cutting edge features
- May have bugs
- For testing/development

### Finding Versions

Go to: `https://github.com/yourusername/HackDS/releases`

- **Latest Release**: Green "Latest" tag
- **All Releases**: Scroll down
- **Pre-releases**: Marked with orange tag

---

## Updating HackDS

Once booted, HackDS can auto-update:

```bash
# Check for updates
hackds-updater check

# Install updates
hackds-updater update

# Or enable auto-updates
hackds-updater enable
```

No need to re-flash SD card!

---

## Troubleshooting

### Download is slow

**Solutions:**
- Use faster internet connection
- Try different time of day
- Download from GitHub release page directly
- Check if mirrors available

### ZIP file won't extract

**Solutions:**
- Windows: Use 7-Zip (free)
- Mac: Double-click should work
- Linux: `unzip hackds-*.img.zip`

### balenaEtcher says "Image too large"

**Problem:** SD card too small

**Solution:**
- Use 8GB or larger SD card
- Check SD card isn't fake/corrupted

### SD card won't boot

**Solutions:**
1. Re-flash the image
2. Try different SD card
3. Verify SHA256 checksum
4. Check Pi's power supply (need 2A)

### Can't find .img file

**Problem:** Forgot to extract ZIP

**Solution:**
- Right-click ZIP → Extract All
- Or use 7-Zip to extract
- IMG file is inside ZIP

---

## Comparison: Build vs Download

### Download Pre-Built (This Guide)

✅ **5 minutes** to download
✅ **No technical knowledge** needed
✅ **No build tools** required
✅ **No errors** or troubleshooting
✅ **Tested and verified**
✅ **Just works**

### Build Yourself (Other Guides)

⏱️ **30-60 minutes** to build
🔧 **Need Docker** or WSL
💾 **20GB disk space** for build
⚡ **High CPU usage** during build
🐛 **Possible errors** to debug
📚 **More complex**

**Recommendation:** Use pre-built images unless you're modifying the code!

---

## When to Build Yourself

You should build only if:

- You're developing HackDS features
- You've modified the source code
- You want bleeding-edge unreleased changes
- Pre-built image doesn't exist yet
- You enjoy building things!

For everyone else: **Just download the pre-built image!**

---

## Creating Your Own Releases

If you've forked HackDS and made changes:

### 1. Edit GitHub Actions

Edit `.github/workflows/build-release.yml`:
- Already configured!
- Just push code to trigger

### 2. Create a Release

```bash
# Tag your version
git tag -a v0.1.0 -m "Version 0.1.0"
git push origin v0.1.0
```

### 3. Wait for Build

1. Go to GitHub → Actions tab
2. Watch build progress (~30-45 min)
3. When done, image appears in Releases
4. Download and share!

---

## Bandwidth Considerations

### GitHub Release Limits

- **Free accounts**: No bandwidth limit for releases
- **File size**: Up to 2GB per file (our images are ~500MB-1GB)
- **Storage**: Unlimited for releases

You can host as many images as you want!

### Alternative Hosting

For faster downloads, you can also host on:

- **Google Drive**: Share public link
- **Dropbox**: Public folder
- **OneDrive**: Share link
- **Own server**: Direct download

Just include link in README!

---

## Multi-Platform Downloads

### Windows

1. Download ZIP from releases
2. Extract with Windows Explorer or 7-Zip
3. Flash with balenaEtcher

### macOS

1. Download ZIP from releases
2. Double-click to extract
3. Flash with balenaEtcher

### Linux

```bash
# Download
wget https://github.com/user/HackDS/releases/download/v0.1.0/hackds-v0.1.0.img.zip

# Extract
unzip hackds-v0.1.0.img.zip

# Flash
sudo dd if=hackds-v0.1.0.img of=/dev/sdX bs=4M status=progress
```

---

## FAQ

**Q: Do I need to build HackDS?**
A: **No!** Just download pre-built images from Releases.

**Q: How often are new images released?**
A: Whenever new versions are tagged. Check Releases page.

**Q: Can I get updates without re-flashing?**
A: Yes! HackDS has auto-update system built-in.

**Q: Which version should I download?**
A: Latest stable release (green "Latest" tag).

**Q: Is the download safe?**
A: Yes! Built automatically by GitHub Actions. Verify SHA256 if concerned.

**Q: Can I share the image?**
A: Yes! It's open source. Share the GitHub release link.

**Q: What if download link is broken?**
A: Report issue on GitHub. We'll fix it.

**Q: Can I download older versions?**
A: Yes! All releases stay available on Releases page.

---

## Support

Need help?

1. **Documentation**: Check guides in repository
2. **Issues**: Report bugs on GitHub Issues
3. **Discussions**: Ask questions in Discussions
4. **Downloads**: Always use official Releases page

---

## Summary

**For most users:**

1. ✅ Download pre-built image
2. ✅ Flash to SD card
3. ✅ Boot and enjoy
4. ✅ Update via built-in system

**Skip building entirely!**

---

## Quick Reference

| Task | Time | Tool Needed |
|------|------|-------------|
| Download image | 5 min | Web browser |
| Extract ZIP | 1 min | Windows Explorer / 7-Zip |
| Flash to SD | 8 min | balenaEtcher |
| Boot HackDS | 1 min | Raspberry Pi |
| **Total** | **15 min** | **Ready to play!** |

---

**No building. No compiling. No waiting.**

**Just download and go!** 🚀

---

## Download Links

**Latest Release:**
https://github.com/yourusername/HackDS/releases/latest

**All Releases:**
https://github.com/yourusername/HackDS/releases

**balenaEtcher:**
https://etcher.balena.io/

---

*Updated: 2025-11-09*
*For building instructions, see BUILD_ON_WINDOWS.md or QUICKSTART.md*
