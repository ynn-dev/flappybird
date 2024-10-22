#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <time.h>

#if defined(__IPHONEOS__)
const Uint32 WINDOW_FLAGS = SDL_WINDOW_BORDERLESS | SDL_WINDOW_ALLOW_HIGHDPI;
#else
const Uint32 WINDOW_FLAGS = SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
#endif

#define COLLISION_DETECTION 1

const SDL_Rect SPRITE_BACKGROUND  = { .x = 3,   .y = 0,   .w = 144, .h = 256 };
const SDL_Rect SPRITE_GROUND      = { .x = 215, .y = 10,  .w = 12,  .h = 56  };
const SDL_Rect SPRITE_PIPE        = { .x = 152, .y = 3,   .w = 26,  .h = 147 };
const SDL_Rect SPRITE_PIPE_TOP    = { .x = 152, .y = 150, .w = 26,  .h = 13  };
const SDL_Rect SPRITE_PIPE_BOTTOM = { .x = 180, .y = 3,   .w = 26,  .h = 13  };
const SDL_Rect SPRITE_PLAYERS[3]  = {
    { .x = 381, .y = 187, .w = 17, .h = 12  },
    { .x = 381, .y = 213, .w = 17, .h = 12  },
    { .x = 381, .y = 239, .w = 17, .h = 12  },
};

const SDL_Rect SPRITE_NUMBERS[10] = {
    { .x = 254, .y = 98,  .w = 12, .h = 18 },
    { .x = 238, .y = 80,  .w = 8,  .h = 18 },
    { .x = 325, .y = 148, .w = 12, .h = 18 },
    { .x = 339, .y = 148, .w = 12, .h = 18 },
    { .x = 353, .y = 148, .w = 12, .h = 18 },
    { .x = 367, .y = 148, .w = 12, .h = 18 },
    { .x = 325, .y = 172, .w = 12, .h = 18 },
    { .x = 339, .y = 172, .w = 12, .h = 18 },
    { .x = 353, .y = 172, .w = 12, .h = 18 },
    { .x = 367, .y = 172, .w = 12, .h = 18 },
};

const float SPRITE_SCALE = 7.0f;

#define GROUND_WIDTH (SPRITE_GROUND.w * SPRITE_SCALE)
#define PIPE_WIDTH (SPRITE_PIPE.w * SPRITE_SCALE)

#define PLAYER_X (window_width / 5)

const int   window_width_initial = 1170 * 0.5;
const int   window_height_initial = 2532 * 0.5;
const float player_width = 75.0f;
const float player_height = 75.0f;
const float player_y_initial = 100.0f;
const float player_velocity_y_initial = 0.0f;
const float GRAVITY = 3000.0f;
const float jump_velocity_x = -1100.0f;
const float pipe_velocity_x = -400.0f;

const float pipe_gap = 400.0f;
#define pipe_spacing (SPRITE_PIPE.w * SPRITE_SCALE * 3.0f)
const float pipe_gap_padding_top = 100.0f;
const float pipe_gap_padding_bottom = 100.0f;

int window_width  = 0;
int window_height = 0;

float ground_offset = 0;

SDL_Window   *window;
SDL_Renderer *renderer;
SDL_Texture  *texture;

Mix_Chunk *sfx_die;
Mix_Chunk *sfx_hit;
Mix_Chunk *sfx_point;
Mix_Chunk *sfx_swooshing;
Mix_Chunk *sfx_wing;

SDL_Event     event;

int      running;
int      game_over;
uint64_t ticks;
int      score;

float player_y;
float player_velocity_y;
const SDL_Rect *player_sprite;

typedef struct pipe {
    float x;
    float gap_y;
} pipe;

const int max_pipes = 10;
pipe pipes[max_pipes];
int pipes_len = 0;

int pipe_to_pass = 0;

void reset() {
    game_over = 0;
    player_y = player_y_initial;
    player_velocity_y = player_velocity_y_initial;
    pipes_len = 0;
    score = 0;
    pipe_to_pass = 0;
}

void action() {
    if (!game_over) {
        Mix_PlayChannel(-1, sfx_wing, 0);
        player_velocity_y = jump_velocity_x;
    } else {
        Mix_PlayChannel(-1, sfx_swooshing, 0);
        reset();
    }
}

