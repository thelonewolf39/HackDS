#!/usr/bin/env python3
"""
HackDS WiFi/Network Manager
Manages WiFi connections for internet access
"""

import os
import sys
import subprocess
import time
import re
from typing import List, Dict, Optional

class WiFiManager:
    def __init__(self):
        self.interface = 'wlan0'
        self.connected_ssid = None

    def check_wifi_available(self) -> bool:
        """Check if WiFi hardware is available"""
        try:
            result = subprocess.run(['iw', 'dev'], capture_output=True, text=True)
            return self.interface in result.stdout
        except FileNotFoundError:
            print("iw tools not installed")
            return False

    def enable_wifi(self) -> bool:
        """Enable WiFi interface"""
        try:
            subprocess.run(['sudo', 'ip', 'link', 'set', self.interface, 'up'], check=True)
            print(f"WiFi enabled on {self.interface}")
            return True
        except subprocess.CalledProcessError as e:
            print(f"Failed to enable WiFi: {e}")
            return False

    def disable_wifi(self) -> bool:
        """Disable WiFi interface"""
        try:
            subprocess.run(['sudo', 'ip', 'link', 'set', self.interface, 'down'], check=True)
            print(f"WiFi disabled on {self.interface}")
            return True
        except subprocess.CalledProcessError as e:
            print(f"Failed to disable WiFi: {e}")
            return False

    def scan_networks(self) -> List[Dict]:
        """Scan for available WiFi networks"""
        print("Scanning for WiFi networks...")

        try:
            # Trigger scan
            subprocess.run(['sudo', 'iw', self.interface, 'scan'],
                          capture_output=True, check=True)

            time.sleep(2)

            # Get scan results
            result = subprocess.run(
                ['sudo', 'iw', self.interface, 'scan'],
                capture_output=True,
                text=True,
                check=True
            )

            networks = []
            current_network = {}

            for line in result.stdout.splitlines():
                line = line.strip()

                if line.startswith('BSS'):
                    if current_network:
                        networks.append(current_network)
                    current_network = {
                        'bssid': line.split()[1].rstrip(':'),
                        'ssid': '',
                        'signal': 0,
                        'frequency': 0,
                        'encrypted': False
                    }

                elif 'SSID:' in line:
                    current_network['ssid'] = line.split('SSID: ')[1]

                elif 'signal:' in line:
                    signal_str = line.split('signal: ')[1].split()[0]
                    current_network['signal'] = float(signal_str)

                elif 'freq:' in line:
                    freq_str = line.split('freq: ')[1]
                    current_network['frequency'] = int(freq_str)

                elif 'WPA' in line or 'RSN' in line:
                    current_network['encrypted'] = True

            if current_network:
                networks.append(current_network)

            # Filter out empty SSIDs and sort by signal strength
            networks = [n for n in networks if n['ssid']]
            networks.sort(key=lambda x: x['signal'], reverse=True)

            print(f"Found {len(networks)} networks")
            return networks

        except Exception as e:
            print(f"Error scanning: {e}")
            return []

    def connect_network(self, ssid: str, password: Optional[str] = None) -> bool:
        """Connect to a WiFi network"""
        print(f"Connecting to {ssid}...")

        # Create wpa_supplicant configuration
        config_file = '/tmp/wpa_supplicant.conf'

        try:
            with open(config_file, 'w') as f:
                f.write('ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev\n')
                f.write('update_config=1\n')
                f.write('country=US\n\n')
                f.write('network={\n')
                f.write(f'    ssid="{ssid}"\n')

                if password:
                    # Generate PSK
                    psk_result = subprocess.run(
                        ['wpa_passphrase', ssid, password],
                        capture_output=True,
                        text=True,
                        check=True
                    )

                    # Extract PSK from output
                    for line in psk_result.stdout.splitlines():
                        if 'psk=' in line and '#psk=' not in line:
                            f.write(f'    {line.strip()}\n')
                else:
                    f.write('    key_mgmt=NONE\n')

                f.write('}\n')

            # Kill existing wpa_supplicant
            subprocess.run(['sudo', 'killall', 'wpa_supplicant'],
                          capture_output=True)
            time.sleep(1)

            # Start wpa_supplicant
            subprocess.Popen(
                ['sudo', 'wpa_supplicant', '-B', '-i', self.interface,
                 '-c', config_file],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL
            )

            time.sleep(3)

            # Get IP address via DHCP
            subprocess.run(['sudo', 'dhclient', self.interface],
                          capture_output=True, timeout=15)

            # Check if connected
            if self.check_connection():
                self.connected_ssid = ssid
                print(f"Connected to {ssid}")

                # Save to persistent config
                self._save_network_config(ssid, password)

                return True
            else:
                print("Failed to get IP address")
                return False

        except Exception as e:
            print(f"Error connecting: {e}")
            return False

    def disconnect(self) -> bool:
        """Disconnect from current network"""
        print("Disconnecting from WiFi...")

        try:
            subprocess.run(['sudo', 'killall', 'wpa_supplicant'],
                          capture_output=True)
            subprocess.run(['sudo', 'killall', 'dhclient'],
                          capture_output=True)
            subprocess.run(['sudo', 'ip', 'addr', 'flush', 'dev', self.interface],
                          check=True)

            self.connected_ssid = None
            print("Disconnected")
            return True

        except Exception as e:
            print(f"Error disconnecting: {e}")
            return False

    def check_connection(self) -> bool:
        """Check if connected to internet"""
        try:
            # Check if we have an IP address
            result = subprocess.run(
                ['ip', 'addr', 'show', self.interface],
                capture_output=True,
                text=True,
                check=True
            )

            if 'inet ' not in result.stdout:
                return False

            # Try to ping Google DNS
            subprocess.run(
                ['ping', '-c', '1', '-W', '2', '8.8.8.8'],
                capture_output=True,
                check=True,
                timeout=3
            )

            return True

        except Exception:
            return False

    def get_current_network(self) -> Optional[str]:
        """Get currently connected network SSID"""
        try:
            result = subprocess.run(
                ['iw', self.interface, 'info'],
                capture_output=True,
                text=True,
                check=True
            )

            for line in result.stdout.splitlines():
                if 'ssid' in line.lower():
                    ssid = line.split('ssid ')[1].strip()
                    self.connected_ssid = ssid
                    return ssid

            return None

        except Exception:
            return None

    def get_ip_address(self) -> Optional[str]:
        """Get current IP address"""
        try:
            result = subprocess.run(
                ['ip', 'addr', 'show', self.interface],
                capture_output=True,
                text=True,
                check=True
            )

            for line in result.stdout.splitlines():
                if 'inet ' in line:
                    ip = line.strip().split()[1].split('/')[0]
                    return ip

            return None

        except Exception:
            return None

    def _save_network_config(self, ssid: str, password: Optional[str]):
        """Save network config to persistent storage"""
        config_dir = '/settings/wifi'
        config_file = f'{config_dir}/wpa_supplicant.conf'

        try:
            os.makedirs(config_dir, exist_ok=True)

            with open(config_file, 'w') as f:
                f.write('ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev\n')
                f.write('update_config=1\n')
                f.write('country=US\n\n')
                f.write('network={\n')
                f.write(f'    ssid="{ssid}"\n')

                if password:
                    psk_result = subprocess.run(
                        ['wpa_passphrase', ssid, password],
                        capture_output=True,
                        text=True,
                        check=True
                    )

                    for line in psk_result.stdout.splitlines():
                        if 'psk=' in line and '#psk=' not in line:
                            f.write(f'    {line.strip()}\n')
                else:
                    f.write('    key_mgmt=NONE\n')

                f.write('}\n')

            print(f"Network config saved to {config_file}")

        except Exception as e:
            print(f"Warning: Could not save config: {e}")

    def load_saved_network(self) -> bool:
        """Load and connect to saved network"""
        config_file = '/settings/wifi/wpa_supplicant.conf'

        if not os.path.exists(config_file):
            return False

        try:
            # Kill existing wpa_supplicant
            subprocess.run(['sudo', 'killall', 'wpa_supplicant'],
                          capture_output=True)
            time.sleep(1)

            # Start wpa_supplicant with saved config
            subprocess.Popen(
                ['sudo', 'wpa_supplicant', '-B', '-i', self.interface,
                 '-c', config_file],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL
            )

            time.sleep(3)

            # Get IP address
            subprocess.run(['sudo', 'dhclient', self.interface],
                          capture_output=True, timeout=15)

            return self.check_connection()

        except Exception as e:
            print(f"Error loading saved network: {e}")
            return False


