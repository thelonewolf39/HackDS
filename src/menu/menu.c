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
    game_entry_t *games;
    int game_count;
    int selected_index;
    int scroll_offset;
} menu_state_t;

static int scan_games(menu_state_t *state);
static void render_menu(menu_state_t *state);
static void render_text(SDL_Renderer *renderer, TTF_Font *font,
                       const char *text, int x, int y, SDL_Color color);
static int launch_game(const char *game_path);
static void cleanup(menu_state_t *state);

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

    if (!state.font_large || !state.font_small) {
        fprintf(stderr, "TTF_OpenFont failed: %s\n", TTF_GetError());
        // Continue without fonts - we can still show colored boxes
    }

    // Hide cursor
    SDL_ShowCursor(SDL_DISABLE);

    // Scan for games
    printf("Scanning for games...\n");
    scan_games(&state);
    printf("Found %d games\n", state.game_count);

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
                            if (state.game_count > 0) {
                                launch_game(state.games[state.selected_index].path);
                            }
                            break;
                        case SDL_CONTROLLER_BUTTON_START:
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

    // Draw games list
    int y = 120;
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
        render_text(state->renderer, state->font_small,
                   "UP/DOWN: Select  |  ENTER: Play  |  R: Refresh  |  ESC: Exit",
                   40, SCREEN_HEIGHT - 40, text);
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
    if (state->renderer) SDL_DestroyRenderer(state->renderer);
    if (state->window) SDL_DestroyWindow(state->window);

    TTF_Quit();
    SDL_Quit();
}
