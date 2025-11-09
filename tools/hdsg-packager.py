#!/usr/bin/env python3
"""
HackDS Game Packager
Creates .hdsg game files from game directories
"""

import os
import sys
import json
import struct
import zlib
from pathlib import Path
from typing import Dict, List, Optional

# Magic numbers
MAGIC_HDSG = 0x47534448
MAGIC_HDSM = 0x4D534448
MAGIC_HDSS = 0x53534448
MAGIC_HDSH = 0x48534448

# Flags
FLAG_COMPRESSED = 1 << 0

class HDSGPackager:
    def __init__(self):
        self.version_major = 1
        self.version_minor = 0

    def create_hdsg(self, game_dir: str, output_file: str,
                    metadata: Dict, compress: bool = True) -> bool:
        """Create a .hdsg game file from a directory"""

        print(f"Packaging game: {metadata.get('name', 'Unknown')}")
        print(f"Source: {game_dir}")
        print(f"Output: {output_file}")

        # Validate metadata
        required_fields = ['name', 'version', 'author', 'engine', 'entrypoint']
        for field in required_fields:
            if field not in metadata:
                print(f"Error: Missing required field: {field}")
                return False

        # Serialize metadata
        metadata_json = json.dumps(metadata, indent=2).encode('utf-8')

        # Build payload from game directory
        payload = self._build_archive(game_dir)
        if not payload:
            print("Error: Failed to build archive")
            return False

        print(f"Archive size: {len(payload)} bytes")

        # Compress if requested
        flags = 0
        if compress:
            print("Compressing...")
            compressed = zlib.compress(payload, 9)
            print(f"Compressed size: {len(compressed)} bytes "
                  f"({100 * len(compressed) / len(payload):.1f}%)")
            payload = compressed
            flags = FLAG_COMPRESSED | (9 << 8)

        # Build header
        header = self._build_header(
            MAGIC_HDSG,
            flags,
            len(metadata_json),
            len(payload)
        )

        # Write file
        try:
            with open(output_file, 'wb') as f:
                f.write(header)
                f.write(metadata_json)
                f.write(payload)
            print(f"Successfully created: {output_file}")
            return True
        except Exception as e:
            print(f"Error writing file: {e}")
            return False

    def create_hdsm(self, mod_dir: str, output_file: str,
                    metadata: Dict, compress: bool = True) -> bool:
        """Create a .hdsm mod file"""
        return self._create_file(MAGIC_HDSM, mod_dir, output_file,
                                metadata, compress)

    def create_hdss(self, settings: Dict, output_file: str) -> bool:
        """Create a .hdss settings file"""
        metadata = {
            "type": "system",
            "version": "1.0.0"
        }

        metadata_json = json.dumps(metadata, indent=2).encode('utf-8')
        payload = json.dumps(settings, indent=2).encode('utf-8')

        header = self._build_header(MAGIC_HDSS, 0, len(metadata_json), len(payload))

        try:
            with open(output_file, 'wb') as f:
                f.write(header)
                f.write(metadata_json)
                f.write(payload)
            return True
        except Exception as e:
            print(f"Error writing file: {e}")
            return False

    def create_hdsh(self, hack_dir: str, output_file: str,
                    metadata: Dict, compress: bool = True) -> bool:
        """Create a .hdsh hack file"""
        return self._create_file(MAGIC_HDSH, hack_dir, output_file,
                                metadata, compress)

    def _create_file(self, magic: int, source_dir: str, output_file: str,
                     metadata: Dict, compress: bool) -> bool:
        """Generic file creation"""
        metadata_json = json.dumps(metadata, indent=2).encode('utf-8')
        payload = self._build_archive(source_dir)

        if not payload:
            return False

        flags = 0
        if compress:
            payload = zlib.compress(payload, 9)
            flags = FLAG_COMPRESSED | (9 << 8)

        header = self._build_header(magic, flags, len(metadata_json), len(payload))

        try:
            with open(output_file, 'wb') as f:
                f.write(header)
                f.write(metadata_json)
                f.write(payload)
            return True
        except Exception as e:
            print(f"Error: {e}")
            return False

    def _build_header(self, magic: int, flags: int,
                     metadata_size: int, payload_size: int) -> bytes:
        """Build the file header"""
        header = struct.pack('<IHHH',
            magic,
            self.version_major,
            self.version_minor,
            flags
        )

        # Calculate checksum (excluding the checksum field itself)
        header += struct.pack('<I', 0)  # Placeholder for checksum
        header += struct.pack('<IQQ',
            metadata_size,
            payload_size,
            0  # Reserved
        )

        # Calculate and update checksum
        checksum = zlib.crc32(header[:8] + header[12:])
        header = header[:8] + struct.pack('<I', checksum) + header[12:]

        return header

    def _build_archive(self, directory: str) -> Optional[bytes]:
        """Build archive payload from directory"""
        dir_path = Path(directory)
        if not dir_path.exists() or not dir_path.is_dir():
            print(f"Error: Directory not found: {directory}")
            return None

        # Collect all files
        files = []
        for file_path in dir_path.rglob('*'):
            if file_path.is_file():
                rel_path = file_path.relative_to(dir_path)
                files.append((str(rel_path), file_path))

        if not files:
            print("Error: No files found in directory")
            return None

        print(f"Found {len(files)} files")

        # Build file entries
        entries = b''
        file_data = b''
        current_offset = 0

        for rel_path, file_path in files:
            # Read file
            try:
                with open(file_path, 'rb') as f:
                    data = f.read()
            except Exception as e:
                print(f"Warning: Failed to read {file_path}: {e}")
                continue

            # Calculate CRC
            crc = zlib.crc32(data)

            # Build entry
            filename_bytes = rel_path.encode('utf-8')
            entry = struct.pack('<H', len(filename_bytes))
            entry += filename_bytes
            entry += struct.pack('<QQI',
                len(data),
                current_offset,
                crc
            )

            entries += entry
            file_data += data
            current_offset += len(data)

        # Calculate data offset (after all entries)
        data_offset = len(entries)

        # Update offsets in entries
        updated_entries = b''
        pos = 0

        while pos < len(entries):
            # Read entry
            name_len = struct.unpack('<H', entries[pos:pos+2])[0]
            pos += 2

            filename = entries[pos:pos+name_len]
            pos += name_len

            file_size, offset, crc = struct.unpack('<QQI', entries[pos:pos+20])
            pos += 20

            # Update offset
            offset += data_offset

            # Write updated entry
            updated_entries += struct.pack('<H', name_len)
            updated_entries += filename
            updated_entries += struct.pack('<QQI', file_size, offset, crc)

        return updated_entries + file_data


