# HackDS Auto-Update System

## Overview

HackDS includes a built-in auto-update system that can check for and apply updates from your GitHub repository. Updates can be installed automatically or manually, and the system includes rollback capabilities for safety.

## Features

- **GitHub Integration**: Checks releases and commits from your repo
- **User Consent**: Updates only install when approved
- **Safe Updates**: Automatic backup before applying changes
- **Rollback Support**: Restore previous version if needed
- **Update Channels**: Stable releases or development builds
- **GUI Integration**: Update notifications in the menu

## Configuration

### Setting Your GitHub Repository

Edit the updater script to point to your repository:

```bash
sudo nano /system/bin/hackds-updater
```

Change this line:
```python
GITHUB_REPO = "YOUR_USERNAME/HackDS"  # Update this!
```

To your actual repository:
```python
GITHUB_REPO = "myusername/HackDS"
```

### Update Settings File

Settings are stored in `/settings/update-settings.json`:

```json
{
  "auto_update_enabled": false,
  "check_on_boot": true,
  "update_channel": "stable",
  "github_repo": "myusername/HackDS"
}
```

**Settings Explained:**

- `auto_update_enabled` (boolean): If true, updates install automatically without asking
- `check_on_boot` (boolean): Check for updates when system boots
- `update_channel` (string): "stable" for releases, "dev" for latest commits
- `github_repo` (string): Your GitHub repository (username/repo-name)

## Using the Update System

### From the Menu (GUI)

1. **Check for Updates**: Press `U` key
2. **Install Update**: If update is available, press `I` key
3. **Status**: Update notification appears as yellow banner when available

### From Command Line

```bash
# Check for updates
hackds-updater check

# Check and install updates (with confirmation)
hackds-updater update

# Enable auto-updates
hackds-updater enable

# Disable auto-updates
hackds-updater disable

# Show update status
hackds-updater status

# Rollback to previous version
hackds-updater rollback
```

## How It Works

### Update Detection

The updater checks GitHub in two ways:

1. **Stable Channel**: Checks for tagged releases
   - Uses GitHub API: `/repos/user/repo/releases/latest`
   - Compares version numbers (e.g., v0.1.0 → v0.2.0)

2. **Dev Channel**: Checks latest commits on main branch
   - Uses GitHub API: `/repos/user/repo/commits/main`
   - Downloads latest source code

### Update Process

1. **Check**: Query GitHub API for new versions
2. **Download**: Download tarball from GitHub
3. **Backup**: Create backup of current system
4. **Extract**: Extract new files to temporary location
5. **Build**: Compile updated binaries
6. **Install**: Replace system files
7. **Verify**: Ensure installation succeeded
8. **Reboot**: Restart to apply changes

If any step fails, the system automatically rolls back to the previous backup.

### Backup System

Before each update:
- Current system files are archived to `/var/backups/hackds/`
- Backup includes: `/system/bin` and `/system/lib`
- Last 3 backups are kept, older ones are deleted
- Backups are named: `backup-{timestamp}.tar.gz`

## Creating Releases for Updates

### Method 1: GitHub Releases (Recommended)

1. Tag your commit:
```bash
git tag -a v0.2.0 -m "Version 0.2.0"
git push origin v0.2.0
```

2. Create release on GitHub:
   - Go to your repo → Releases → Draft a new release
   - Select your tag
   - Add release notes
   - Publish release

3. Users will see the update in HackDS menu

### Method 2: Development Builds

Just push to main branch:
```bash
git add .
git commit -m "Updated feature X"
git push origin main
```

Users on "dev" channel will get the latest commit.

## Update Notifications

When an update is available:

- **GUI**: Yellow banner appears at top of menu
  - Shows version number
  - Press `I` to install

- **CLI**: "Update available" message when running `hackds-updater check`

## Safety Features

### Automatic Backup

Every update creates a backup first. If something goes wrong, the system rolls back automatically.

### Manual Rollback

If you need to revert to a previous version:

```bash
hackds-updater rollback
```

This restores the most recent backup. You'll need to reboot after rollback.

### Integrity Checks

- Downloads are verified for completeness
- Build process must succeed or update is aborted
- System checks are performed before finalizing

## Enabling Auto-Updates

### For Users

Enable automatic updates (will install without asking):

```bash
hackds-updater enable
```

Configure settings:
```bash
sudo nano /settings/update-settings.json
```

Set:
```json
{
  "auto_update_enabled": true,
  "check_on_boot": true,
  "update_channel": "stable"
}
```

