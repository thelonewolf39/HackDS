#!/usr/bin/env python3
"""
HackDS Auto-Update System
Checks GitHub repository for updates and applies them
"""

import os
import sys
import json
import hashlib
import subprocess
import urllib.request
import urllib.error
from pathlib import Path
from typing import Optional, Dict, Tuple

# Configuration
GITHUB_REPO = "YOUR_USERNAME/HackDS"  # User should update this
GITHUB_API = f"https://api.github.com/repos/{GITHUB_REPO}"
CURRENT_VERSION = "0.1.0"
SETTINGS_FILE = "/settings/update-settings.json"
BACKUP_DIR = "/var/backups/hackds"
UPDATE_CACHE = "/tmp/hackds-update"

class UpdateChecker:
    def __init__(self):
        self.settings = self.load_settings()
        self.current_version = CURRENT_VERSION

    def load_settings(self) -> Dict:
        """Load update settings"""
        default_settings = {
            "auto_update_enabled": False,
            "check_on_boot": True,
            "update_channel": "stable",  # stable, beta, dev
            "last_check": None,
            "last_update": None,
            "github_repo": GITHUB_REPO
        }

        try:
            if os.path.exists(SETTINGS_FILE):
                with open(SETTINGS_FILE, 'r') as f:
                    loaded = json.load(f)
                    default_settings.update(loaded)
        except Exception as e:
            print(f"Warning: Could not load settings: {e}")

        return default_settings

    def save_settings(self):
        """Save update settings"""
        try:
            os.makedirs(os.path.dirname(SETTINGS_FILE), exist_ok=True)
            with open(SETTINGS_FILE, 'w') as f:
                json.dump(self.settings, f, indent=2)
        except Exception as e:
            print(f"Error saving settings: {e}")

    def check_for_updates(self) -> Optional[Dict]:
        """Check GitHub for available updates"""
        print("Checking for updates...")

        # Update last check time
        import time
        self.settings['last_check'] = time.time()
        self.save_settings()

        try:
            # Get latest release
            url = f"{GITHUB_API}/releases/latest"
            req = urllib.request.Request(url)
            req.add_header('User-Agent', f'HackDS/{CURRENT_VERSION}')

            with urllib.request.urlopen(req, timeout=10) as response:
                data = json.loads(response.read().decode())

                latest_version = data.get('tag_name', '').lstrip('v')

                if self._is_newer_version(latest_version, self.current_version):
                    print(f"Update available: {latest_version}")
                    return {
                        'version': latest_version,
                        'name': data.get('name'),
                        'body': data.get('body'),
                        'published_at': data.get('published_at'),
                        'assets': data.get('assets', []),
                        'tarball_url': data.get('tarball_url'),
                        'zipball_url': data.get('zipball_url')
                    }
                else:
                    print(f"Already up to date (v{self.current_version})")
                    return None

        except urllib.error.HTTPError as e:
            if e.code == 404:
                print("No releases found. Checking commits...")
                return self._check_commits()
            else:
                print(f"HTTP Error: {e}")
                return None
        except Exception as e:
            print(f"Error checking for updates: {e}")
            return None

    def _check_commits(self) -> Optional[Dict]:
        """Check for new commits on main branch"""
        try:
            url = f"{GITHUB_API}/commits/main"
            req = urllib.request.Request(url)
            req.add_header('User-Agent', f'HackDS/{CURRENT_VERSION}')

            with urllib.request.urlopen(req, timeout=10) as response:
                data = json.loads(response.read().decode())

                commit_sha = data.get('sha', '')[:7]
                commit_msg = data.get('commit', {}).get('message', '')
                commit_date = data.get('commit', {}).get('author', {}).get('date', '')

                print(f"Latest commit: {commit_sha}")
                print(f"Message: {commit_msg}")

                return {
                    'version': f"dev-{commit_sha}",
                    'name': f"Development Build ({commit_sha})",
                    'body': commit_msg,
                    'published_at': commit_date,
                    'tarball_url': f"https://github.com/{GITHUB_REPO}/archive/main.tar.gz",
                    'type': 'commit'
                }

        except Exception as e:
            print(f"Error checking commits: {e}")
            return None

    def _is_newer_version(self, remote: str, local: str) -> bool:
        """Compare version strings"""
        try:
            remote_parts = [int(x) for x in remote.split('.')]
            local_parts = [int(x) for x in local.split('.')]

            # Pad to same length
            while len(remote_parts) < len(local_parts):
                remote_parts.append(0)
            while len(local_parts) < len(remote_parts):
                local_parts.append(0)

            return remote_parts > local_parts
        except:
            return remote != local

    def download_update(self, update_info: Dict) -> Optional[str]:
        """Download update package"""
        print(f"Downloading update: {update_info['version']}")

        # Create update cache directory
        os.makedirs(UPDATE_CACHE, exist_ok=True)

        # Download tarball
        tarball_url = update_info.get('tarball_url')
        if not tarball_url:
            print("Error: No download URL found")
            return None

        download_path = os.path.join(UPDATE_CACHE, "update.tar.gz")

        try:
            print(f"Downloading from: {tarball_url}")
            req = urllib.request.Request(tarball_url)
            req.add_header('User-Agent', f'HackDS/{CURRENT_VERSION}')

            with urllib.request.urlopen(req, timeout=30) as response:
                with open(download_path, 'wb') as f:
                    chunk_size = 8192
                    while True:
                        chunk = response.read(chunk_size)
                        if not chunk:
                            break
                        f.write(chunk)

            print(f"Downloaded to: {download_path}")
            print(f"Size: {os.path.getsize(download_path)} bytes")

            return download_path

        except Exception as e:
            print(f"Error downloading update: {e}")
            return None

    def apply_update(self, update_path: str) -> bool:
        """Apply downloaded update"""
        print("Applying update...")

        # Create backup first
        if not self._create_backup():
            print("Error: Failed to create backup")
            return False

        try:
            # Extract update
            extract_dir = os.path.join(UPDATE_CACHE, "extracted")
            os.makedirs(extract_dir, exist_ok=True)

            print("Extracting update...")
            subprocess.run(['tar', '-xzf', update_path, '-C', extract_dir],
                          check=True)

            # Find the extracted directory (GitHub adds repo name prefix)
            extracted_dirs = os.listdir(extract_dir)
            if not extracted_dirs:
                print("Error: No files extracted")
                return False

            source_dir = os.path.join(extract_dir, extracted_dirs[0])

            # Update system binaries
            print("Updating system binaries...")
            self._update_files(source_dir)

            # Update version
            self._update_version_file(update_path)

            # Update last update time
            import time
            self.settings['last_update'] = time.time()
            self.save_settings()

            print("Update applied successfully!")
            return True

        except Exception as e:
            print(f"Error applying update: {e}")
            print("Rolling back...")
            self._rollback()
            return False

    def _create_backup(self) -> bool:
        """Create backup of current system"""
        print("Creating backup...")

        try:
            os.makedirs(BACKUP_DIR, exist_ok=True)

            import time
            backup_name = f"backup-{int(time.time())}.tar.gz"
            backup_path = os.path.join(BACKUP_DIR, backup_name)

            # Backup system directory
            subprocess.run([
                'tar', '-czf', backup_path,
                '-C', '/',
                'system/bin',
                'system/lib'
            ], check=True)

            print(f"Backup created: {backup_path}")

            # Keep only last 3 backups
            self._cleanup_old_backups(3)

            return True

        except Exception as e:
            print(f"Error creating backup: {e}")
            return False

    def _cleanup_old_backups(self, keep: int):
        """Remove old backups, keeping only the newest ones"""
        try:
            backups = sorted(Path(BACKUP_DIR).glob("backup-*.tar.gz"))
            if len(backups) > keep:
                for backup in backups[:-keep]:
                    backup.unlink()
                    print(f"Removed old backup: {backup}")
        except Exception as e:
            print(f"Error cleaning up backups: {e}")

    def _update_files(self, source_dir: str):
        """Update system files from extracted update"""
        # Update binaries
        bin_src = os.path.join(source_dir, "src")
        if os.path.exists(bin_src):
            print("Building updated binaries...")
            subprocess.run(['make', '-C', bin_src, 'clean', 'all'],
                          check=True, cwd=source_dir)
            subprocess.run(['make', '-C', bin_src, 'install'],
                          check=True, cwd=source_dir,
                          env={**os.environ, 'DESTDIR': '/'})

        # Update scripts
        scripts_src = os.path.join(source_dir, "tools")
        if os.path.exists(scripts_src):
            print("Updating tools...")
            subprocess.run(['cp', '-r', scripts_src, '/system/'],
                          check=True)

    def _update_version_file(self, update_path: str):
        """Update version file"""
        version_file = "/system/etc/hackds-version"
        try:
            with open(version_file, 'w') as f:
                f.write(f"{update_path}\n")
        except Exception as e:
            print(f"Error updating version file: {e}")

    def _rollback(self) -> bool:
        """Rollback to previous backup"""
        print("Rolling back to previous version...")

        try:
            backups = sorted(Path(BACKUP_DIR).glob("backup-*.tar.gz"))
            if not backups:
                print("Error: No backups found")
                return False

            latest_backup = backups[-1]
            print(f"Restoring from: {latest_backup}")

            subprocess.run([
                'tar', '-xzf', str(latest_backup),
                '-C', '/'
            ], check=True)

            print("Rollback successful")
            return True

        except Exception as e:
            print(f"Error during rollback: {e}")
            return False


