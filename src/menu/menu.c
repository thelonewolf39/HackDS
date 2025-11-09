/*
 * HackDS GUI Menu System
 * Main menu interface for game selection
 */

#include "../libhackds/hackds_format.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define GAME_DIR "/games"
#define SETTINGS_FILE "/settings/system.hdss"

#define COLOR_BG      {20, 20, 30, 255}
#define COLOR_TEXT    {220, 220, 220, 255}
#define COLOR_SELECTED {100, 150, 255, 255}
#define COLOR_ACCENT  {60, 120, 220, 255}

typedef struct {
    char path[512];
    char name[256];
    char version[32];
    char author[128];
    SDL_Texture *icon;
} game_entry_t;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font_large;
    TTF_Font *font_small;
    TTF_Font *font_tiny;
    game_entry_t *games;
    int game_count;
    int selected_index;
    int scroll_offset;
    int update_available;
    char update_version[32];
} menu_state_t;

static int scan_games(menu_state_t *state);
static void render_menu(menu_state_t *state);
static void render_text(SDL_Renderer *renderer, TTF_Font *font,
                       const char *text, int x, int y, SDL_Color color);
static int launch_game(const char *game_path);
static void cleanup(menu_state_t *state);
static void check_for_updates(menu_state_t *state);
static void trigger_update(void);

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    printf("HackDS Menu System starting...\n");

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    // Initialize TTF
    if (TTF_Init() < 0) {
        fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    // Create menu state
    menu_state_t state = {0};

    // Create window
    state.window = SDL_CreateWindow(
        "HackDS",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_FULLSCREEN_DESKTOP
    );

    if (!state.window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Create renderer
    state.renderer = SDL_CreateRenderer(state.window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!state.renderer) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(state.window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Load fonts (you'd need to include a font file in the system)
    state.font_large = TTF_OpenFont("/system/share/fonts/default.ttf", 32);
    state.font_small = TTF_OpenFont("/system/share/fonts/default.ttf", 20);
    state.font_tiny = TTF_OpenFont("/system/share/fonts/default.ttf", 16);

    if (!state.font_large || !state.font_small || !state.font_tiny) {
        fprintf(stderr, "TTF_OpenFont failed: %s\n", TTF_GetError());
        // Continue without fonts - we can still show colored boxes
    }

    // Hide cursor
    SDL_ShowCursor(SDL_DISABLE);

    // Scan for games
    printf("Scanning for games...\n");
    scan_games(&state);
    printf("Found %d games\n", state.game_count);

    // Check for updates
    check_for_updates(&state);

    // Main loop
    int running = 1;
    SDL_Event event;

    while (running) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = 0;
                    break;

                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_ESCAPE:
                        case SDLK_q:
                            running = 0;
                            break;

                        case SDLK_UP:
                        case SDLK_w:
                            if (state.selected_index > 0) {
                                state.selected_index--;
                                if (state.selected_index < state.scroll_offset) {
                                    state.scroll_offset--;
                                }
                            }
                            break;

                        case SDLK_DOWN:
                        case SDLK_s:
                            if (state.selected_index < state.game_count - 1) {
                                state.selected_index++;
                                int visible_count = 8;
                                if (state.selected_index >= state.scroll_offset + visible_count) {
                                    state.scroll_offset++;
                                }
                            }
                            break;

                        case SDLK_RETURN:
                        case SDLK_SPACE:
                            if (state.game_count > 0) {
                                printf("Launching game: %s\n",
                                       state.games[state.selected_index].name);
                                launch_game(state.games[state.selected_index].path);
                            }
                            break;

                        case SDLK_r:
                            // Rescan games
                            printf("Rescanning games...\n");
                            scan_games(&state);
                            break;

                        case SDLK_u:
                            // Check for updates
                            printf("Checking for updates...\n");
                            check_for_updates(&state);
                            break;

                        case SDLK_i:
                            // Install update if available
                            if (state.update_available) {
                                printf("Installing update...\n");
                                trigger_update();
                            }
                            break;

                        case SDLK_F1:
                        case SDLK_TAB:
                            // Open settings menu
                            printf("Opening settings...\n");
                            system("/system/bin/hackds-settings");
                            break;
                    }
                    break;

                case SDL_CONTROLLERBUTTONDOWN:
                    switch (event.cbutton.button) {
                        case SDL_CONTROLLER_BUTTON_DPAD_UP:
                            if (state.selected_index > 0) state.selected_index--;
                            break;
                        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                            if (state.selected_index < state.game_count - 1)
                                state.selected_index++;
                            break;
                        case SDL_CONTROLLER_BUTTON_A:
                        case SDL_CONTROLLER_BUTTON_X:
                            // X on PS5, A on Xbox - Launch game
                            if (state.game_count > 0) {
                                launch_game(state.games[state.selected_index].path);
                            }
                            break;
                        case SDL_CONTROLLER_BUTTON_Y:
                        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
                            // Triangle on PS5 - Check for updates
                            printf("Checking for updates...\n");
                            check_for_updates(&state);
                            break;
                        case SDL_CONTROLLER_BUTTON_BACK:
                        case SDL_CONTROLLER_BUTTON_GUIDE:
                            // Options button on PS5 - Open settings
                            printf("Opening settings...\n");
                            system("/system/bin/hackds-settings");
                            break;
                        case SDL_CONTROLLER_BUTTON_START:
                            // Plus button - Exit
                            running = 0;
                            break;
                    }
                    break;
            }
        }

        // Render
        render_menu(&state);

        SDL_Delay(16);  // ~60 FPS
    }

    cleanup(&state);

    return 0;
}