### For Developers

This is **not recommended** for development systems as it may overwrite your changes!

Instead, use manual updates:
```bash
hackds-updater disable
```

## Troubleshooting

### "Failed to check for updates"

**Problem**: Can't connect to GitHub

**Solutions**:
- Check network connection
- Verify GitHub repository exists
- Check `/settings/update-settings.json` has correct repo
- Try: `ping github.com`

### "Download failed"

**Problem**: Update download interrupted

**Solutions**:
- Check available disk space: `df -h`
- Verify network is stable
- Try updating manually: `hackds-updater update`

### "Update failed"

**Problem**: Installation encountered an error

**Solutions**:
- System automatically rolled back
- Check logs: `dmesg | tail -50`
- Try updating again after reboot
- If persistent, manually rollback: `hackds-updater rollback`

### "No releases found"

**Problem**: Repository has no tagged releases

**Solutions**:
- Create a release on GitHub (see "Creating Releases" above)
- Or use dev channel: Set `"update_channel": "dev"` in settings

### Rollback fails

**Problem**: No backups available

**Solutions**:
- Backups are in `/var/backups/hackds/`
- Check if directory exists: `ls -la /var/backups/hackds/`
- May need to reinstall from SD card image

## Advanced Configuration

### Custom Update Server

You can point to a different GitHub repo (e.g., for testing):

```bash
sudo nano /system/bin/hackds-updater
```

Change:
```python
GITHUB_REPO = "your-fork/HackDS"
```

### Update Frequency

To check for updates regularly, add a cron job:

```bash
sudo crontab -e
```

Add:
```
0 2 * * * /system/bin/hackds-updater check
```

This checks daily at 2 AM.

### Network Configuration

Update checker requires internet access. If using WiFi, ensure it's configured before boot.

## Security Considerations

1. **HTTPS**: All downloads use HTTPS from GitHub
2. **No Auto-Execute**: Updates require user confirmation (unless auto-update enabled)
3. **Backup First**: Always creates backup before modifying system
4. **Source Verification**: Downloads only from specified GitHub repo
5. **No Sudo in Auto**: Updater doesn't require sudo for checking

## API Rate Limits

GitHub API has rate limits:
- **Unauthenticated**: 60 requests/hour
- **Authenticated**: 5000 requests/hour

For personal use, this is more than enough. If you hit limits, check less frequently.

## Example Workflow

### As a Developer

1. Make changes to HackDS
2. Test locally
3. Commit and push to GitHub
4. Create release with version tag
5. Users automatically notified
6. Users press `I` to install
7. System updates and reboots

### As a User

1. Boot HackDS
2. See yellow "Update Available" banner
3. Press `I` to install
4. Wait for download and installation
5. System reboots automatically
6. Enjoy updated features!

## Uninstalling Updates

To remove the auto-update system:

```bash
sudo rm /system/bin/hackds-updater
sudo rm /settings/update-settings.json
```

Updates will no longer check or install.

## Contributing

If you want to improve the update system:

1. Fork the repository
2. Modify `src/updater/hackds_updater.py`
3. Test thoroughly
4. Submit a pull request

## Files Modified by Updates

The updater can update:
- `/system/bin/*` - System binaries
- `/system/lib/*` - Libraries
- `/system/share/*` - Shared resources
- Configuration files (carefully!)

The updater **never** modifies:
- `/games/*` - Your games are safe
- `/mods/*` - Mods preserved
- `/settings/*` - Settings kept (except update-settings.json)
- `/boot/*` - Boot partition unchanged

## Version History Tracking

Version information is stored in:
- `/system/etc/hackds-version` - Current version
- `/settings/update-settings.json` - Update history

## FAQ

**Q: Will updates delete my games?**
A: No! Games in `/games/` are never touched by updates.

**Q: Can I disable updates?**
A: Yes. Run `hackds-updater disable` or set `check_on_boot: false` in settings.

**Q: How much space do updates need?**
A: Typically 50-100MB for download + backup. Ensure 200MB free space.

**Q: Can I update without internet?**
A: No. Updates require GitHub access. You can manually copy files via SD card instead.

**Q: What if an update breaks my system?**
A: Run `hackds-updater rollback` or reflash SD card from backup image.

**Q: Can I preview updates before installing?**
A: Yes! Check the release notes on GitHub before pressing `I` to install.

---

**Questions or Issues?**

Report bugs or request features on GitHub: https://github.com/YOUR_USERNAME/HackDS/issues
