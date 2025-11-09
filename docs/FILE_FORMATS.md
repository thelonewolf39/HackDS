# HackDS File Format Specifications

## Overview

All HackDS file formats use a common binary structure with a header, metadata section, and payload. This allows for efficient parsing and validation.

## Common Header Structure

All HackDS files begin with a common 32-byte header:

```
Offset | Size | Description
-------|------|------------
0x00   | 4    | Magic number (identifies file type)
0x04   | 2    | Format version (major.minor)
0x06   | 2    | Flags (compression, encryption, etc.)
0x08   | 4    | Header checksum (CRC32)
0x0C   | 4    | Metadata size in bytes
0x10   | 8    | Payload size in bytes
0x18   | 8    | Reserved for future use
```

### Magic Numbers

- **HDSG**: `0x47534448` ("HDSG" in ASCII)
- **HDSM**: `0x4D534448` ("HDSM" in ASCII)
- **HDSS**: `0x53534448` ("HDSS" in ASCII)
- **HDSH**: `0x48534448` ("HDSH" in ASCII)

### Flags (Bitfield)

```
Bit 0: Compressed (0=no, 1=yes, using zlib)
Bit 1: Encrypted (reserved for future use)
Bit 2-7: Reserved
Bit 8-15: Compression level (0-9 for zlib)
```

## .hdsg - Game File Format

### Metadata Structure (JSON)

After the common header, the metadata section contains JSON with game information:

```json
{
  "name": "Game Name",
  "version": "1.0.0",
  "author": "Author Name",
  "description": "Game description",
  "engine": "python" | "cpp" | "hybrid",
  "entrypoint": "main.py" | "game.bin",
  "icon": "icon.png",
  "resolution": [640, 480],
  "requires": {
    "python_version": ">=3.7",
    "libraries": ["pygame", "numpy"]
  },
  "assets": {
    "count": 123,
    "total_size": 1048576
  }
}
```

### Payload Structure

The payload contains the actual game files in a custom archive format:

```
[File Entry 1]
[File Entry 2]
...
[File Entry N]
[File Data]
```

Each file entry:
```
Offset | Size | Description
-------|------|------------
0x00   | 2    | Filename length
0x02   | N    | Filename (UTF-8)
N+2    | 8    | File size
N+10   | 8    | File offset from start of payload
N+18   | 4    | File CRC32
```

## .hdsm - Mod File Format

### Metadata Structure (JSON)

```json
{
  "name": "Mod Name",
  "version": "1.0.0",
  "author": "Author Name",
  "description": "Mod description",
  "target_game": "game_identifier",
  "target_version": ">=1.0.0",
  "mod_type": "asset" | "code" | "both",
  "priority": 100,
  "overwrites": ["file1.png", "script.py"],
  "patches": [
    {
      "file": "config.json",
      "type": "json_merge" | "replace" | "patch"
    }
  ]
}
```

### Payload Structure

Similar to .hdsg, contains modified or additional files.

## .hdss - Settings File Format

### Metadata Structure (JSON)

```json
{
  "type": "system" | "game",
  "game_id": "game_identifier",
  "version": "1.0.0",
  "last_modified": "2025-01-09T12:00:00Z"
}
```

### Payload Structure

The payload contains a JSON or binary configuration:

```json
{
  "graphics": {
    "resolution": [640, 480],
    "fullscreen": true,
    "vsync": true
  },
  "audio": {
    "master_volume": 0.8,
    "sfx_volume": 0.7,
    "music_volume": 0.6
  },
  "controls": {
    "up": "w",
    "down": "s",
    "left": "a",
    "right": "d"
  },
  "game_specific": {}
}
```

## .hdsh - Hacks File Format

### Metadata Structure (JSON)

```json
{
  "name": "Hack Name",
  "version": "1.0.0",
  "author": "Author Name",
  "description": "Hack description",
  "target_game": "game_identifier",
  "target_version": ">=1.0.0",
  "hack_type": "cheat" | "trainer" | "patch",
  "codes": [
    {
      "name": "Infinite Lives",
      "description": "Never run out of lives",
      "enabled": false,
      "type": "memory" | "file" | "code_injection"
    }
  ]
}
```

### Payload Structure

Contains hack code/patches:

```
[Hack Entry 1]
[Hack Entry 2]
...

Each hack entry:
- Type identifier (1 byte)
- Target address/file (variable)
- Original value (for verification)
- New value/code
```

## File Creation Example (Python)

```python
import struct
import json
import zlib

def create_hdsg(metadata, files, output_path, compress=True):
    # Serialize metadata
    metadata_json = json.dumps(metadata, indent=2).encode('utf-8')

    # Build payload
    payload = build_archive(files)

    # Compress if requested
    flags = 0
    if compress:
        flags |= 0x01 | (9 << 8)  # Compressed + level 9
        payload = zlib.compress(payload, 9)

    # Build header
    magic = 0x47534448  # HDSG
    version = (1 << 8) | 0  # 1.0
    metadata_size = len(metadata_json)
    payload_size = len(payload)

    header = struct.pack('<IHHIIQQ',
        magic, version, flags, 0,  # checksum calculated later
        metadata_size, payload_size, 0
    )

    # Calculate header checksum
    checksum = zlib.crc32(header[:8] + header[12:])
    header = struct.pack('<IHHIIQQ',
        magic, version, flags, checksum,
        metadata_size, payload_size, 0
    )

    # Write file
    with open(output_path, 'wb') as f:
        f.write(header)
        f.write(metadata_json)
        f.write(payload)
```

## Validation

All files should be validated on load:
1. Check magic number matches expected type
2. Verify header checksum
3. Validate metadata JSON schema
4. Check payload size matches header
5. Verify file integrity (CRC32 for each archived file)
