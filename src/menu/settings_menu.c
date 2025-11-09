/*
 * HackDS Settings Menu
 * Settings interface for WiFi, Bluetooth, and system configuration
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define COLOR_BG      {20, 20, 30, 255}
#define COLOR_TEXT    {220, 220, 220, 255}
#define COLOR_SELECTED {100, 150, 255, 255}
#define COLOR_ACCENT  {60, 120, 220, 255}
#define COLOR_SUCCESS {50, 200, 50, 255}
#define COLOR_ERROR   {200, 50, 50, 255}

typedef enum {
    MENU_MAIN,
    MENU_WIFI,
    MENU_BLUETOOTH,
    MENU_SYSTEM
} menu_mode_t;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font_large;
    TTF_Font *font_small;
    TTF_Font *font_tiny;
    menu_mode_t current_menu;
    int selected_index;
    char status_message[256];
    SDL_GameController *controller;
} settings_state_t;

static void render_main_menu(settings_state_t *state);
static void render_wifi_menu(settings_state_t *state);
static void render_bluetooth_menu(settings_state_t *state);
static void render_system_menu(settings_state_t *state);
static void render_text(SDL_Renderer *renderer, TTF_Font *font,
                       const char *text, int x, int y, SDL_Color color);
static void set_status(settings_state_t *state, const char *message);
static void init_controller(settings_state_t *state);

int run_settings_menu(void) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    if (TTF_Init() < 0) {
        fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    settings_state_t state = {0};
    state.current_menu = MENU_MAIN;

    // Create window
    state.window = SDL_CreateWindow(
        "HackDS Settings",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (!state.window) {
        fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Create renderer
    state.renderer = SDL_CreateRenderer(state.window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!state.renderer) {
        fprintf(stderr, "Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(state.window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Load fonts
    state.font_large = TTF_OpenFont("/system/share/fonts/default.ttf", 36);
    state.font_small = TTF_OpenFont("/system/share/fonts/default.ttf", 24);
    state.font_tiny = TTF_OpenFont("/system/share/fonts/default.ttf", 18);

    // Hide cursor
    SDL_ShowCursor(SDL_DISABLE);

    // Initialize controller
    init_controller(&state);

    snprintf(state.status_message, sizeof(state.status_message),
            "Use D-Pad or Arrow Keys to navigate");

    // Main loop
    int running = 1;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = 0;
                    break;

                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_ESCAPE:
                        case SDLK_q:
                            if (state.current_menu == MENU_MAIN) {
                                running = 0;
                            } else {
                                state.current_menu = MENU_MAIN;
                                state.selected_index = 0;
                            }
                            break;

                        case SDLK_UP:
                        case SDLK_w:
                            if (state.selected_index > 0) {
                                state.selected_index--;
                            }
                            break;

                        case SDLK_DOWN:
                        case SDLK_s:
                            state.selected_index++;
                            break;

                        case SDLK_RETURN:
                        case SDLK_SPACE:
                            // Handle selection based on current menu
                            if (state.current_menu == MENU_MAIN) {
                                if (state.selected_index == 0) {
                                    state.current_menu = MENU_WIFI;
                                    state.selected_index = 0;
                                } else if (state.selected_index == 1) {
                                    state.current_menu = MENU_BLUETOOTH;
                                    state.selected_index = 0;
                                } else if (state.selected_index == 2) {
                                    state.current_menu = MENU_SYSTEM;
                                    state.selected_index = 0;
                                } else if (state.selected_index == 3) {
                                    running = 0;
                                }
                            }
                            break;
                    }
                    break;

                case SDL_CONTROLLERBUTTONDOWN:
                    switch (event.cbutton.button) {
                        case SDL_CONTROLLER_BUTTON_DPAD_UP:
                            if (state.selected_index > 0) {
                                state.selected_index--;
                            }
                            break;

                        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                            state.selected_index++;
                            break;

                        case SDL_CONTROLLER_BUTTON_A:
                        case SDL_CONTROLLER_BUTTON_X:
                            // Handle selection
                            if (state.current_menu == MENU_MAIN) {
                                if (state.selected_index == 0) {
                                    state.current_menu = MENU_WIFI;
                                    state.selected_index = 0;
                                } else if (state.selected_index == 1) {
                                    state.current_menu = MENU_BLUETOOTH;
                                    state.selected_index = 0;
                                } else if (state.selected_index == 2) {
                                    state.current_menu = MENU_SYSTEM;
                                    state.selected_index = 0;
                                } else if (state.selected_index == 3) {
                                    running = 0;
                                }
                            }
                            break;

                        case SDL_CONTROLLER_BUTTON_B:
                        case SDL_CONTROLLER_BUTTON_BACK:
                            if (state.current_menu == MENU_MAIN) {
                                running = 0;
                            } else {
                                state.current_menu = MENU_MAIN;
                                state.selected_index = 0;
                            }
                            break;

                        case SDL_CONTROLLER_BUTTON_START:
                            running = 0;
                            break;
                    }
                    break;

                case SDL_CONTROLLERDEVICEADDED:
                    init_controller(&state);
                    set_status(&state, "Controller connected");
                    break;

                case SDL_CONTROLLERDEVICEREMOVED:
                    if (state.controller) {
                        SDL_GameControllerClose(state.controller);
                        state.controller = NULL;
                    }
                    set_status(&state, "Controller disconnected");
                    break;
            }
        }

        // Render based on current menu
        switch (state.current_menu) {
            case MENU_MAIN:
                render_main_menu(&state);
                break;
            case MENU_WIFI:
                render_wifi_menu(&state);
                break;
            case MENU_BLUETOOTH:
                render_bluetooth_menu(&state);
                break;
            case MENU_SYSTEM:
                render_system_menu(&state);
                break;
        }

        SDL_Delay(16);  // ~60 FPS
    }

    // Cleanup
    if (state.controller) SDL_GameControllerClose(state.controller);
    if (state.font_large) TTF_CloseFont(state.font_large);
    if (state.font_small) TTF_CloseFont(state.font_small);
    if (state.font_tiny) TTF_CloseFont(state.font_tiny);
    if (state.renderer) SDL_DestroyRenderer(state.renderer);
    if (state.window) SDL_DestroyWindow(state.window);

    TTF_Quit();
    SDL_Quit();

    return 0;
}

static void init_controller(settings_state_t *state) {
    // Try to open first available controller
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            state->controller = SDL_GameControllerOpen(i);
            if (state->controller) {
                printf("Controller connected: %s\n",
                      SDL_GameControllerName(state->controller));
                return;
            }
        }
    }
}

static void set_status(settings_state_t *state, const char *message) {
    snprintf(state->status_message, sizeof(state->status_message), "%s", message);
}

static void render_main_menu(settings_state_t *state) {
    SDL_Color bg = COLOR_BG;
    SDL_Color text = COLOR_TEXT;
    SDL_Color selected = COLOR_SELECTED;
    SDL_Color accent = COLOR_ACCENT;

    SDL_SetRenderDrawColor(state->renderer, bg.r, bg.g, bg.b, bg.a);
    SDL_RenderClear(state->renderer);

    // Title bar
    SDL_SetRenderDrawColor(state->renderer, accent.r, accent.g, accent.b, accent.a);
    SDL_Rect title_bar = {0, 0, SCREEN_WIDTH, 80};
    SDL_RenderFillRect(state->renderer, &title_bar);

    if (state->font_large) {
        render_text(state->renderer, state->font_large,
                   "⚙ Settings", 40, 20, text);
    }

    // Menu items
    const char *items[] = {
        "WiFi & Internet",
        "Bluetooth Devices",
        "System Settings",
        "Exit Settings"
    };

    int y = 150;
    for (int i = 0; i < 4; i++) {
        if (i == state->selected_index) {
            SDL_SetRenderDrawColor(state->renderer,
                selected.r, selected.g, selected.b, selected.a);
            SDL_Rect highlight = {40, y, SCREEN_WIDTH - 80, 60};
            SDL_RenderFillRect(state->renderer, &highlight);
        }

        if (state->font_small) {
            SDL_Color color = (i == state->selected_index) ?
                (SDL_Color){255, 255, 255, 255} : text;
            render_text(state->renderer, state->font_small,
                       items[i], 60, y + 15, color);
        }

        y += 80;
    }

    // Status message
    if (state->font_tiny) {
        render_text(state->renderer, state->font_tiny,
                   state->status_message, 40, SCREEN_HEIGHT - 60, text);
    }

    // Controls hint
    if (state->font_tiny) {
        const char *hint = state->controller ?
            "D-Pad: Navigate | A/X: Select | B/Circle: Back" :
            "Arrow Keys: Navigate | Enter: Select | ESC: Back";
        render_text(state->renderer, state->font_tiny,
                   hint, 40, SCREEN_HEIGHT - 35, text);
    }

    SDL_RenderPresent(state->renderer);
}

static void render_wifi_menu(settings_state_t *state) {
    SDL_Color bg = COLOR_BG;
    SDL_Color text = COLOR_TEXT;
    SDL_Color accent = COLOR_ACCENT;

    SDL_SetRenderDrawColor(state->renderer, bg.r, bg.g, bg.b, bg.a);
    SDL_RenderClear(state->renderer);

    // Title
    SDL_SetRenderDrawColor(state->renderer, accent.r, accent.g, accent.b, accent.a);
    SDL_Rect title_bar = {0, 0, SCREEN_WIDTH, 80};
    SDL_RenderFillRect(state->renderer, &title_bar);

    if (state->font_large) {
        render_text(state->renderer, state->font_large,
                   "WiFi Settings", 40, 20, text);
    }

    if (state->font_small) {
        render_text(state->renderer, state->font_small,
                   "WiFi configuration managed via command line", 400, 300, text);
        render_text(state->renderer, state->font_small,
                   "Run: wifi-manager scan", 400, 350, text);
        render_text(state->renderer, state->font_small,
                   "Then: wifi-manager connect <SSID> <password>", 400, 400, text);
    }

    if (state->font_tiny) {
        render_text(state->renderer, state->font_tiny,
                   "Press B/Circle or ESC to go back", 40, SCREEN_HEIGHT - 35, text);
    }

    SDL_RenderPresent(state->renderer);
}

static void render_bluetooth_menu(settings_state_t *state) {
    SDL_Color bg = COLOR_BG;
    SDL_Color text = COLOR_TEXT;
    SDL_Color accent = COLOR_ACCENT;

    SDL_SetRenderDrawColor(state->renderer, bg.r, bg.g, bg.b, bg.a);
    SDL_RenderClear(state->renderer);

    // Title
    SDL_SetRenderDrawColor(state->renderer, accent.r, accent.g, accent.b, accent.a);
    SDL_Rect title_bar = {0, 0, SCREEN_WIDTH, 80};
    SDL_RenderFillRect(state->renderer, &title_bar);

    if (state->font_large) {
        render_text(state->renderer, state->font_large,
                   "Bluetooth Settings", 40, 20, text);
    }

    if (state->font_small) {
        render_text(state->renderer, state->font_small,
                   "PS5 Controller Pairing:", 300, 250, text);
        render_text(state->renderer, state->font_small,
                   "1. Hold PS + Share until light flashes", 300, 300, text);
        render_text(state->renderer, state->font_small,
                   "2. Run: bluetooth-manager ps5-setup", 300, 350, text);
        render_text(state->renderer, state->font_small,
                   "3. Follow on-screen prompts", 300, 400, text);
    }

    if (state->font_tiny) {
        render_text(state->renderer, state->font_tiny,
                   "Press B/Circle or ESC to go back", 40, SCREEN_HEIGHT - 35, text);
    }

    SDL_RenderPresent(state->renderer);
}

static void render_system_menu(settings_state_t *state) {
    SDL_Color bg = COLOR_BG;
    SDL_Color text = COLOR_TEXT;
    SDL_Color accent = COLOR_ACCENT;

    SDL_SetRenderDrawColor(state->renderer, bg.r, bg.g, bg.b, bg.a);
    SDL_RenderClear(state->renderer);

    // Title
    SDL_SetRenderDrawColor(state->renderer, accent.r, accent.g, accent.b, accent.a);
    SDL_Rect title_bar = {0, 0, SCREEN_WIDTH, 80};
    SDL_RenderFillRect(state->renderer, &title_bar);

    if (state->font_large) {
        render_text(state->renderer, state->font_large,
                   "System Settings", 40, 20, text);
    }

    if (state->font_small) {
        render_text(state->renderer, state->font_small,
                   "HackDS v0.1.0", 400, 250, text);
        render_text(state->renderer, state->font_small,
                   "Auto-updates: Press U in main menu", 400, 300, text);
        render_text(state->renderer, state->font_small,
                   "System info: Run 'uname -a'", 400, 350, text);
    }

    if (state->font_tiny) {
        render_text(state->renderer, state->font_tiny,
                   "Press B/Circle or ESC to go back", 40, SCREEN_HEIGHT - 35, text);
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

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    return run_settings_menu();
}
