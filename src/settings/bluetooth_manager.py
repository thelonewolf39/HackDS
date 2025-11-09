#!/usr/bin/env python3
"""
HackDS Bluetooth Manager
Manages Bluetooth pairing, especially for PS5 DualSense controllers
"""

import os
import sys
import subprocess
import time
import re
from typing import List, Dict, Optional

class BluetoothManager:
    def __init__(self):
        self.paired_devices = []
        self.available_devices = []
        self.scanning = False

    def check_bluetooth_available(self) -> bool:
        """Check if Bluetooth hardware is available"""
        try:
            result = subprocess.run(['hciconfig'], capture_output=True, text=True)
            return 'UP RUNNING' in result.stdout
        except FileNotFoundError:
            print("BlueZ tools not installed")
            return False

    def enable_bluetooth(self) -> bool:
        """Enable Bluetooth adapter"""
        try:
            subprocess.run(['sudo', 'hciconfig', 'hci0', 'up'], check=True)
            subprocess.run(['sudo', 'bluetoothctl', 'power', 'on'],
                          input=b'', check=True)
            print("Bluetooth enabled")
            return True
        except subprocess.CalledProcessError as e:
            print(f"Failed to enable Bluetooth: {e}")
            return False

    def disable_bluetooth(self) -> bool:
        """Disable Bluetooth adapter"""
        try:
            subprocess.run(['sudo', 'bluetoothctl', 'power', 'off'],
                          input=b'', check=True)
            subprocess.run(['sudo', 'hciconfig', 'hci0', 'down'], check=True)
            print("Bluetooth disabled")
            return True
        except subprocess.CalledProcessError as e:
            print(f"Failed to disable Bluetooth: {e}")
            return False

    def start_scan(self, duration: int = 10) -> List[Dict]:
        """Scan for Bluetooth devices"""
        print(f"Scanning for Bluetooth devices ({duration}s)...")

        self.available_devices = []
        devices = {}

        try:
            # Start scanning
            scan_process = subprocess.Popen(
                ['sudo', 'bluetoothctl', 'scan', 'on'],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )

            # Collect devices for duration
            start_time = time.time()
            while time.time() - start_time < duration:
                line = scan_process.stdout.readline()
                if not line:
                    break

                # Parse device info: [NEW] Device AA:BB:CC:DD:EE:FF Name
                if 'Device' in line:
                    parts = line.split()
                    if len(parts) >= 3:
                        mac = parts[2]
                        name = ' '.join(parts[3:]) if len(parts) > 3 else 'Unknown'

                        # Identify device type
                        device_type = 'Unknown'
                        if 'DualSense' in name or 'Wireless Controller' in name:
                            device_type = 'PS5 Controller'
                        elif 'PlayStation' in name:
                            device_type = 'PlayStation Device'
                        elif 'Xbox' in name:
                            device_type = 'Xbox Controller'
                        elif 'Joy-Con' in name or 'Nintendo' in name:
                            device_type = 'Nintendo Controller'
                        elif 'keyboard' in name.lower():
                            device_type = 'Keyboard'
                        elif 'mouse' in name.lower():
                            device_type = 'Mouse'

                        devices[mac] = {
                            'mac': mac,
                            'name': name,
                            'type': device_type,
                            'paired': False
                        }

            # Stop scanning
            scan_process.terminate()
            scan_process.wait()
            subprocess.run(['sudo', 'bluetoothctl', 'scan', 'off'],
                          input=b'', check=True, capture_output=True)

            self.available_devices = list(devices.values())
            print(f"Found {len(self.available_devices)} devices")

            return self.available_devices

        except Exception as e:
            print(f"Error scanning: {e}")
            return []

    def pair_device(self, mac_address: str) -> bool:
        """Pair with a Bluetooth device"""
        print(f"Pairing with {mac_address}...")

        try:
            # Trust the device
            subprocess.run(['sudo', 'bluetoothctl', 'trust', mac_address],
                          input=b'', check=True, capture_output=True)

            # Pair
            result = subprocess.run(
                ['sudo', 'bluetoothctl', 'pair', mac_address],
                input=b'yes\n',
                capture_output=True,
                text=True,
                timeout=30
            )

            if 'Pairing successful' in result.stdout or result.returncode == 0:
                print("Pairing successful")

                # Connect
                subprocess.run(['sudo', 'bluetoothctl', 'connect', mac_address],
                              input=b'', check=True, capture_output=True)
                print("Connected")

                return True
            else:
                print(f"Pairing failed: {result.stdout}")
                return False

        except subprocess.TimeoutExpired:
            print("Pairing timeout - device may require manual confirmation")
            return False
        except subprocess.CalledProcessError as e:
            print(f"Error pairing: {e}")
            return False

    def unpair_device(self, mac_address: str) -> bool:
        """Unpair a Bluetooth device"""
        print(f"Unpairing {mac_address}...")

        try:
            subprocess.run(['sudo', 'bluetoothctl', 'remove', mac_address],
                          input=b'', check=True, capture_output=True)
            print("Device removed")
            return True
        except subprocess.CalledProcessError as e:
            print(f"Error unpairing: {e}")
            return False

    def connect_device(self, mac_address: str) -> bool:
        """Connect to a paired device"""
        print(f"Connecting to {mac_address}...")

        try:
            subprocess.run(['sudo', 'bluetoothctl', 'connect', mac_address],
                          input=b'', check=True, capture_output=True, timeout=10)
            print("Connected")
            return True
        except Exception as e:
            print(f"Error connecting: {e}")
            return False

    def disconnect_device(self, mac_address: str) -> bool:
        """Disconnect a device"""
        print(f"Disconnecting {mac_address}...")

        try:
            subprocess.run(['sudo', 'bluetoothctl', 'disconnect', mac_address],
                          input=b'', check=True, capture_output=True)
            print("Disconnected")
            return True
        except subprocess.CalledProcessError as e:
            print(f"Error disconnecting: {e}")
            return False

    def list_paired_devices(self) -> List[Dict]:
        """List all paired devices"""
        try:
            result = subprocess.run(
                ['bluetoothctl', 'paired-devices'],
                capture_output=True,
                text=True,
                check=True
            )

            devices = []
            for line in result.stdout.splitlines():
                # Format: Device AA:BB:CC:DD:EE:FF Name
                parts = line.split()
                if len(parts) >= 3 and parts[0] == 'Device':
                    mac = parts[1]
                    name = ' '.join(parts[2:])
                    devices.append({
                        'mac': mac,
                        'name': name,
                        'paired': True
                    })

            self.paired_devices = devices
            return devices

        except Exception as e:
            print(f"Error listing devices: {e}")
            return []

    def setup_ps5_controller(self, mac_address: str) -> bool:
        """Special setup for PS5 DualSense controller"""
        print("Setting up PS5 DualSense Controller...")

        # Pair the controller
        if not self.pair_device(mac_address):
            return False

        # Configure input mapping
        # The DualSense shows up as a standard gamepad in SDL2
        print("PS5 controller configured")
        print("Controller mapping will be handled by SDL2")

        return True


