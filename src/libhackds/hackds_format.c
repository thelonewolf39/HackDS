/*
 * HackDS File Format Library
 * Implementation
 */

#include "hackds_format.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <sys/stat.h>
#include <errno.h>

static char error_buffer[256] = {0};

static void set_error(const char *msg) {
    snprintf(error_buffer, sizeof(error_buffer), "%s", msg);
}

const char* hackds_get_error(void) {
    return error_buffer;
}

// CRC32 calculation using zlib
uint32_t hackds_crc32(const uint8_t *data, size_t length) {
    return crc32(0L, data, length);
}

hackds_file_type_t hackds_get_type(uint32_t magic) {
    switch (magic) {
        case MAGIC_HDSG: return HACKDS_TYPE_GAME;
        case MAGIC_HDSM: return HACKDS_TYPE_MOD;
        case MAGIC_HDSS: return HACKDS_TYPE_SETTINGS;
        case MAGIC_HDSH: return HACKDS_TYPE_HACK;
        default: return HACKDS_TYPE_UNKNOWN;
    }
}

hackds_file_t* hackds_open(const char *path) {
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        set_error("Failed to open file");
        return NULL;
    }

    // Allocate file structure
    hackds_file_t *file = calloc(1, sizeof(hackds_file_t));
    if (!file) {
        set_error("Memory allocation failed");
        fclose(fp);
        return NULL;
    }

    // Read header
    if (fread(&file->header, sizeof(hackds_header_t), 1, fp) != 1) {
        set_error("Failed to read header");
        free(file);
        fclose(fp);
        return NULL;
    }

    // Validate magic number
    file->type = hackds_get_type(file->header.magic);
    if (file->type == HACKDS_TYPE_UNKNOWN) {
        set_error("Invalid magic number");
        free(file);
        fclose(fp);
        return NULL;
    }

    // Validate version
    if (file->header.version_major != HACKDS_VERSION_MAJOR) {
        set_error("Unsupported format version");
        free(file);
        fclose(fp);
        return NULL;
    }

    // Validate header checksum
    uint32_t saved_crc = file->header.header_crc;
    file->header.header_crc = 0;
    uint32_t calc_crc = hackds_crc32((uint8_t*)&file->header, sizeof(hackds_header_t));
    file->header.header_crc = saved_crc;

    if (calc_crc != saved_crc) {
        set_error("Header checksum mismatch");
        free(file);
        fclose(fp);
        return NULL;
    }

    // Read metadata
    if (file->header.metadata_size > 0) {
        file->metadata = malloc(file->header.metadata_size + 1);
        if (!file->metadata) {
            set_error("Memory allocation failed");
            free(file);
            fclose(fp);
            return NULL;
        }

        if (fread(file->metadata, file->header.metadata_size, 1, fp) != 1) {
            set_error("Failed to read metadata");
            free(file->metadata);
            free(file);
            fclose(fp);
            return NULL;
        }

        file->metadata[file->header.metadata_size] = '\0';
    }

    // Read payload
    if (file->header.payload_size > 0) {
        file->payload = malloc(file->header.payload_size);
        if (!file->payload) {
            set_error("Memory allocation failed");
            free(file->metadata);
            free(file);
            fclose(fp);
            return NULL;
        }

        if (fread(file->payload, file->header.payload_size, 1, fp) != 1) {
            set_error("Failed to read payload");
            free(file->payload);
            free(file->metadata);
            free(file);
            fclose(fp);
            return NULL;
        }

        // Decompress if needed
        if (file->header.flags & FLAG_COMPRESSED) {
            uint8_t *decompressed = NULL;
            size_t decompressed_size = 0;

            if (hackds_decompress(file->payload, file->header.payload_size,
                                  &decompressed, &decompressed_size) != 0) {
                set_error("Decompression failed");
                free(file->payload);
                free(file->metadata);
                free(file);
                fclose(fp);
                return NULL;
            }

            free(file->payload);
            file->payload = decompressed;
            file->header.payload_size = decompressed_size;
        }
    }

    fclose(fp);
    file->loaded = true;

    return file;
}

void hackds_close(hackds_file_t *file) {
    if (!file) return;

    if (file->metadata) free(file->metadata);
    if (file->payload) free(file->payload);

    if (file->files) {
        for (size_t i = 0; i < file->file_count; i++) {
            if (file->files[i].filename) free(file->files[i].filename);
            if (file->files[i].data) free(file->files[i].data);
        }
        free(file->files);
    }

    free(file);
}