def main():
    if len(sys.argv) < 2:
        print("HackDS Game Packager")
        print("\nUsage:")
        print("  hdsg-packager.py <command> [arguments]")
        print("\nCommands:")
        print("  game <dir> <output.hdsg> <metadata.json>  - Package a game")
        print("  mod  <dir> <output.hdsm> <metadata.json>  - Package a mod")
        print("  hack <dir> <output.hdsh> <metadata.json>  - Package a hack")
        print("\nMetadata JSON format:")
        print('''{
  "name": "Game Name",
  "version": "1.0.0",
  "author": "Your Name",
  "description": "Game description",
  "engine": "python",
  "entrypoint": "main.py"
}''')
        return 1

    command = sys.argv[1].lower()
    packager = HDSGPackager()

    if command == 'game':
        if len(sys.argv) != 5:
            print("Usage: hdsg-packager.py game <dir> <output.hdsg> <metadata.json>")
            return 1

        game_dir = sys.argv[2]
        output = sys.argv[3]
        metadata_file = sys.argv[4]

        try:
            with open(metadata_file, 'r') as f:
                metadata = json.load(f)
        except Exception as e:
            print(f"Error reading metadata: {e}")
            return 1

        if packager.create_hdsg(game_dir, output, metadata):
            return 0
        else:
            return 1

    elif command == 'mod':
        if len(sys.argv) != 5:
            print("Usage: hdsg-packager.py mod <dir> <output.hdsm> <metadata.json>")
            return 1

        mod_dir = sys.argv[2]
        output = sys.argv[3]
        metadata_file = sys.argv[4]

        try:
            with open(metadata_file, 'r') as f:
                metadata = json.load(f)
        except Exception as e:
            print(f"Error reading metadata: {e}")
            return 1

        if packager.create_hdsm(mod_dir, output, metadata):
            return 0
        else:
            return 1

    elif command == 'hack':
        if len(sys.argv) != 5:
            print("Usage: hdsg-packager.py hack <dir> <output.hdsh> <metadata.json>")
            return 1

        hack_dir = sys.argv[2]
        output = sys.argv[3]
        metadata_file = sys.argv[4]

        try:
            with open(metadata_file, 'r') as f:
                metadata = json.load(f)
        except Exception as e:
            print(f"Error reading metadata: {e}")
            return 1

        if packager.create_hdsh(hack_dir, output, metadata):
            return 0
        else:
            return 1

    else:
        print(f"Unknown command: {command}")
        print("Valid commands: game, mod, hack")
        return 1


if __name__ == '__main__':
    sys.exit(main())