def main():
    if len(sys.argv) < 2:
        print("HackDS WiFi Manager")
        print("\nUsage:")
        print("  wifi-manager enable              - Enable WiFi")
        print("  wifi-manager disable             - Disable WiFi")
        print("  wifi-manager scan                - Scan for networks")
        print("  wifi-manager connect <SSID> [password] - Connect to network")
        print("  wifi-manager disconnect          - Disconnect from network")
        print("  wifi-manager status              - Show connection status")
        print("  wifi-manager auto-connect        - Connect to saved network")
        return 1

    manager = WiFiManager()
    command = sys.argv[1]

    if command == "enable":
        return 0 if manager.enable_wifi() else 1

    elif command == "disable":
        return 0 if manager.disable_wifi() else 1

    elif command == "scan":
        networks = manager.scan_networks()
        print("\nAvailable networks:")
        for i, network in enumerate(networks, 1):
            security = "🔒" if network['encrypted'] else "  "
            signal_bars = "▂▄▆█"[min(3, max(0, int((network['signal'] + 100) / 25)))]
            print(f"{i}. {security} {network['ssid']:<32} {signal_bars} {network['signal']:.0f} dBm")
        return 0

    elif command == "connect":
        if len(sys.argv) < 3:
            print("Error: SSID required")
            return 1

        ssid = sys.argv[2]
        password = sys.argv[3] if len(sys.argv) > 3 else None

        return 0 if manager.connect_network(ssid, password) else 1

    elif command == "disconnect":
        return 0 if manager.disconnect() else 1

    elif command == "status":
        connected = manager.check_connection()
        ssid = manager.get_current_network()
        ip = manager.get_ip_address()

        print("\nWiFi Status:")
        print(f"Connected: {'Yes' if connected else 'No'}")
        if ssid:
            print(f"Network: {ssid}")
        if ip:
            print(f"IP Address: {ip}")

        return 0

    elif command == "auto-connect":
        print("Connecting to saved network...")
        return 0 if manager.load_saved_network() else 1

    else:
        print(f"Unknown command: {command}")
        return 1


if __name__ == '__main__':
    sys.exit(main())