bool hackds_validate(hackds_file_t *file) {
    if (!file || !file->loaded) return false;

    // Header was already validated during load
    // Could add additional validation here

    return true;
}

const char* hackds_get_metadata(hackds_file_t *file) {
    if (!file) return NULL;
    return file->metadata;
}

int hackds_decompress(const uint8_t *in, size_t in_size,
                      uint8_t **out, size_t *out_size) {
    // Start with a reasonable output size
    size_t buf_size = in_size * 4;
    uint8_t *buffer = malloc(buf_size);
    if (!buffer) return -1;

    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.avail_in = in_size;
    stream.next_in = (Bytef*)in;
    stream.avail_out = buf_size;
    stream.next_out = buffer;

    if (inflateInit(&stream) != Z_OK) {
        free(buffer);
        return -1;
    }

    int ret = inflate(&stream, Z_FINISH);
    if (ret != Z_STREAM_END) {
        inflateEnd(&stream);
        free(buffer);
        return -1;
    }

    *out_size = stream.total_out;
    *out = realloc(buffer, *out_size);
    if (!*out) *out = buffer;

    inflateEnd(&stream);
    return 0;
}

int hackds_compress(const uint8_t *in, size_t in_size,
                    uint8_t **out, size_t *out_size, int level) {
    size_t buf_size = compressBound(in_size);
    uint8_t *buffer = malloc(buf_size);
    if (!buffer) return -1;

    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.avail_in = in_size;
    stream.next_in = (Bytef*)in;
    stream.avail_out = buf_size;
    stream.next_out = buffer;

    if (deflateInit(&stream, level) != Z_OK) {
        free(buffer);
        return -1;
    }

    int ret = deflate(&stream, Z_FINISH);
    if (ret != Z_STREAM_END) {
        deflateEnd(&stream);
        free(buffer);
        return -1;
    }

    *out_size = stream.total_out;
    *out = realloc(buffer, *out_size);
    if (!*out) *out = buffer;

    deflateEnd(&stream);
    return 0;
}

// Parse archive structure from payload
static int parse_archive(hackds_file_t *file) {
    if (!file || !file->payload) return -1;

    uint8_t *ptr = file->payload;
    uint8_t *end = ptr + file->header.payload_size;

    // Count files first
    uint8_t *scan = ptr;
    size_t count = 0;

    while (scan < end) {
        uint16_t name_len;
        memcpy(&name_len, scan, 2);
        scan += 2 + name_len + 8 + 8 + 4;  // name + size + offset + crc
        if (scan > end) break;
        count++;
    }

    file->file_count = count;
    file->files = calloc(count, sizeof(hackds_file_entry_t));
    if (!file->files) return -1;

    // Parse entries
    for (size_t i = 0; i < count && ptr < end; i++) {
        uint16_t name_len;
        memcpy(&name_len, ptr, 2);
        ptr += 2;

        file->files[i].filename = malloc(name_len + 1);
        memcpy(file->files[i].filename, ptr, name_len);
        file->files[i].filename[name_len] = '\0';
        ptr += name_len;

        memcpy(&file->files[i].size, ptr, 8);
        ptr += 8;

        memcpy(&file->files[i].offset, ptr, 8);
        ptr += 8;

        memcpy(&file->files[i].crc32, ptr, 4);
        ptr += 4;
    }

    return 0;
}

int hackds_extract_file(hackds_file_t *file, const char *filename,
                        uint8_t **data, size_t *size) {
    if (!file || !filename || !data || !size) return -1;

    // Parse archive if not done yet
    if (!file->files && parse_archive(file) != 0) {
        return -1;
    }

    // Find file
    for (size_t i = 0; i < file->file_count; i++) {
        if (strcmp(file->files[i].filename, filename) == 0) {
            *size = file->files[i].size;
            *data = malloc(*size);
            if (!*data) return -1;

            // Copy data from payload
            memcpy(*data, file->payload + file->files[i].offset, *size);
            return 0;
        }
    }

    return -1;  // File not found
}

int hackds_list_files(hackds_file_t *file, char ***filenames, size_t *count) {
    if (!file || !filenames || !count) return -1;

    if (!file->files && parse_archive(file) != 0) {
        return -1;
    }

    *count = file->file_count;
    *filenames = malloc(sizeof(char*) * file->file_count);
    if (!*filenames) return -1;

    for (size_t i = 0; i < file->file_count; i++) {
        (*filenames)[i] = strdup(file->files[i].filename);
    }

    return 0;
}