def main():
    print("=== HackDS Update System ===\n")

    if len(sys.argv) < 2:
        print("Usage:")
        print("  hackds-updater check              - Check for updates")
        print("  hackds-updater update              - Check and apply updates")
        print("  hackds-updater enable              - Enable auto-updates")
        print("  hackds-updater disable             - Disable auto-updates")
        print("  hackds-updater status              - Show update status")
        print("  hackds-updater rollback            - Rollback to previous version")
        return 1

    command = sys.argv[1]
    updater = UpdateChecker()

    if command == "check":
        update_info = updater.check_for_updates()
        if update_info:
            print(f"\nUpdate available: {update_info['version']}")
            print(f"Name: {update_info['name']}")
            print(f"Description: {update_info['body'][:200]}...")
            print(f"\nRun 'hackds-updater update' to install")
        else:
            print("No updates available")

    elif command == "update":
        update_info = updater.check_for_updates()
        if not update_info:
            print("No updates available")
            return 0

        print(f"\nUpdate found: {update_info['version']}")
        print(f"{update_info['name']}\n")

        # Ask for confirmation if not auto-update
        if not updater.settings.get('auto_update_enabled'):
            response = input("Apply this update? (yes/no): ")
            if response.lower() not in ['yes', 'y']:
                print("Update cancelled")
                return 0

        download_path = updater.download_update(update_info)
        if not download_path:
            print("Error: Failed to download update")
            return 1

        if updater.apply_update(download_path):
            print("\nUpdate complete! Please reboot for changes to take effect.")
            print("Run: sudo reboot")
            return 0
        else:
            print("\nUpdate failed!")
            return 1

    elif command == "enable":
        updater.settings['auto_update_enabled'] = True
        updater.save_settings()
        print("Auto-updates enabled")

    elif command == "disable":
        updater.settings['auto_update_enabled'] = False
        updater.save_settings()
        print("Auto-updates disabled")

    elif command == "status":
        print(f"Current version: {CURRENT_VERSION}")
        print(f"Auto-updates: {'Enabled' if updater.settings.get('auto_update_enabled') else 'Disabled'}")
        print(f"Check on boot: {'Yes' if updater.settings.get('check_on_boot') else 'No'}")
        print(f"Update channel: {updater.settings.get('update_channel')}")

        last_check = updater.settings.get('last_check')
        if last_check:
            import time
            print(f"Last check: {time.ctime(last_check)}")

        last_update = updater.settings.get('last_update')
        if last_update:
            import time
            print(f"Last update: {time.ctime(last_update)}")

    elif command == "rollback":
        response = input("Rollback to previous version? This cannot be undone. (yes/no): ")
        if response.lower() in ['yes', 'y']:
            if updater._rollback():
                print("\nRollback complete! Please reboot.")
                return 0
            else:
                print("\nRollback failed!")
                return 1

    else:
        print(f"Unknown command: {command}")
        return 1

    return 0


if __name__ == '__main__':
    sys.exit(main())
