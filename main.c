#include <SDL2/SDL.h>

#define WINDOW_WIDTH 585 * 0.6
#define WINDOW_HEIGHT 1266 * 0.6

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Event event;
int running;

float player_y = 100.0f;
float player_velocity_y = 0.0f;
const float gravity = 8.0f;

void processEvents() {
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                running = 0;
                break;
            case SDL_KEYDOWN:
                if (event.key.repeat != 0) {
                    break;
                }

                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        running = 0;
                        break;
                    case SDLK_SPACE:
                        player_velocity_y = -2.5f;
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }
}

void update(float dt) {
    player_velocity_y += gravity * dt;
    player_y += player_velocity_y;
}

void render() {
    SDL_SetRenderDrawColor(renderer, 128, 64, 200, 255);
    SDL_RenderClear(renderer);

    SDL_FRect rect = {
        .x = 100,
        .y = player_y,
        .w = 100,
        .h = 100, 
    };

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRectF(renderer, &rect);

    SDL_RenderPresent(renderer);
}

void run() {
    running = 1;
    Uint64 lastTime = SDL_GetTicks64();

    while (running) {
        Uint64 startTime = SDL_GetTicks64();
        float dt         = (startTime - lastTime) / 1000.0f;
        lastTime         = startTime;

        processEvents();
        update(dt);
        render();
    }
}

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("Moving Waveform Visualization", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!window) {
        fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        fprintf(stderr, "Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        return 1;
    }

    run();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}