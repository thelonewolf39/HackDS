# HackDS Auto-Update Quick Setup

## For Users (5-Minute Setup)

### 1. Configure Your Settings

Create or edit `/settings/update-settings.json` on your Pi:

```bash
nano /settings/update-settings.json
```

Add this content (replace with the actual repository):

```json
{
  "auto_update_enabled": false,
  "check_on_boot": true,
  "update_channel": "stable",
  "github_repo": "yourusername/HackDS"
}
```

**Important**: Change `yourusername/HackDS` to the actual GitHub repository!

### 2. Configure the Updater

Edit the updater script:

```bash
sudo nano /system/bin/hackds-updater
```

Find this line near the top:
```python
GITHUB_REPO = "YOUR_USERNAME/HackDS"
```

Change it to:
```python
GITHUB_REPO = "yourusername/HackDS"  # Your actual repo!
```

Save and exit (Ctrl+X, Y, Enter).

### 3. Test It

Check if updates work:

```bash
hackds-updater check
```

You should see either:
- "Update available: vX.X.X" - if there's a new version
- "Already up to date" - if you're current
- "Error" - if there's a configuration problem

### 4. Enable Auto-Updates (Optional)

If you want automatic updates:

```bash
hackds-updater enable
```

Now updates will install automatically on boot if available.

### 5. Use It

#### From GUI Menu:
- Press `U` to check for updates
- Press `I` to install if update is available

#### From Command Line:
```bash
# Check for updates
hackds-updater check

# Install updates
hackds-updater update

# Show status
hackds-updater status
```

Done! 🎉

---

## For Developers (Repository Setup)

### 1. Fork or Clone HackDS

```bash
git clone https://github.com/your username/HackDS.git
cd HackDS
```

### 2. Make Changes

Edit your code, add features, fix bugs...

```bash
# Make your changes
nano src/menu/menu.c

# Test locally
./build-all.sh
```

### 3. Create a Release

#### Option A: Tagged Release (Recommended)

```bash
# Commit your changes
git add .
git commit -m "Add awesome new feature"

# Tag the release
git tag -a v0.2.0 -m "Version 0.2.0 - Added feature X"

# Push everything
git push origin main
git push origin v0.2.0
```

Then on GitHub:
1. Go to your repo → Releases
2. Click "Draft a new release"
3. Select tag `v0.2.0`
4. Add release notes
5. Click "Publish release"

#### Option B: Development Build

Just push to main:

```bash
git add .
git commit -m "Updated feature"
git push origin main
```

Users on "dev" channel will get this automatically.

### 4. Users Get Notified

When users boot their HackDS:
1. System checks GitHub
2. Sees your new release
3. Shows update notification
4. User presses `I` to install
5. Your changes are deployed!

---

## Quick Reference

### Common Commands

```bash
# Check for updates
hackds-updater check

# Install update
hackds-updater update

# Enable auto-updates
hackds-updater enable

# Disable auto-updates
hackds-updater disable

# Show current status
hackds-updater status

# Rollback to previous version
hackds-updater rollback
```

### Settings File Location

`/settings/update-settings.json`

### Updater Script Location

`/system/bin/hackds-updater`

### Backup Location

`/var/backups/hackds/backup-*.tar.gz`

---

## Troubleshooting

### "No releases found"

**Solution**: Create a release on GitHub with a version tag (v0.1.0, v0.2.0, etc.)

### "Can't connect to GitHub"

**Solution**: Check network connection. Try `ping github.com`

### "Update failed"

**Solution**: System auto-rolled back. Try again or run `hackds-updater rollback`

### Want to test without breaking production?

1. Create a test branch
2. Point updater to test branch
3. Test updates
4. Merge to main when ready

---

## Advanced: Testing Updates Locally

### Test the Update System

```bash
# Dry run - check without installing
hackds-updater check

# See what would be downloaded
grep tarball_url /tmp/hackds-update-check.log

# Test rollback
hackds-updater rollback
```

### Use a Different Repository

For testing, point to a different repo:

```bash
sudo nano /system/bin/hackds-updater
```

Change:
```python
GITHUB_REPO = "test-user/HackDS-test"
```

---

## Safety Reminders

✅ **Automatic backup** before every update
✅ **Rollback available** if something goes wrong
✅ **Games never deleted** - stored separately
✅ **User confirmation** required (unless auto-update enabled)

❌ **Don't enable auto-update** on development systems
❌ **Don't modify** `/var/backups/hackds/` manually
❌ **Always test** changes before releasing

---

## Example Update Workflow

```bash
# 1. Developer makes changes
vim src/menu/menu.c
git commit -am "Added feature X"

# 2. Developer creates release
git tag v0.2.0
git push origin main --tags

# 3. User checks for updates
User presses 'U' in menu

# 4. System shows notification
"Update Available: v0.2.0"

# 5. User installs
User presses 'I'

# 6. System updates and reboots
Downloads → Backs up → Installs → Reboots

# 7. User enjoys new features!
```

---

## Need More Help?

- **Full documentation**: See `docs/AUTO_UPDATE.md`
- **Report bugs**: GitHub Issues
- **Ask questions**: GitHub Discussions

Happy updating! 🚀
