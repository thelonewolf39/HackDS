# How to Create HackDS Releases

## For Repository Owners

This guide shows how to create pre-built images that users can download (no building required on their end).

---

## Automatic Build System

HackDS uses **GitHub Actions** to automatically build images whenever you create a release. No local building needed!

### What Happens Automatically

1. You create a version tag
2. GitHub Actions detects the tag
3. Automatically builds HackDS
4. Creates bootable `.img` file
5. Uploads to Releases page
6. Users download and flash!

**You don't build anything locally!** GitHub does it for you.

---

## Creating a Release (3 Steps)

### Step 1: Tag Your Version

```bash
# Make sure your code is committed
git add .
git commit -m "Release v0.1.0"

# Create a version tag
git tag -a v0.1.0 -m "HackDS v0.1.0 - Initial Release"

# Push code and tags
git push origin main
git push origin v0.1.0
```

### Step 2: Wait for GitHub Actions

1. Go to your repository on GitHub
2. Click **"Actions"** tab
3. See "Build HackDS Image" workflow running
4. Wait 30-45 minutes for build to complete
5. Green checkmark = success!

### Step 3: Check the Release

1. Go to **"Releases"** page
2. See new release with your tag (v0.1.0)
3. Download section has:
   - `hackds-v0.1.0.img.zip` (the image!)
   - `hackds-v0.1.0.sha256` (checksum)

**Done!** Users can now download and flash.

---

## Manual Trigger (Optional)

You can also trigger builds manually without creating a tag:

1. Go to GitHub repository
2. Click **"Actions"** tab
3. Click **"Build HackDS Image"** workflow
4. Click **"Run workflow"** button
5. Select branch (usually `main`)
6. Click green **"Run workflow"** button

Build starts immediately!

---

## Version Numbering

Use semantic versioning:

```
v0.1.0 - Major.Minor.Patch

Examples:
v0.1.0 - Initial release
v0.2.0 - New features added
v0.2.1 - Bug fixes
v1.0.0 - First stable release
```

---

## Release Checklist

Before creating a release:

- [ ] Code is tested and working
- [ ] Documentation is updated
- [ ] Example games work
- [ ] WiFi/Bluetooth tested
- [ ] PS5 controller tested
- [ ] Auto-update tested
- [ ] README has correct version
- [ ] CHANGELOG updated

---

## What Gets Built

The GitHub Actions workflow automatically:

✅ Sets up Ubuntu build environment
✅ Installs all dependencies
✅ Downloads Raspberry Pi kernel source
✅ Compiles kernel for Pi Zero 2W
✅ Builds HackDS system components
✅ Creates bootable filesystem
✅ Generates SD card image
✅ Compresses to ZIP file
✅ Calculates SHA256 checksum
✅ Uploads to Releases page

**Total build time: 30-45 minutes** (all automated)

---

## Build Configuration

The workflow is defined in `.github/workflows/build-release.yml`

You can customize:

- Build triggers (tags, branches, manual)
- Ubuntu version
- Build steps
- Output files
- Release notes format

Just edit the YAML file and push changes!

---

## Troubleshooting Builds

### Build fails with "out of disk space"

**Fix:** The workflow includes cleanup step:
```yaml
- name: Free up disk space
  run: |
    sudo rm -rf /usr/share/dotnet
    sudo rm -rf /opt/ghc
```

This should be enough. If not, open an issue on GitHub Actions.

### Build fails at kernel compilation

**Fix:**
1. Check `build-all.sh` script
2. Ensure kernel source downloads correctly
3. Check cross-compiler is installed
4. Review error logs in Actions tab

### Release doesn't appear

**Fix:**
1. Check Actions tab for errors
2. Ensure tag starts with `v` (e.g., v0.1.0)
3. Workflow only runs on tags matching `v*`

### Image is too large to upload

**Fix:**
- Images are compressed to ZIP (much smaller)
- GitHub allows up to 2GB per file
- Our images are typically 500MB-1GB compressed

---

## Monitoring Builds

### Real-Time Progress

1. Go to Actions tab during build
2. Click on running workflow
3. See live console output
4. Monitor each step

### Build Artifacts

Even if release fails, artifacts are saved:

