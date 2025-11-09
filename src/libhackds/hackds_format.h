/*
 * HackDS File Format Library
 * Header file for parsing .hdsg, .hdsm, .hdss, .hdsh files
 */

#ifndef HACKDS_FORMAT_H
#define HACKDS_FORMAT_H

#include <stdint.h>
#include <stdbool.h>

#define HACKDS_VERSION_MAJOR 1
#define HACKDS_VERSION_MINOR 0

// Magic numbers
#define MAGIC_HDSG 0x47534448  // "HDSG"
#define MAGIC_HDSM 0x4D534448  // "HDSM"
#define MAGIC_HDSS 0x53534448  // "HDSS"
#define MAGIC_HDSH 0x48534448  // "HDSH"

// Flags
#define FLAG_COMPRESSED (1 << 0)
#define FLAG_ENCRYPTED  (1 << 1)

// File types
typedef enum {
    HACKDS_TYPE_GAME = 0,
    HACKDS_TYPE_MOD,
    HACKDS_TYPE_SETTINGS,
    HACKDS_TYPE_HACK,
    HACKDS_TYPE_UNKNOWN
} hackds_file_type_t;

// Common header structure (32 bytes)
typedef struct __attribute__((packed)) {
    uint32_t magic;           // Magic number
    uint16_t version_major;   // Format version major
    uint16_t version_minor;   // Format version minor
    uint16_t flags;           // Flags (compression, etc.)
    uint16_t reserved1;
    uint32_t header_crc;      // Header checksum
    uint32_t metadata_size;   // Metadata size in bytes
    uint64_t payload_size;    // Payload size in bytes
    uint64_t reserved2;
} hackds_header_t;

// File entry in archive
typedef struct {
    char *filename;
    uint64_t size;
    uint64_t offset;
    uint32_t crc32;
    uint8_t *data;  // Loaded on demand
} hackds_file_entry_t;

// Main file structure
typedef struct {
    hackds_file_type_t type;
    hackds_header_t header;
    char *metadata;           // JSON metadata
    uint8_t *payload;         // Raw payload data
    hackds_file_entry_t *files;  // Archived files
    size_t file_count;
    bool loaded;
} hackds_file_t;

// Function prototypes

// Open and parse a HackDS file
hackds_file_t* hackds_open(const char *path);

// Close and free a HackDS file
void hackds_close(hackds_file_t *file);

// Validate file integrity
bool hackds_validate(hackds_file_t *file);

// Get file type from magic number
hackds_file_type_t hackds_get_type(uint32_t magic);

// Extract a specific file from the archive
int hackds_extract_file(hackds_file_t *file, const char *filename,
                        uint8_t **data, size_t *size);

// Extract all files to a directory
int hackds_extract_all(hackds_file_t *file, const char *dest_dir);

// Get metadata as JSON string
const char* hackds_get_metadata(hackds_file_t *file);

// Parse metadata to get specific fields
char* hackds_get_metadata_field(hackds_file_t *file, const char *field);

// List all files in the archive
int hackds_list_files(hackds_file_t *file, char ***filenames, size_t *count);

// CRC32 calculation
uint32_t hackds_crc32(const uint8_t *data, size_t length);

// Compression/decompression
int hackds_decompress(const uint8_t *in, size_t in_size,
                      uint8_t **out, size_t *out_size);

int hackds_compress(const uint8_t *in, size_t in_size,
                    uint8_t **out, size_t *out_size, int level);

// Error handling
const char* hackds_get_error(void);

#endif // HACKDS_FORMAT_H
