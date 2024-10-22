#include <SDL2/SDL.h>

#define WINDOW_WIDTH 585 * 0.6
#define WINDOW_HEIGHT 1266 * 0.6

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Event event;
int running;

const float gravity = 1000.0f;
float player_velocity_y = 0.0f;
float player_y = 100.0f;

const float jump_velocity_x = -600.0f;

const float pipe_velocity_x = -100.0f;
float pipe_x = 500.0f;

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
                        player_velocity_y = jump_velocity_x;
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
    pipe_x += pipe_velocity_x * dt;

    player_velocity_y += gravity * dt;
    player_y += player_velocity_y * dt;
}

void render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_FRect pipe = {
        .x = pipe_x,
        .y = 0,
        .w = 100,
        .h = 400,
    };

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRectF(renderer, &pipe);

    SDL_FRect pipe2 = {
        .x = pipe_x,
        .y = 700,
        .w = 100,
        .h = 300,
    };

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRectF(renderer, &pipe2);

    SDL_FRect rect = {
        .x = 100,
        .y = player_y,
        .w = 100,
        .h = 100, 
    };

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRectF(renderer, &rect);

    SDL_RenderPresent(renderer);
}

void run() {
    running = 1;
    Uint64 lastTime = SDL_GetTicks64();

    uint64_t frames = 0;

    while (running) {
        Uint64 startTime = SDL_GetTicks64();
        float dt         = (startTime - lastTime) / 1000.0f;
        lastTime         = startTime;

        processEvents();
        update(dt);
        render();

        frames++;
    }

    printf("ticks = %llu, frames = %llu", SDL_GetTicks64(), frames);
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

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
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