1. Go to completed workflow
2. Scroll to "Artifacts" section
3. Download `hackds-image.zip`
4. Kept for 90 days

---

## Testing Releases

Before announcing to users:

```bash
# Download your release
wget https://github.com/user/HackDS/releases/download/v0.1.0/hackds-v0.1.0.img.zip

# Extract
unzip hackds-v0.1.0.img.zip

# Verify checksum
sha256sum -c hackds-v0.1.0.sha256

# Flash to test SD card
# Boot and test all features
```

---

## Release Notes

GitHub Actions auto-generates release notes with:

- Download instructions
- What's included
- SHA256 checksum
- First boot instructions
- Links to documentation

You can edit after creation:

1. Go to Releases page
2. Click "Edit" on release
3. Modify description
4. Save changes

---

## Deleting Releases

If something went wrong:

1. Go to Releases page
2. Click release to delete
3. Click "Delete" button
4. Confirm deletion
5. Optionally delete tag:
   ```bash
   git tag -d v0.1.0
   git push origin :refs/tags/v0.1.0
   ```

---

## Pre-Releases

For beta testing:

```bash
# Create pre-release tag
git tag -a v0.2.0-beta -m "Beta release"
git push origin v0.2.0-beta
```

Mark as pre-release on GitHub:
1. Go to created release
2. Click "Edit"
3. Check "This is a pre-release"
4. Save

---

## Multiple Variants

To build different configurations:

```bash
# Example: Lite version
git tag -a v0.1.0-lite -m "Lite version"

# Example: Full version
git tag -a v0.1.0-full -m "Full version"
```

Edit workflow to detect tag suffix and build accordingly.

---

## Automatic Releases

Set up automatic releases on every push to main:

```yaml
# In .github/workflows/build-release.yml
on:
  push:
    branches:
      - main
```

This creates nightly builds!

---

## Storage Limits

GitHub provides:

- **Unlimited** storage for releases
- **Unlimited** downloads
- **Unlimited** bandwidth

Your images won't cost anything to host!

---

## Collaboration

Multiple people can create releases:

1. Give collaborators "Write" access
2. They can push tags
3. GitHub Actions builds automatically
4. Anyone with access can manage releases

---

## Quick Reference Commands

```bash
# Create and push release tag
git tag -a v0.1.0 -m "Version 0.1.0"
git push origin v0.1.0

# Delete tag (if needed)
git tag -d v0.1.0
git push origin :refs/tags/v0.1.0

# List all tags
git tag -l

# View tag info
git show v0.1.0
```

---

## After Release

Announce to users:

1. Update README with release link
2. Post in Discussions
3. Tweet/share on social media
4. Add to changelog

Users can now:
1. Download image
2. Flash to SD card
3. Boot HackDS immediately

No building required on their end!

---

## Success Metrics

Check release success:

- **Downloads**: See count on release page
- **Issues**: Users report problems
- **Stars**: Repository popularity
- **Forks**: People modifying code

---

## Example Workflow

Typical release process:

```bash
# Day 1: Development
git commit -m "Add new feature"
git push

# Day 2: More development
git commit -m "Fix bugs"
git push

# Day 3: Ready for release
git tag -a v0.2.0 -m "Version 0.2.0 - New features"
git push origin v0.2.0

# GitHub builds automatically (30-45 min)

# Day 3 (later): Announce
# Users download and flash
# Feedback comes in

# Day 4: Hotfix if needed
git commit -m "Hotfix"
git tag -a v0.2.1 -m "Version 0.2.1 - Bugfix"
git push origin v0.2.1
```

---

## Summary

**To create a release:**

1. Commit your code
2. Create a tag: `git tag -a v0.1.0 -m "Version 0.1.0"`
3. Push tag: `git push origin v0.1.0`
4. **GitHub builds automatically!**
5. Wait 30-45 minutes
6. Image appears in Releases
7. Users download and flash

**You never build locally!** GitHub does all the work.

---

## Support

For issues with GitHub Actions:

- Check Actions tab for errors
- Review `.github/workflows/build-release.yml`
- GitHub Actions documentation
- Ask in GitHub Discussions

---

**Happy releasing!** 🚀

Now users can enjoy HackDS without building anything!