def main():
    if len(sys.argv) < 2:
        print("HackDS Bluetooth Manager")
        print("\nUsage:")
        print("  bluetooth-manager enable         - Enable Bluetooth")
        print("  bluetooth-manager disable        - Disable Bluetooth")
        print("  bluetooth-manager scan           - Scan for devices")
        print("  bluetooth-manager pair <MAC>     - Pair with device")
        print("  bluetooth-manager unpair <MAC>   - Unpair device")
        print("  bluetooth-manager connect <MAC>  - Connect to device")
        print("  bluetooth-manager disconnect <MAC> - Disconnect device")
        print("  bluetooth-manager list           - List paired devices")
        print("  bluetooth-manager ps5-setup      - Interactive PS5 setup")
        return 1

    manager = BluetoothManager()
    command = sys.argv[1]

    if command == "enable":
        return 0 if manager.enable_bluetooth() else 1

    elif command == "disable":
        return 0 if manager.disable_bluetooth() else 1

    elif command == "scan":
        devices = manager.start_scan(10)
        print("\nFound devices:")
        for i, device in enumerate(devices, 1):
            print(f"{i}. {device['name']} ({device['type']})")
            print(f"   MAC: {device['mac']}")
        return 0

    elif command == "pair":
        if len(sys.argv) < 3:
            print("Error: MAC address required")
            return 1
        mac = sys.argv[2]
        return 0 if manager.pair_device(mac) else 1

    elif command == "unpair":
        if len(sys.argv) < 3:
            print("Error: MAC address required")
            return 1
        mac = sys.argv[2]
        return 0 if manager.unpair_device(mac) else 1

    elif command == "connect":
        if len(sys.argv) < 3:
            print("Error: MAC address required")
            return 1
        mac = sys.argv[2]
        return 0 if manager.connect_device(mac) else 1

    elif command == "disconnect":
        if len(sys.argv) < 3:
            print("Error: MAC address required")
            return 1
        mac = sys.argv[2]
        return 0 if manager.disconnect_device(mac) else 1

    elif command == "list":
        devices = manager.list_paired_devices()
        print("\nPaired devices:")
        for device in devices:
            print(f"- {device['name']}")
            print(f"  MAC: {device['mac']}")
        return 0

    elif command == "ps5-setup":
        print("=== PS5 DualSense Controller Setup ===")
        print("\n1. Put your PS5 controller in pairing mode:")
        print("   Hold PS + Share buttons until light flashes")
        print("\n2. Scanning for controllers...")

        manager.enable_bluetooth()
        time.sleep(2)

        devices = manager.start_scan(15)

        # Find DualSense controller
        ps5_controller = None
        for device in devices:
            if 'DualSense' in device['name'] or 'Wireless Controller' in device['name']:
                ps5_controller = device
                break

        if not ps5_controller:
            print("\nNo PS5 controller found!")
            print("Make sure it's in pairing mode and try again")
            return 1

        print(f"\nFound: {ps5_controller['name']}")
        print(f"MAC: {ps5_controller['mac']}")

        response = input("\nPair this controller? (yes/no): ")
        if response.lower() in ['yes', 'y']:
            if manager.setup_ps5_controller(ps5_controller['mac']):
                print("\n✓ PS5 controller paired successfully!")
                print("You can now use it to navigate HackDS")
                return 0
            else:
                print("\n✗ Failed to pair controller")
                return 1
        else:
            print("Cancelled")
            return 0

    else:
        print(f"Unknown command: {command}")
        return 1


if __name__ == '__main__':
    sys.exit(main())
