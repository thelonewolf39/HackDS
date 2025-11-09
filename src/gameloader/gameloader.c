/*
 * HackDS Game Loader
 * Loads and executes .hdsg game files
 */

#include "../libhackds/hackds_format.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>

#define TEMP_DIR "/tmp/hackds_game"
#define MAX_PATH 512

typedef struct {
    char name[256];
    char version[32];
    char author[128];
    char engine[32];
    char entrypoint[256];
} game_metadata_t;

static int parse_metadata(const char *json, game_metadata_t *meta);
static int extract_game(hackds_file_t *game, const char *dest);
static int run_python_game(const char *game_dir, const char *entrypoint);
static int run_cpp_game(const char *game_dir, const char *entrypoint);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <game.hdsg>\n", argv[0]);
        return 1;
    }

    const char *game_path = argv[1];

    printf("HackDS Game Loader\n");
    printf("Loading: %s\n", game_path);

    // Open the game file
    hackds_file_t *game = hackds_open(game_path);
    if (!game) {
        fprintf(stderr, "Error: %s\n", hackds_get_error());
        return 1;
    }

    // Verify it's a game file
    if (game->type != HACKDS_TYPE_GAME) {
        fprintf(stderr, "Error: Not a game file\n");
        hackds_close(game);
        return 1;
    }

    // Parse metadata
    game_metadata_t meta;
    if (parse_metadata(hackds_get_metadata(game), &meta) != 0) {
        fprintf(stderr, "Error: Failed to parse game metadata\n");
        hackds_close(game);
        return 1;
    }

    printf("Game: %s v%s\n", meta.name, meta.version);
    printf("Author: %s\n", meta.author);
    printf("Engine: %s\n", meta.engine);

    // Create temporary directory
    mkdir(TEMP_DIR, 0755);

    // Extract game files
    printf("Extracting game files...\n");
    if (extract_game(game, TEMP_DIR) != 0) {
        fprintf(stderr, "Error: Failed to extract game\n");
        hackds_close(game);
        return 1;
    }

    hackds_close(game);

    // Run the game based on engine type
    int result = 0;
    if (strcmp(meta.engine, "python") == 0) {
        result = run_python_game(TEMP_DIR, meta.entrypoint);
    } else if (strcmp(meta.engine, "cpp") == 0) {
        result = run_cpp_game(TEMP_DIR, meta.entrypoint);
    } else {
        fprintf(stderr, "Error: Unsupported engine: %s\n", meta.engine);
        result = 1;
    }

    // Cleanup
    printf("Cleaning up...\n");
    system("rm -rf " TEMP_DIR);

    return result;
}

static int parse_metadata(const char *json, game_metadata_t *meta) {
    if (!json || !meta) return -1;

    // Simple JSON parsing (you'd want a proper JSON library in production)
    // For now, we'll do basic string parsing

    const char *name_start = strstr(json, "\"name\"");
    if (name_start) {
        name_start = strchr(name_start, ':');
        if (name_start) {
            name_start = strchr(name_start, '"');
            if (name_start) {
                name_start++;
                const char *name_end = strchr(name_start, '"');
                if (name_end) {
                    size_t len = name_end - name_start;
                    if (len < sizeof(meta->name)) {
                        strncpy(meta->name, name_start, len);
                        meta->name[len] = '\0';
                    }
                }
            }
        }
    }

    const char *version_start = strstr(json, "\"version\"");
    if (version_start) {
        version_start = strchr(version_start, ':');
        if (version_start) {
            version_start = strchr(version_start, '"');
            if (version_start) {
                version_start++;
                const char *version_end = strchr(version_start, '"');
                if (version_end) {
                    size_t len = version_end - version_start;
                    if (len < sizeof(meta->version)) {
                        strncpy(meta->version, version_start, len);
                        meta->version[len] = '\0';
                    }
                }
            }
        }
    }

    const char *author_start = strstr(json, "\"author\"");
    if (author_start) {
        author_start = strchr(author_start, ':');
        if (author_start) {
            author_start = strchr(author_start, '"');
            if (author_start) {
                author_start++;
                const char *author_end = strchr(author_start, '"');
                if (author_end) {
                    size_t len = author_end - author_start;
                    if (len < sizeof(meta->author)) {
                        strncpy(meta->author, author_start, len);
                        meta->author[len] = '\0';
                    }
                }
            }
        }
    }

    const char *engine_start = strstr(json, "\"engine\"");
    if (engine_start) {
        engine_start = strchr(engine_start, ':');
        if (engine_start) {
            engine_start = strchr(engine_start, '"');
            if (engine_start) {
                engine_start++;
                const char *engine_end = strchr(engine_start, '"');
                if (engine_end) {
                    size_t len = engine_end - engine_start;
                    if (len < sizeof(meta->engine)) {
                        strncpy(meta->engine, engine_start, len);
                        meta->engine[len] = '\0';
                    }
                }
            }
        }
    }

    const char *entry_start = strstr(json, "\"entrypoint\"");
    if (entry_start) {
        entry_start = strchr(entry_start, ':');
        if (entry_start) {
            entry_start = strchr(entry_start, '"');
            if (entry_start) {
                entry_start++;
                const char *entry_end = strchr(entry_start, '"');
                if (entry_end) {
                    size_t len = entry_end - entry_start;
                    if (len < sizeof(meta->entrypoint)) {
                        strncpy(meta->entrypoint, entry_start, len);
                        meta->entrypoint[len] = '\0';
                    }
                }
            }
        }
    }

    return 0;
}

