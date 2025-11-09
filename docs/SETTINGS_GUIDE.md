

# HackDS Settings & Controller Guide

Complete guide for configuring WiFi, Bluetooth, and PS5 DualSense controllers on HackDS.

## Table of Contents

- [PS5 Controller Setup](#ps5-controller-setup)
- [WiFi Configuration](#wifi-configuration)
- [Bluetooth Devices](#bluetooth-devices)
- [Settings Menu](#settings-menu)
- [Controller Button Mapping](#controller-button-mapping)
- [Troubleshooting](#troubleshooting)

---

## PS5 Controller Setup

### Quick Setup (Recommended)

1. **Put controller in pairing mode:**
   - Hold **PS + Share** buttons simultaneously
   - Controller light will start flashing rapidly (blue)

2. **Run automated setup:**
   ```bash
   bluetooth-manager ps5-setup
   ```

3. **Follow prompts:**
   - Confirms controller detection
   - Pairs automatically
   - Connects and configures

4. **Done!** Controller is now ready to use

### Manual Setup

```bash
# Enable Bluetooth
bluetooth-manager enable

# Scan for devices
bluetooth-manager scan

# Pair with controller (use MAC address from scan)
bluetooth-manager pair AA:BB:CC:DD:EE:FF

# Connect
bluetooth-manager connect AA:BB:CC:DD:EE:FF
```

### Reconnecting After Reboot

The controller should auto-connect when turned on. If not:

```bash
bluetooth-manager connect AA:BB:CC:DD:EE:FF
```

Or from settings menu:
- Press **F1** or **Tab** in main menu
- Select "Bluetooth Devices"
- Follow instructions

---

## WiFi Configuration

### Connect to WiFi Network

```bash
# Scan for available networks
wifi-manager scan

# Connect to network (with password)
wifi-manager connect "My Network" "mypassword"

# Connect to open network (no password)
wifi-manager connect "OpenWiFi"
```

### Check Connection Status

```bash
wifi-manager status
```

Output shows:
- Connection status
- Network SSID
- IP address

### Auto-Connect on Boot

WiFi credentials are saved automatically. To reconnect on boot:

```bash
wifi-manager auto-connect
```

Or add to system startup scripts.

### Disconnect

```bash
wifi-manager disconnect
```

### Disable WiFi

```bash
wifi-manager disable
```

---

## Bluetooth Devices

### Supported Devices

- **Controllers**: PS5 DualSense, PS4, Xbox, Nintendo Switch Pro
- **Keyboards**: Most Bluetooth keyboards
- **Mice**: Bluetooth mice
- **Audio**: Bluetooth headphones/speakers (experimental)

### Pair Any Device

```bash
# Enable Bluetooth
bluetooth-manager enable

# Scan for devices (10 seconds)
bluetooth-manager scan

# Pair with device
bluetooth-manager pair <MAC_ADDRESS>

# Connect
bluetooth-manager connect <MAC_ADDRESS>
```

### List Paired Devices

```bash
bluetooth-manager list
```

### Remove Paired Device

```bash
bluetooth-manager unpair <MAC_ADDRESS>
```

### Disconnect Device

```bash
bluetooth-manager disconnect <MAC_ADDRESS>
```

---

## Settings Menu

### Accessing Settings

**From Main Menu:**
- Press **F1** or **Tab** key
- Or press **Options** button on PS5 controller
- Or press **Menu** button on Xbox controller

**From Command Line:**
```bash
hackds-settings
```

### Settings Categories

1. **WiFi & Internet**
   - View network status
   - Connection instructions
   - Troubleshooting tips

2. **Bluetooth Devices**
   - Pair new devices
   - Manage paired devices
   - PS5 controller setup

3. **System Settings**
   - Version information
   - Auto-update configuration
   - System info

4. **Exit Settings**
   - Return to main menu

### Navigation

**Keyboard:**
- **Arrow Keys / WASD**: Navigate
- **Enter / Space**: Select
- **ESC**: Back/Exit

**PS5 Controller:**
- **D-Pad**: Navigate
- **X / Square**: Select
- **Circle**: Back
- **Options**: Exit

---

## Controller Button Mapping

### PS5 DualSense Controller

#### Main Menu

| Button | Action |
|--------|--------|
| D-Pad Up/Down | Navigate games |
| X | Launch game |
| Triangle | Check for updates |
| Options | Open settings |
| Share | (Reserved) |
| PS | (System button) |
| R1 | Check for updates |

#### In Settings

| Button | Action |
|--------|--------|
| D-Pad | Navigate |
| X | Select |
| Circle | Back |
| Options | Exit settings |

#### In Games

Button mapping depends on the game. SDL2 provides standard gamepad mapping.

### Xbox Controller

Same mapping as PS5, with Xbox button equivalents:
- **A** = X (PS5)
- **B** = Circle (PS5)
- **Y** = Triangle (PS5)
- **View** = Share (PS5)
- **Menu** = Options (PS5)

### Nintendo Switch Pro Controller

Standard SDL2 gamepad mapping applies.

---

## Cursor Settings

### Cursor is Disabled

The cursor is automatically hidden in HackDS menu and settings to provide a console-like experience. Navigate with:
- Keyboard (arrow keys)
- Game controller (D-pad)

No mouse required!

---

## Advanced Configuration

### Enable Services on Boot

To automatically enable Bluetooth and WiFi on boot, add to `/system/etc/rc.local`:

```bash
#!/bin/bash

# Enable Bluetooth
bluetooth-manager enable &

# Connect to saved WiFi
wifi-manager auto-connect &

# Wait for network (optional)
sleep 5

exit 0
```

Make executable:
```bash
chmod +x /system/etc/rc.local
```

### System Dependencies

Required packages (should be included in HackDS):
- `bluez` - Bluetooth stack
- `bluez-tools` - Bluetooth utilities
- `wireless-tools` - WiFi tools
- `wpa_supplicant` - WiFi authentication
- `dhclient` - DHCP client

### Kernel Modules

Required modules for Bluetooth/WiFi:
- `bluetooth` - Core Bluetooth
- `btbcm` - Broadcom Bluetooth
- `hci_uart` - UART Bluetooth interface
- `brcmfmac` - Broadcom WiFi
- `hid_sony` - PS5 controller support

Load modules:
```bash
sudo modprobe bluetooth
sudo modprobe btbcm
sudo modprobe hci_uart
sudo modprobe brcmfmac
sudo modprobe hid_sony
```

---

## Troubleshooting

### PS5 Controller Won't Pair

**Problem**: Controller not detected or pairing fails

**Solutions**:
1. Make sure controller is in pairing mode (PS + Share, light flashing)
2. Reset controller: Small pinhole button on back (hold 5 seconds)
3. Check Bluetooth is enabled: `bluetooth-manager enable`
4. Restart Bluetooth service:
   ```bash
   sudo systemctl restart bluetooth
   ```
5. Re-scan: `bluetooth-manager scan`

### Controller Disconnects Randomly

**Problem**: Controller keeps disconnecting

**Solutions**:
1. Check battery level (charge controller)
2. Reduce distance from Pi
3. Remove interference (other Bluetooth devices, WiFi routers)
4. Verify kernel module loaded:
   ```bash
   lsmod | grep hid_sony
   ```

### WiFi Won't Connect

**Problem**: Can't connect to network

**Solutions**:
1. Verify WiFi is enabled: `wifi-manager enable`
2. Check network name (SSID) is correct
3. Verify password is correct
4. Check signal strength: `wifi-manager scan`
5. Try open network first to test hardware
6. Check kernel module:
   ```bash
   lsmod | grep brcmfmac
   ```

### No Internet After WiFi Connection

**Problem**: Connected to WiFi but no internet

**Solutions**:
1. Check IP address: `wifi-manager status`
2. Test ping: `ping 8.8.8.8`
3. Check DNS: `ping google.com`
4. Restart DHCP:
   ```bash
   sudo dhclient -r wlan0
   sudo dhclient wlan0
   ```
5. Check router settings

### Bluetooth Service Not Running

**Problem**: `bluetooth-manager` commands fail

**Solutions**:
1. Check service status:
   ```bash
   sudo systemctl status bluetooth
   ```
2. Start service:
   ```bash
   sudo systemctl start bluetooth
   ```
3. Enable on boot:
   ```bash
   sudo systemctl enable bluetooth
   ```

### Settings Menu Won't Open

**Problem**: F1/Tab/Options button doesn't open settings

**Solutions**:
1. Check if settings binary exists:
   ```bash
   ls -la /system/bin/hackds-settings
   ```
2. Run manually:
   ```bash
   /system/bin/hackds-settings
   ```
3. Check for error messages
4. Reinstall system binaries

### Controller Input Not Working in Games

**Problem**: Controller works in menu but not in games

**Solutions**:
1. Game must support SDL2 game controller API
2. Check game documentation for controller support
3. Test with example games provided with HackDS
4. Verify controller is detected:
   ```bash
   ls /dev/input/js*
   ```

---

## Command Reference

### Bluetooth Manager

```bash
bluetooth-manager enable              # Enable Bluetooth
bluetooth-manager disable             # Disable Bluetooth
bluetooth-manager scan                # Scan for devices (10s)
bluetooth-manager pair <MAC>          # Pair with device
bluetooth-manager unpair <MAC>        # Remove pairing
bluetooth-manager connect <MAC>       # Connect to device
bluetooth-manager disconnect <MAC>    # Disconnect device
bluetooth-manager list                # List paired devices
bluetooth-manager ps5-setup           # Interactive PS5 setup
```

### WiFi Manager

```bash
wifi-manager enable                   # Enable WiFi
wifi-manager disable                  # Disable WiFi
wifi-manager scan                     # Scan networks
wifi-manager connect <SSID> [pass]    # Connect to network
wifi-manager disconnect               # Disconnect
wifi-manager status                   # Show connection info
wifi-manager auto-connect             # Connect to saved network
```

### Settings Menu

```bash
hackds-settings                       # Open settings GUI
```

---

## File Locations

### Configuration Files

- **WiFi Config**: `/settings/wifi/wpa_supplicant.conf`
- **Bluetooth Config**: `/var/lib/bluetooth/`
- **Update Settings**: `/settings/update-settings.json`

### System Binaries

- **Bluetooth Manager**: `/system/bin/bluetooth-manager`
- **WiFi Manager**: `/system/bin/wifi-manager`
- **Settings Menu**: `/system/bin/hackds-settings`
- **Main Menu**: `/system/bin/hackds-menu`

### Logs

- **System Log**: `/var/log/syslog`
- **Bluetooth Log**: `/var/log/bluetooth.log`
- **Kernel Log**: `dmesg`

---

## Tips & Best Practices

### Battery Life
- Turn off Bluetooth when not using controller
- Disable WiFi when not needed
- Controllers auto-sleep after 10 minutes

### Performance
- Use wired connection when possible (USB)
- Limit number of active Bluetooth devices
- WiFi can interfere with Bluetooth (both use 2.4GHz)

### Security
- Change default WiFi passwords
- Only pair trusted Bluetooth devices
- Unpair devices you no longer use

### Pairing Multiple Controllers
1. Pair first controller
2. Disconnect first controller
3. Pair second controller
4. Both can now connect (may need to alternate)

---

## FAQ

**Q: Can I use keyboard and mouse wirelessly?**
A: Yes! Pair Bluetooth keyboard/mouse via `bluetooth-manager`.

**Q: Does PS4 controller work?**
A: Yes! Same pairing process as PS5. Use `bluetooth-manager pair <MAC>`.

**Q: Can I connect to 5GHz WiFi?**
A: No, Pi Zero 2W only supports 2.4GHz WiFi.

**Q: How many controllers can I connect?**
A: Up to 4 Bluetooth devices simultaneously (depends on Bluetooth stack).

**Q: Will my controller stay paired after reboot?**
A: Yes! Pairing is persistent. Just turn on controller after reboot.

**Q: Can I use USB controller instead of Bluetooth?**
A: Yes! USB controllers work plug-and-play, no pairing needed.

**Q: Does DualShock 4 (PS4) work?**
A: Yes! Fully supported. Pair with `bluetooth-manager`.

**Q: How do I reset all Bluetooth pairings?**
A: `sudo rm -rf /var/lib/bluetooth/*` then reboot.

---

## Support

For issues or questions:
- Check GitHub Issues
- Read `docs/AUTO_UPDATE.md` for updates
- See `QUICKSTART.md` for general help

---

**Enjoy your HackDS gaming experience with full wireless control!** 🎮