static int scan_games(menu_state_t *state) {
    // Free existing games
    if (state->games) {
        for (int i = 0; i < state->game_count; i++) {
            if (state->games[i].icon) {
                SDL_DestroyTexture(state->games[i].icon);
            }
        }
        free(state->games);
    }

    state->games = NULL;
    state->game_count = 0;
    state->selected_index = 0;
    state->scroll_offset = 0;

    DIR *dir = opendir(GAME_DIR);
    if (!dir) {
        printf("No games directory found\n");
        return 0;
    }

    // Count .hdsg files
    struct dirent *entry;
    int count = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".hdsg") != NULL) {
            count++;
        }
    }

    if (count == 0) {
        closedir(dir);
        return 0;
    }

    // Allocate game entries
    state->games = calloc(count, sizeof(game_entry_t));
    if (!state->games) {
        closedir(dir);
        return -1;
    }

    // Load game info
    rewinddir(dir);
    int index = 0;
    while ((entry = readdir(dir)) != NULL && index < count) {
        if (strstr(entry->d_name, ".hdsg") == NULL) continue;

        snprintf(state->games[index].path, sizeof(state->games[index].path),
                "%s/%s", GAME_DIR, entry->d_name);

        // Try to load game metadata
        hackds_file_t *game = hackds_open(state->games[index].path);
        if (game && game->type == HACKDS_TYPE_GAME) {
            const char *metadata = hackds_get_metadata(game);
            if (metadata) {
                // Parse name, version, author (simple parsing)
                const char *name = strstr(metadata, "\"name\"");
                if (name) {
                    name = strchr(name, '"');
                    if (name) {
                        name = strchr(name + 1, '"');
                        if (name) {
                            name++;
                            const char *end = strchr(name, '"');
                            if (end) {
                                size_t len = end - name;
                                if (len < sizeof(state->games[index].name)) {
                                    strncpy(state->games[index].name, name, len);
                                    state->games[index].name[len] = '\0';
                                }
                            }
                        }
                    }
                }
            }
            hackds_close(game);
        }

        // If name wasn't loaded, use filename
        if (state->games[index].name[0] == '\0') {
            strncpy(state->games[index].name, entry->d_name,
                   sizeof(state->games[index].name) - 1);
        }

        index++;
    }

    state->game_count = index;
    closedir(dir);

    return index;
}

static void render_menu(menu_state_t *state) {
    SDL_Color bg = COLOR_BG;
    SDL_Color text = COLOR_TEXT;
    SDL_Color selected = COLOR_SELECTED;
    SDL_Color accent = COLOR_ACCENT;

    // Clear screen
    SDL_SetRenderDrawColor(state->renderer, bg.r, bg.g, bg.b, bg.a);
    SDL_RenderClear(state->renderer);

    // Draw title
    SDL_SetRenderDrawColor(state->renderer, accent.r, accent.g, accent.b, accent.a);
    SDL_Rect title_bar = {0, 0, SCREEN_WIDTH, 80};
    SDL_RenderFillRect(state->renderer, &title_bar);

    if (state->font_large) {
        render_text(state->renderer, state->font_large, "HackDS", 40, 20, text);
    }

    if (state->font_small) {
        char count_text[64];
        snprintf(count_text, sizeof(count_text), "%d games", state->game_count);
        render_text(state->renderer, state->font_small, count_text,
                   SCREEN_WIDTH - 150, 30, text);
    }

    // Draw update notification if available
    int y_offset = 80;
    if (state->update_available) {
        SDL_Color update_color = {255, 200, 50, 255};
        SDL_SetRenderDrawColor(state->renderer, 200, 150, 0, 255);
        SDL_Rect update_bar = {0, y_offset, SCREEN_WIDTH, 40};
        SDL_RenderFillRect(state->renderer, &update_bar);

        if (state->font_small) {
            char update_text[128];
            snprintf(update_text, sizeof(update_text),
                    "Update Available: %s - Press 'I' to Install",
                    state->update_version);
            render_text(state->renderer, state->font_small, update_text,
                       40, y_offset + 10, (SDL_Color){0, 0, 0, 255});
        }
        y_offset += 40;
    }

    // Draw games list
    int y = y_offset + 40;
    int visible_count = 8;

    for (int i = state->scroll_offset;
         i < state->game_count && i < state->scroll_offset + visible_count;
         i++) {
        int item_y = y + (i - state->scroll_offset) * 70;

        // Draw selection highlight
        if (i == state->selected_index) {
            SDL_SetRenderDrawColor(state->renderer,
                selected.r, selected.g, selected.b, selected.a);
            SDL_Rect highlight = {20, item_y, SCREEN_WIDTH - 40, 60};
            SDL_RenderFillRect(state->renderer, &highlight);
        }

        // Draw game name
        if (state->font_small) {
            SDL_Color color = (i == state->selected_index) ? (SDL_Color){255, 255, 255, 255} : text;
            render_text(state->renderer, state->font_small,
                       state->games[i].name, 40, item_y + 15, color);
        }
    }

    // Draw controls hint
    if (state->font_small) {
        const char *hint = "UP/DOWN: Select  |  ENTER: Play  |  F1/TAB: Settings  |  U: Updates  |  ESC: Exit";
        render_text(state->renderer, state->font_small, hint, 40, SCREEN_HEIGHT - 60, text);

        // Controller hint
        const char *controller_hint = "Controller: D-Pad: Navigate  |  X: Play  |  Triangle: Updates  |  Options: Settings";
        render_text(state->renderer, state->font_tiny,
                   controller_hint, 40, SCREEN_HEIGHT - 35, (SDL_Color){150, 150, 150, 255});
    }

    SDL_RenderPresent(state->renderer);
}