static int extract_game(hackds_file_t *game, const char *dest) {
    char **filenames;
    size_t count;

    if (hackds_list_files(game, &filenames, &count) != 0) {
        return -1;
    }

    for (size_t i = 0; i < count; i++) {
        uint8_t *data;
        size_t size;

        if (hackds_extract_file(game, filenames[i], &data, &size) != 0) {
            fprintf(stderr, "Failed to extract: %s\n", filenames[i]);
            continue;
        }

        // Create full path
        char path[MAX_PATH];
        snprintf(path, sizeof(path), "%s/%s", dest, filenames[i]);

        // Create parent directories if needed
        char *slash = strrchr(path, '/');
        if (slash) {
            *slash = '\0';
            char cmd[MAX_PATH + 20];
            snprintf(cmd, sizeof(cmd), "mkdir -p \"%s\"", path);
            system(cmd);
            *slash = '/';
        }

        // Write file
        FILE *fp = fopen(path, "wb");
        if (fp) {
            fwrite(data, 1, size, fp);
            fclose(fp);
        } else {
            fprintf(stderr, "Failed to write: %s\n", path);
        }

        free(data);
        free(filenames[i]);
    }

    free(filenames);
    return 0;
}

static int run_python_game(const char *game_dir, const char *entrypoint) {
    printf("Starting Python game...\n");

    char path[MAX_PATH];
    snprintf(path, sizeof(path), "%s/%s", game_dir, entrypoint);

    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Fork failed: %s\n", strerror(errno));
        return 1;
    }

    if (pid == 0) {
        // Child process
        chdir(game_dir);

        char *args[] = {"python3", (char*)entrypoint, NULL};
        char *env[] = {
            "PYTHONPATH=/system/lib/python3.11",
            "LD_LIBRARY_PATH=/system/lib",
            NULL
        };

        execve("/system/bin/python3", args, env);

        fprintf(stderr, "Failed to execute Python: %s\n", strerror(errno));
        exit(1);
    }

    // Parent process - wait for game to finish
    int status;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }

    return 1;
}

static int run_cpp_game(const char *game_dir, const char *entrypoint) {
    printf("Starting C++ game...\n");

    char path[MAX_PATH];
    snprintf(path, sizeof(path), "%s/%s", game_dir, entrypoint);

    // Make executable
    chmod(path, 0755);

    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Fork failed: %s\n", strerror(errno));
        return 1;
    }

    if (pid == 0) {
        // Child process
        chdir(game_dir);

        char *args[] = {(char*)entrypoint, NULL};
        char *env[] = {
            "LD_LIBRARY_PATH=/system/lib",
            NULL
        };

        execve(path, args, env);

        fprintf(stderr, "Failed to execute game: %s\n", strerror(errno));
        exit(1);
    }

    // Parent process - wait for game to finish
    int status;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }

    return 1;
}