void do_game_over() {
    Mix_PlayChannel(-1, sfx_hit, 0);
    game_over = 1;
}

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
                        action();
                        break;
                    default:
                        break;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                action();
                break;
            case SDL_WINDOWEVENT:
                SDL_GetRendererOutputSize(renderer, &window_width, &window_height);
                break;
            default:
                break;
        }
    }
}

int get_gap_y() {
    int min_x = pipe_gap_padding_top;
    int max_x = window_height - (window_height / 8.0f) - pipe_gap - pipe_gap_padding_bottom;
    int range = max_x - min_x;

    return (rand() % range) + min_x;
}

void get_background_rect(SDL_FRect *rect) {
    rect->x = 0;
    rect->y = 0;
    rect->w = window_width;
    rect->h = window_height;
}

void get_pipe_top_rect(int i, SDL_FRect *rect) {
    rect->x = pipes[i].x;
    rect->y = 0;
    rect->w = PIPE_WIDTH;
    rect->h = pipes[i].gap_y;
}

void get_pipe_top_end_rect(int i, SDL_FRect *rect) {
    rect->x = pipes[i].x;
    rect->y = pipes[i].gap_y - (SPRITE_PIPE_TOP.h * SPRITE_SCALE);
    rect->w = PIPE_WIDTH;
    rect->h = (SPRITE_PIPE_TOP.h * SPRITE_SCALE);
}

void get_pipe_bottom_rect(int i, SDL_FRect *rect) {
    rect->x = pipes[i].x;
    rect->y = pipes[i].gap_y + pipe_gap;
    rect->w = PIPE_WIDTH;
    rect->h = window_height - (window_height / 8.0f) - (pipes[i].gap_y + pipe_gap);
}

void get_pipe_bottom_end_rect(int i, SDL_FRect *rect) {
    rect->x = pipes[i].x;
    rect->y = pipes[i].gap_y + pipe_gap;
    rect->w = PIPE_WIDTH;
    rect->h = (SPRITE_PIPE_TOP.h * SPRITE_SCALE);
}

void get_player_rect(SDL_FRect *rect) {
    rect->x = PLAYER_X;
    rect->y = player_y;
    rect->w = 17 * SPRITE_SCALE;
    rect->h = 12 * SPRITE_SCALE;
}

void get_top_rect(SDL_FRect *rect) {
    rect->x = 0;
    rect->y = -100;
    rect->w = window_width;
    rect->h = 100;
}

void get_bottom_rect(SDL_FRect *rect) {
    rect->x = 0;
    rect->y = (window_height / 8.0f) * 7;
    rect->w = window_width;
    rect->h = (window_height / 8.0f) * 1;
}

void get_ground_segment_rect(float x, SDL_FRect *rect) {
    rect->x = x;
    rect->y = (window_height / 8.0f) * 7;
    rect->w = GROUND_WIDTH;
    rect->h = (window_height / 8.0f) * 1;
}

void update(float dt) {
    if (game_over) {
        return;
    }

    ground_offset += pipe_velocity_x * dt;
    ground_offset = fmodf(ground_offset, SPRITE_GROUND.w * SPRITE_SCALE);

    // If there are no pipes, create a pipe
    if (pipes_len == 0) {
        pipes[0].x = window_width;
        pipes[0].gap_y = get_gap_y();
        pipes_len++;
    }

    // If the last pipe is futher away than pipe_spacing, create a new one
    if (pipes_len < max_pipes && pipes[pipes_len - 1].x < window_width - pipe_spacing) {
        pipes[pipes_len].x = window_width;
        pipes[pipes_len].gap_y = get_gap_y();
        pipes_len++; 
    }

    // Remove pipes than the player has passed
    while (pipes[0].x + PIPE_WIDTH < 0) {
        // Move pipes down in the array
        for (int i = 0; i < pipes_len - 1; i++) {
            pipes[i] = pipes[i + 1];
        }
        pipes_len--;
        pipe_to_pass--;
    }

    for (int i = 0; i < pipes_len; i++) {
        pipes[i].x += pipe_velocity_x * dt;
    }

    player_velocity_y += GRAVITY * dt;
    player_y += player_velocity_y * dt;
    player_sprite = &SPRITE_PLAYERS[ticks % 300 / 100];

#if COLLISION_DETECTION
    // Collision detection
    SDL_FRect a;
    SDL_FRect b;

    get_player_rect(&a);

    get_top_rect(&b);
    if (SDL_HasIntersectionF(&a, &b)) {
        do_game_over();
        return;
    }

    get_bottom_rect(&b);
    if (SDL_HasIntersectionF(&a, &b)) {
        do_game_over();
        return;
    }

    for (int i = 0; i < pipes_len; i++) {
        get_pipe_top_rect(i, &b);
        if (SDL_HasIntersectionF(&a, &b)) {
            do_game_over();
            return;
        }

        get_pipe_bottom_rect(i, &b);
        if (SDL_HasIntersectionF(&a, &b)) {
            do_game_over();
            return;
        }
    }
#endif // COLLISION_DETECTION

    if (PLAYER_X > pipes[pipe_to_pass].x + (PIPE_WIDTH / 2)) {
        Mix_PlayChannel(-1, sfx_point, 0);
        score++;
        pipe_to_pass++;
    }
}