static void render_text(SDL_Renderer *renderer, TTF_Font *font,
                       const char *text, int x, int y, SDL_Color color) {
    if (!font) return;

    SDL_Surface *surface = TTF_RenderText_Blended(font, text, color);
    if (!surface) return;

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture) {
        SDL_Rect dest = {x, y, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, NULL, &dest);
        SDL_DestroyTexture(texture);
    }

    SDL_FreeSurface(surface);
}

static int launch_game(const char *game_path) {
    // Fork and exec the game loader
    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Fork failed\n");
        return -1;
    }

    if (pid == 0) {
        // Child process
        char *args[] = {"/system/bin/hackds-gameloader", (char*)game_path, NULL};
        execv("/system/bin/hackds-gameloader", args);

        fprintf(stderr, "Failed to launch game loader\n");
        exit(1);
    }

    // Parent - wait for game to finish
    int status;
    waitpid(pid, &status, 0);

    return 0;
}

static void cleanup(menu_state_t *state) {
    if (state->games) {
        for (int i = 0; i < state->game_count; i++) {
            if (state->games[i].icon) {
                SDL_DestroyTexture(state->games[i].icon);
            }
        }
        free(state->games);
    }

    if (state->font_large) TTF_CloseFont(state->font_large);
    if (state->font_small) TTF_CloseFont(state->font_small);
    if (state->font_tiny) TTF_CloseFont(state->font_tiny);
    if (state->renderer) SDL_DestroyRenderer(state->renderer);
    if (state->window) SDL_DestroyWindow(state->window);

    TTF_Quit();
    SDL_Quit();
}

static void check_for_updates(menu_state_t *state) {
    // Run the update checker in silent mode
    FILE *fp = popen("/system/bin/hackds-updater check 2>/dev/null", "r");
    if (!fp) {
        printf("Failed to run update checker\n");
        return;
    }

    char buffer[256];
    state->update_available = 0;

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        // Look for "Update available:" in output
        if (strstr(buffer, "Update available:") != NULL) {
            state->update_available = 1;

            // Extract version number
            char *version_start = strstr(buffer, ": ");
            if (version_start) {
                version_start += 2;
                char *version_end = strchr(version_start, '\n');
                if (version_end) *version_end = '\0';

                strncpy(state->update_version, version_start,
                       sizeof(state->update_version) - 1);
                state->update_version[sizeof(state->update_version) - 1] = '\0';
            }

            printf("Update found: %s\n", state->update_version);
            break;
        }
    }

    pclose(fp);

    if (!state->update_available) {
        printf("No updates available\n");
    }
}

static void trigger_update(void) {
    printf("Triggering system update...\n");

    // Fork and run the updater
    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Fork failed\n");
        return;
    }

    if (pid == 0) {
        // Child process - run updater
        char *args[] = {"/system/bin/hackds-updater", "update", NULL};
        execv("/system/bin/hackds-updater", args);

        fprintf(stderr, "Failed to launch updater\n");
        exit(1);
    }

    // Parent - wait for updater to finish
    int status;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        printf("Update completed successfully!\n");
        printf("System will reboot in 5 seconds...\n");
        sleep(5);
        system("sudo reboot");
    } else {
        printf("Update failed!\n");
    }
}
