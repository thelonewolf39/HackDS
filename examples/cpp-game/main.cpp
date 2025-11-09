/*
 * Simple C++ HackDS Game
 * A basic SDL2 particle effect demo
 */

#include <SDL2/SDL.h>
#include <vector>
#include <random>
#include <cmath>

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const int FPS = 60;

struct Particle {
    float x, y;
    float vx, vy;
    float life;
    uint8_t r, g, b;
};

class ParticleSystem {
private:
    std::vector<Particle> particles;
    std::mt19937 rng;
    std::uniform_real_distribution<float> dist_angle;
    std::uniform_real_distribution<float> dist_speed;
    std::uniform_int_distribution<int> dist_color;

public:
    ParticleSystem() :
        rng(std::random_device{}()),
        dist_angle(0.0f, 2.0f * M_PI),
        dist_speed(2.0f, 8.0f),
        dist_color(100, 255) {}

    void emit(float x, float y, int count) {
        for (int i = 0; i < count; i++) {
            Particle p;
            p.x = x;
            p.y = y;

            float angle = dist_angle(rng);
            float speed = dist_speed(rng);
            p.vx = cos(angle) * speed;
            p.vy = sin(angle) * speed;

            p.life = 1.0f;
            p.r = dist_color(rng);
            p.g = dist_color(rng);
            p.b = dist_color(rng);

            particles.push_back(p);
        }
    }

    void update(float dt) {
        for (auto it = particles.begin(); it != particles.end();) {
            it->x += it->vx;
            it->y += it->vy;
            it->vy += 0.2f; // Gravity
            it->life -= dt;

            if (it->life <= 0.0f) {
                it = particles.erase(it);
            } else {
                ++it;
            }
        }
    }

    void draw(SDL_Renderer* renderer) {
        for (const auto& p : particles) {
            uint8_t alpha = static_cast<uint8_t>(p.life * 255);
            SDL_SetRenderDrawColor(renderer, p.r, p.g, p.b, alpha);
            SDL_Rect rect = {
                static_cast<int>(p.x - 2),
                static_cast<int>(p.y - 2),
                4, 4
            };
            SDL_RenderFillRect(renderer, &rect);
        }
    }

    size_t count() const { return particles.size(); }
};

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "HackDS C++ Game",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!renderer) {
        SDL_Log("Renderer creation failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    ParticleSystem particles;
    bool running = true;
    SDL_Event event;
    Uint32 last_time = SDL_GetTicks();

    while (running) {
        // Calculate delta time
        Uint32 current_time = SDL_GetTicks();
        float dt = (current_time - last_time) / 1000.0f;
        last_time = current_time;

        // Handle events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;

                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE ||
                        event.key.keysym.sym == SDLK_q) {
                        running = false;
                    }
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    particles.emit(
                        static_cast<float>(event.button.x),
                        static_cast<float>(event.button.y),
                        50
                    );
                    break;

                case SDL_MOUSEMOTION:
                    if (event.motion.state & SDL_BUTTON_LMASK) {
                        particles.emit(
                            static_cast<float>(event.motion.x),
                            static_cast<float>(event.motion.y),
                            10
                        );
                    }
                    break;
            }
        }

        // Update
        particles.update(dt);

        // Draw
        SDL_SetRenderDrawColor(renderer, 10, 10, 20, 255);
        SDL_RenderClear(renderer);

        particles.draw(renderer);

        // Draw title
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        // Note: In production you'd use SDL_ttf for text

        SDL_RenderPresent(renderer);

        // Cap frame rate
        SDL_Delay(1000 / FPS);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