void render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_FRect rect;

    get_background_rect(&rect);
    SDL_RenderCopyF(renderer, texture, &SPRITE_BACKGROUND, &rect);

    for (int i = 0; ; i++) {
        int ground_x = ground_offset + GROUND_WIDTH * i;
        if (ground_x > window_width) {
            break;
        }

        get_ground_segment_rect(ground_x, &rect);
        SDL_RenderCopyF(renderer, texture, &SPRITE_GROUND, &rect);
    }

    for (int i = 0; i < pipes_len; i++) {
        get_pipe_top_rect(i, &rect);
        SDL_RenderCopyF(renderer, texture, &SPRITE_PIPE, &rect);

        get_pipe_top_end_rect(i, &rect);
        SDL_RenderCopyF(renderer, texture, &SPRITE_PIPE_TOP, &rect);

        get_pipe_bottom_rect(i, &rect);
        SDL_RenderCopyF(renderer, texture, &SPRITE_PIPE, &rect);

        get_pipe_bottom_end_rect(i, &rect);
        SDL_RenderCopyF(renderer, texture, &SPRITE_PIPE_BOTTOM, &rect);
    }

    get_player_rect(&rect);
    SDL_RenderCopyF(renderer, texture, player_sprite, &rect);

    // int score_cache = score;
    // int digits = 1;

    // while (score_cache / 10 > 0) {
    //     digits++;
    //     score_cache /= 10;
    // }

    // for (int i = digits; i > 0; i--) {
    //     rect.x = (window_width / 2) - ((SPRITE_NUMBERS[score / (int)pow(10, digits - 1)].w * SPRITE_SCALE) / 2) - 1.1f * (SPRITE_NUMBERS[score / (int)pow(10, digits - 1)].w * SPRITE_SCALE);
    //     rect.y = 100;
    //     rect.w = SPRITE_NUMBERS[score / (int)pow(10, digits - 1)].w * SPRITE_SCALE;
    //     rect.h = SPRITE_NUMBERS[score / (int)pow(10, digits - 1)].h * SPRITE_SCALE;
    //     SDL_RenderCopyF(renderer, texture, &SPRITE_NUMBERS[score / (int)pow(10, digits - 1)], &rect);
    // }

    if (score < 10) {
        rect.x = (window_width / 2) - ((SPRITE_NUMBERS[score].w * SPRITE_SCALE) / 2);
        rect.y = 200;
        rect.w = SPRITE_NUMBERS[score].w * SPRITE_SCALE;
        rect.h = SPRITE_NUMBERS[score].h * SPRITE_SCALE;
        SDL_RenderCopyF(renderer, texture, &SPRITE_NUMBERS[score], &rect);
    } else if (score < 100) {
        rect.x = (window_width / 2) - ((SPRITE_NUMBERS[score / 10 % 10].w * SPRITE_SCALE) / 2) - (SPRITE_NUMBERS[score / 10 % 10].w * SPRITE_SCALE) / 1.9f;
        rect.y = 200;
        rect.w = SPRITE_NUMBERS[score / 10 % 10].w * SPRITE_SCALE;
        rect.h = SPRITE_NUMBERS[score / 10 % 10].h * SPRITE_SCALE;
        SDL_RenderCopyF(renderer, texture, &SPRITE_NUMBERS[score / 10 % 10], &rect);

        rect.x = (window_width / 2) - ((SPRITE_NUMBERS[score % 10].w * SPRITE_SCALE) / 2) + (SPRITE_NUMBERS[score % 10].w * SPRITE_SCALE) / 1.9f;
        rect.y = 200;
        rect.w = SPRITE_NUMBERS[score % 10].w * SPRITE_SCALE;
        rect.h = SPRITE_NUMBERS[score % 10].h * SPRITE_SCALE;
        SDL_RenderCopyF(renderer, texture, &SPRITE_NUMBERS[score % 10], &rect);
    } else if (score < 1000) {
        rect.x = (window_width / 2) - ((SPRITE_NUMBERS[score / 100].w * SPRITE_SCALE) / 2) - 1.1f * (SPRITE_NUMBERS[score / 100].w * SPRITE_SCALE);
        rect.y = 200;
        rect.w = SPRITE_NUMBERS[score / 100].w * SPRITE_SCALE;
        rect.h = SPRITE_NUMBERS[score / 100].h * SPRITE_SCALE;
        SDL_RenderCopyF(renderer, texture, &SPRITE_NUMBERS[score / 100], &rect);

        rect.x = (window_width / 2) - ((SPRITE_NUMBERS[score / 10 % 10].w * SPRITE_SCALE) / 2);
        rect.y = 200;
        rect.w = SPRITE_NUMBERS[score / 10 % 10].w * SPRITE_SCALE;
        rect.h = SPRITE_NUMBERS[score / 10 % 10].h * SPRITE_SCALE;
        SDL_RenderCopyF(renderer, texture, &SPRITE_NUMBERS[score / 10 % 10], &rect);

        rect.x = (window_width / 2) - ((SPRITE_NUMBERS[score % 10].w * SPRITE_SCALE) / 2) + 1.1f * (SPRITE_NUMBERS[score % 10].w * SPRITE_SCALE);
        rect.y = 200;
        rect.w = SPRITE_NUMBERS[score % 10].w * SPRITE_SCALE;
        rect.h = SPRITE_NUMBERS[score % 10].h * SPRITE_SCALE;
        SDL_RenderCopyF(renderer, texture, &SPRITE_NUMBERS[score % 10], &rect);
    }

    SDL_RenderPresent(renderer);
}

