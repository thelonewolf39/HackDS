#!/bin/bash
#
# HackDS Boot-time Update Checker
# Checks for updates when the system boots (if enabled)
#

SETTINGS_FILE="/settings/update-settings.json"

# Check if auto-update checking is enabled
if [ -f "${SETTINGS_FILE}" ]; then
    CHECK_ON_BOOT=$(grep -o '"check_on_boot"[[:space:]]*:[[:space:]]*true' "${SETTINGS_FILE}")

    if [ -n "${CHECK_ON_BOOT}" ]; then
        echo "Checking for HackDS updates..."

        # Run update checker in background to not delay boot
        (
            sleep 10  # Wait for network to come up
            /system/bin/hackds-updater check

            # If auto_update_enabled is true, install automatically
            AUTO_UPDATE=$(grep -o '"auto_update_enabled"[[:space:]]*:[[:space:]]*true' "${SETTINGS_FILE}")
            if [ -n "${AUTO_UPDATE}" ]; then
                /system/bin/hackds-updater update
            fi
        ) &
    fi
fi