void run() {
    running           = 1;
    uint64_t lastTicks = SDL_GetTicks64();
    uint64_t frames   = 0;

    reset();

    Mix_PlayChannel(-1, sfx_swooshing, 0);

    while (running) {
        ticks     = SDL_GetTicks64();
        float dt  = (ticks - lastTicks) / 1000.0f;
        lastTicks = ticks;

        processEvents();
        update(dt);
        render();

        frames++;
    }

    // printf("ticks = %llu, frames = %llu", SDL_GetTicks64(), frames);
}

int main(int argc, char *argv[]) {
    srand(time(NULL));

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", IMG_GetError(), window);
        return 1;
    }

    if (Mix_Init(0) != 0) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", Mix_GetError(), window);
        return 1;
    }

    window = SDL_CreateWindow("Flappy Bird", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width_initial, window_height_initial, WINDOW_FLAGS);
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

    texture = IMG_LoadTexture(renderer, "spritesheet.png");
    if (!texture) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", IMG_GetError(), window);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return 1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", Mix_GetError(), window);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return 1;
    }

    sfx_die = Mix_LoadWAV("sfx_die.wav");
    if (!(sfx_die)) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", Mix_GetError(), window);
        return 1;
    }

    sfx_hit = Mix_LoadWAV("sfx_hit.wav");
    if (!sfx_hit) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", Mix_GetError(), window);
        return 1;
    }

    sfx_point = Mix_LoadWAV("sfx_point.wav");
    if (!sfx_point) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", Mix_GetError(), window);
        return 1;
    }

    sfx_swooshing = Mix_LoadWAV("sfx_swooshing.wav");
    if (!sfx_swooshing) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", Mix_GetError(), window);
        return 1;
    }

    sfx_wing = Mix_LoadWAV("sfx_wing.wav");
    if (!sfx_wing) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", Mix_GetError(), window);
        return 1;
    }

    if (SDL_GetRendererOutputSize(renderer, &window_width, &window_height) != 0) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", SDL_GetError(), window);
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return 1;
    }

    // printf("width = %d, height = %d\n", window_width, window_height);

    run();

    Mix_CloseAudio();
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    Mix_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
