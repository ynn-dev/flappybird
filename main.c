#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>

#if defined(__IPHONEOS__)
const Uint32 WINDOW_FLAGS = SDL_WINDOW_BORDERLESS | SDL_WINDOW_ALLOW_HIGHDPI;
#else
const Uint32 WINDOW_FLAGS = SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
#endif

#define COLLISION_DETECTION  1
#define DRAW_PLAYER_COLLIDER 0

const SDL_Rect SPRITE_BACKGROUND   = { .x = 3,   .y = 0,   .w = 144, .h = 256 };
const SDL_Rect SPRITE_GROUND       = { .x = 215, .y = 10,  .w = 12,  .h = 56  };
const SDL_Rect SPRITE_LOGO         = { .x = 152, .y = 200, .w = 89,  .h = 24  };
const SDL_Rect SPRITE_BUTTON_START = { .x = 212, .y = 230, .w = 40,  .h = 14  };
const SDL_Rect SPRITE_BUTTON_OK    = { .x = 212, .y = 154, .w = 40,  .h = 14  };
const SDL_Rect SPRITE_BUTTON_PAUSE = { .x = 261, .y = 174, .w = 13,  .h = 14  };
const SDL_Rect SPRITE_BUTTON_PLAY  = { .x = 412, .y = 94,  .w = 13,  .h = 14  };
const SDL_Rect SPRITE_GET_READY    = { .x = 254, .y = 71,  .w = 92,  .h = 25  };
const SDL_Rect SPRITE_TAP          = { .x = 370, .y = 43,  .w = 57,  .h = 49  };
const SDL_Rect SPRITE_GAME_OVER    = { .x = 152, .y = 173, .w = 96,  .h = 21  };
const SDL_Rect SPRITE_BOARD        = { .x = 260, .y = 195, .w = 113, .h = 57  };
const SDL_Rect SPRITE_NEW          = { .x = 214, .y = 126, .w = 16,  .h = 7   };
const SDL_Rect SPRITE_PIPE         = { .x = 152, .y = 3,   .w = 26,  .h = 147 };
const SDL_Rect SPRITE_PIPE_TOP     = { .x = 152, .y = 150, .w = 26,  .h = 13  };
const SDL_Rect SPRITE_PIPE_BOTTOM  = { .x = 180, .y = 3,   .w = 26,  .h = 13  };
const SDL_Rect SPRITE_PLAYERS[3]   = {
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

const SDL_Rect SPRITE_NUMBERS_SM[10] = {
    { .x = 279, .y = 171, .w = 6,  .h = 7 },
    { .x = 282, .y = 180, .w = 3,  .h = 7 },
    { .x = 289, .y = 171, .w = 6, .h = 7 },
    { .x = 289, .y = 180, .w = 6, .h = 7 },
    { .x = 298, .y = 171, .w = 6, .h = 7 },
    { .x = 298, .y = 180, .w = 6, .h = 7 },
    { .x = 306, .y = 171, .w = 6, .h = 7 },
    { .x = 306, .y = 180, .w = 6, .h = 7 },
    { .x = 315, .y = 171, .w = 6, .h = 7 },
    { .x = 315, .y = 180, .w = 6, .h = 7 },
};

const float SPRITE_SCALE = 7.0f;

#define PLAYER_X (window_width / 5)

const int   WINDOW_WIDTH_INITIAL = 1170 * 0.5;
const int   WINDOW_HEIGHT_INITIAL = 2532 * 0.5;

int window_width_real;
int window_height_real;

float window_width_ratio;
float window_height_ratio;

const float player_y_initial = 100.0f;
const float player_velocity_y_initial = 0.0f;
const float GRAVITY = 3000.0f;
const float jump_velocity_x = -1100.0f;
const float pipe_velocity_x = -400.0f;

int sprite_width(const SDL_Rect *rect) {
    return rect->w * SPRITE_SCALE;
}

int sprite_height(const SDL_Rect *rect) {
    return rect->h * SPRITE_SCALE;
}

const float PIPE_GAP                = 400.0f;
#define     PIPE_SPACING            (SPRITE_PIPE.w * SPRITE_SCALE * 3.0f)
const float PIPE_GAP_PADDING_TOP    = 100.0f;
const float PIPE_GAP_PADDING_BOTTOM = 100.0f;

SDL_Window   *window;
SDL_Renderer *renderer;
int           window_width  = 0;
int           window_height = 0;
SDL_Texture  *texture;

Mix_Chunk *sfx_die;
Mix_Chunk *sfx_hit;
Mix_Chunk *sfx_point;
Mix_Chunk *sfx_swooshing;
Mix_Chunk *sfx_wing;

typedef enum game_state_t {
    STATE_MENU      = 0,
    STATE_READY     = 1,
    STATE_PLAY      = 2,
    STATE_GAME_OVER = 3,
} game_state_t;

game_state_t game_state;
void go_to_state(game_state_t state);

void (*process_events)();
void (*update)(float dt);
void (*render)();

int mouse_button_down = 0;

int menu_button_offset_y = 0;
int game_over_button_offset_y = 0;

float logo_offset   = 0; // menu
float ground_offset = 0; // menu, state, play

SDL_Event event;

int      running;
int      game_over;
uint64_t ticks;

int score;
int max_score;
int new_max_score;

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

int pause = 0;

SDL_Thread *save_thread;

const size_t save_file_path_len = 256;
char save_file_path[save_file_path_len];

int load_file() {
    FILE* f = fopen(save_file_path, "r");
    if (!f) {
        if (errno == ENOENT) {
            printf("save file doesn't exist, max_score = 0\n");
            return 0;
        }

        perror("load: failed to open file");
        return 1;
    }

    if (fscanf(f, "%d\n", &max_score) < 0) {
        perror("load: failed to read from file");
        fclose(f);
        return 1;
    }

    printf("save: successfully read from file. max_score = %d\n", max_score);
    fclose(f);

    return 0;
}

const size_t save_file_errlen = 128;
char save_file_err[save_file_errlen];

typedef enum user_codes_t {
    USER_CODE_SAVE_SUCCESS = 0,
    USER_CODE_SAVE_ERROR   = 1,
} user_codes_t;

int save_file(void *data) {
    SDL_Event event;

    event.type = SDL_USEREVENT;

    FILE* f = fopen(save_file_path, "w+");
    if (!f) {
        event.user.code = USER_CODE_SAVE_ERROR;
        event.user.data1 = (void *)save_file_err;
        snprintf(save_file_err, save_file_errlen, "failed to save: %s", strerror(errno));
        SDL_PushEvent(&event);
        fclose(f);
        return 1;
    }

    if (fprintf(f, "%d\n", max_score) < 0) {
        event.user.code = USER_CODE_SAVE_ERROR;
        event.user.data1 = (void *)save_file_err;
        snprintf(save_file_err, save_file_errlen, "failed to save: %s", strerror(errno));
        SDL_PushEvent(&event);
        fclose(f);
        return 1;
    }

    event.user.code = USER_CODE_SAVE_SUCCESS;
    event.user.data1 = NULL;
    SDL_PushEvent(&event);
    fclose(f);

    printf("end of save thread\n");

    return 0;
}

void reset() {
    game_over = 0;
    player_y = ((window_height) - (window_height / 8.0f)) / 2 - sprite_height(&SPRITE_PLAYERS[0]) / 2;
    player_velocity_y = player_velocity_y_initial;
    pipes_len = 0;
    score = 0;
    pipe_to_pass = 0;
    new_max_score = 0;
}

void jump() {
    Mix_PlayChannel(-1, sfx_wing, 0);
    player_velocity_y = jump_velocity_x;
}

int get_gap_y() {
    int min_y = ((window_height) - (window_height / 8.0f)) / 2 - PIPE_GAP / 2 - 400;
    int max_y = ((window_height) - (window_height / 8.0f)) / 2 - PIPE_GAP / 2 + 400;
    int range = max_y - min_y;
    return (rand() % range) + min_y;
}

void get_rect_background(SDL_FRect *rect) {
    rect->x = 0;
    rect->y = 0;
    rect->w = window_width;
    rect->h = window_height;
}

void get_rect_menu_logo(SDL_FRect *rect) {
    rect->x = (window_width / 2) - sprite_width(&SPRITE_LOGO) / 2 - sprite_width(player_sprite) / 2 - player_sprite->w;
    rect->y = 300 + logo_offset;
    rect->w = sprite_width(&SPRITE_LOGO);
    rect->h = sprite_height(&SPRITE_LOGO);
}

void get_rect_menu_player(SDL_FRect *rect) {
    rect->x = (window_width / 2) + sprite_width(&SPRITE_LOGO) / 2  - sprite_width(player_sprite) / 2 + player_sprite->w;
    rect->y = 300 + logo_offset + sprite_height(player_sprite) / 3;
    rect->w = sprite_width(player_sprite);
    rect->h = sprite_height(player_sprite);
}

void get_rect_menu_button(SDL_FRect *rect) {
    rect->x = (window_width / 2) - sprite_width(&SPRITE_BUTTON_START) / 2;
    rect->y = (window_height / 4.0f) * 3.0f + menu_button_offset_y;
    rect->w = sprite_width(&SPRITE_BUTTON_START);
    rect->h = sprite_height(&SPRITE_BUTTON_START);
}

void get_rect_ready_get_ready(SDL_FRect *rect) {
    rect->x = (window_width / 2) - sprite_width(&SPRITE_GET_READY) / 2;
    rect->y = (window_height / 4.0f);
    rect->w = sprite_width(&SPRITE_GET_READY);
    rect->h = sprite_height(&SPRITE_GET_READY);
}

void get_rect_ready_tap(SDL_FRect *rect) {
    rect->x = (window_width / 2) - sprite_width(&SPRITE_TAP) / 2;
    rect->y = (window_height / 2) - sprite_height(&SPRITE_TAP) / 2;
    rect->w = sprite_width(&SPRITE_TAP);
    rect->h = sprite_height(&SPRITE_TAP);
}

void get_rect_play_pause(SDL_FRect *rect) {
    rect->x = (window_width / 10);
    rect->y = (window_width / 10);
    rect->w = sprite_width(&SPRITE_BUTTON_PAUSE);
    rect->h = sprite_height(&SPRITE_BUTTON_PAUSE);
}

void get_rect_game_over(SDL_FRect *rect) {
    rect->x = (window_width / 2) - sprite_width(&SPRITE_GAME_OVER) / 2;
    rect->y = (window_height / 4.0f);
    rect->w = sprite_width(&SPRITE_GAME_OVER);
    rect->h = sprite_height(&SPRITE_GAME_OVER);
}

void get_rect_game_over_board(SDL_FRect *rect) {
    rect->x = (window_width / 2) - sprite_width(&SPRITE_BOARD) / 2;
    rect->y = (window_height / 2) - sprite_height(&SPRITE_BOARD) / 2;
    rect->w = sprite_width(&SPRITE_BOARD);
    rect->h = sprite_height(&SPRITE_BOARD);
}

void get_rect_game_over_button(SDL_FRect *rect) {
    rect->x = (window_width / 2) - sprite_width(&SPRITE_BUTTON_OK) / 2;
    rect->y = (window_height / 4.0f) * 3.0f + game_over_button_offset_y;
    rect->w = sprite_width(&SPRITE_BUTTON_OK);
    rect->h = sprite_height(&SPRITE_BUTTON_OK);
}

void get_rect_player(SDL_FRect *rect) {
    rect->x = PLAYER_X;
    rect->y = player_y;
    rect->w = sprite_width(player_sprite);
    rect->h = sprite_height(player_sprite);
}

void get_rect_player_collider(SDL_FRect *rect) {
    rect->x = PLAYER_X + 1 * SPRITE_SCALE;
    rect->y = player_y + 1 * SPRITE_SCALE;
    rect->w = sprite_width(player_sprite) - 2 * SPRITE_SCALE;
    rect->h = sprite_height(player_sprite) - 2 * SPRITE_SCALE;
}

void get_pipe_top_rect(int i, SDL_FRect *rect) {
    rect->x = pipes[i].x;
    rect->y = -1000;
    rect->w = sprite_width(&SPRITE_PIPE);
    rect->h = pipes[i].gap_y + 1000;
}

void get_pipe_top_end_rect(int i, SDL_FRect *rect) {
    rect->x = pipes[i].x;
    rect->y = pipes[i].gap_y - (SPRITE_PIPE_TOP.h * SPRITE_SCALE);
    rect->w = sprite_width(&SPRITE_PIPE);
    rect->h = (SPRITE_PIPE_TOP.h * SPRITE_SCALE);
}

void get_pipe_bottom_rect(int i, SDL_FRect *rect) {
    rect->x = pipes[i].x;
    rect->y = pipes[i].gap_y + PIPE_GAP;
    rect->w = sprite_width(&SPRITE_PIPE);
    rect->h = window_height - (window_height / 8.0f) - (pipes[i].gap_y + PIPE_GAP);
}

void get_pipe_bottom_end_rect(int i, SDL_FRect *rect) {
    rect->x = pipes[i].x;
    rect->y = pipes[i].gap_y + PIPE_GAP;
    rect->w = sprite_width(&SPRITE_PIPE);
    rect->h = (SPRITE_PIPE_TOP.h * SPRITE_SCALE);
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
    rect->w = sprite_width(&SPRITE_GROUND);
    rect->h = (window_height / 8.0f) * 1;
}

void draw_ground(SDL_FRect *rect) {
    for (int i = 0; ; i++) {
        int ground_x = ground_offset + sprite_width(&SPRITE_GROUND) * i;
        if (ground_x > window_width) {
            break;
        }

        get_ground_segment_rect(ground_x, rect);
        SDL_RenderCopyF(renderer, texture, &SPRITE_GROUND, rect);
    }
}

void draw_score(SDL_FRect *rect) {
    if (score < 10) {
        rect->x = (window_width / 2) - ((SPRITE_NUMBERS[score].w * SPRITE_SCALE) / 2);
        rect->y = 200;
        rect->w = SPRITE_NUMBERS[score].w * SPRITE_SCALE;
        rect->h = SPRITE_NUMBERS[score].h * SPRITE_SCALE;
        SDL_RenderCopyF(renderer, texture, &SPRITE_NUMBERS[score], rect);
    } else if (score < 100) {
        rect->x = (window_width / 2) - ((SPRITE_NUMBERS[score / 10 % 10].w * SPRITE_SCALE) / 2) - (SPRITE_NUMBERS[score / 10 % 10].w * SPRITE_SCALE) / 1.9f;
        rect->y = 200;
        rect->w = SPRITE_NUMBERS[score / 10 % 10].w * SPRITE_SCALE;
        rect->h = SPRITE_NUMBERS[score / 10 % 10].h * SPRITE_SCALE;
        SDL_RenderCopyF(renderer, texture, &SPRITE_NUMBERS[score / 10 % 10], rect);

        rect->x = (window_width / 2) - ((SPRITE_NUMBERS[score % 10].w * SPRITE_SCALE) / 2) + (SPRITE_NUMBERS[score % 10].w * SPRITE_SCALE) / 1.9f;
        rect->y = 200;
        rect->w = SPRITE_NUMBERS[score % 10].w * SPRITE_SCALE;
        rect->h = SPRITE_NUMBERS[score % 10].h * SPRITE_SCALE;
        SDL_RenderCopyF(renderer, texture, &SPRITE_NUMBERS[score % 10], rect);
    } else if (score < 1000) {
        rect->x = (window_width / 2) - ((SPRITE_NUMBERS[score / 100].w * SPRITE_SCALE) / 2) - 1.1f * (SPRITE_NUMBERS[score / 100].w * SPRITE_SCALE);
        rect->y = 200;
        rect->w = SPRITE_NUMBERS[score / 100].w * SPRITE_SCALE;
        rect->h = SPRITE_NUMBERS[score / 100].h * SPRITE_SCALE;
        SDL_RenderCopyF(renderer, texture, &SPRITE_NUMBERS[score / 100], rect);

        rect->x = (window_width / 2) - ((SPRITE_NUMBERS[score / 10 % 10].w * SPRITE_SCALE) / 2);
        rect->y = 200;
        rect->w = SPRITE_NUMBERS[score / 10 % 10].w * SPRITE_SCALE;
        rect->h = SPRITE_NUMBERS[score / 10 % 10].h * SPRITE_SCALE;
        SDL_RenderCopyF(renderer, texture, &SPRITE_NUMBERS[score / 10 % 10], rect);

        rect->x = (window_width / 2) - ((SPRITE_NUMBERS[score % 10].w * SPRITE_SCALE) / 2) + 1.1f * (SPRITE_NUMBERS[score % 10].w * SPRITE_SCALE);
        rect->y = 200;
        rect->w = SPRITE_NUMBERS[score % 10].w * SPRITE_SCALE;
        rect->h = SPRITE_NUMBERS[score % 10].h * SPRITE_SCALE;
        SDL_RenderCopyF(renderer, texture, &SPRITE_NUMBERS[score % 10], rect);
    }
}

void draw_score_small(SDL_FRect *rect) {
    SDL_FRect board_rect;

    get_rect_game_over_board(&board_rect);

    int score_cache = score;
    int digit;
    int digit_width;
    int digit_height;

    int placement_x = board_rect.x + board_rect.w - 76;

    while (1) {
        digit = score_cache % 10;
        digit_width = sprite_width(&SPRITE_NUMBERS_SM[digit]);
        digit_height = sprite_height(&SPRITE_NUMBERS_SM[digit]);

        rect->x = placement_x - digit_width;
        rect->y = board_rect.y + 125;
        rect->w = digit_width;
        rect->h = digit_height;
        SDL_RenderCopyF(renderer, texture, &SPRITE_NUMBERS_SM[digit], rect);

        score_cache /= 10;
        if (score_cache == 0) {
            break;
        }

        placement_x = placement_x - digit_width - 6;
    }
}

void draw_max_score_small(SDL_FRect *rect) {
    SDL_FRect board_rect;
    get_rect_game_over_board(&board_rect);

    int score_cache = max_score;
    int digit;
    int digit_width;
    int digit_height;

    int placement_x = board_rect.x + board_rect.w - 76;

    while (1) {
        digit = score_cache % 10;
        digit_width = sprite_width(&SPRITE_NUMBERS_SM[digit]);
        digit_height = sprite_height(&SPRITE_NUMBERS_SM[digit]);

        rect->x = placement_x - digit_width;
        rect->y = board_rect.y + 270;
        rect->w = digit_width;
        rect->h = digit_height;
        SDL_RenderCopyF(renderer, texture, &SPRITE_NUMBERS_SM[digit], rect);

        placement_x = placement_x - digit_width - 6;

        score_cache /= 10;
        if (score_cache == 0) {
            break;
        }
    }

    if (new_max_score) {
        rect->x = placement_x - 30 - sprite_width(&SPRITE_NEW);
        rect->y = board_rect.y + 270;
        rect->w = sprite_width(&SPRITE_NEW);
        rect->h = sprite_height(&SPRITE_NEW);
        SDL_RenderCopyF(renderer, texture, &SPRITE_NEW, rect);
    }
}

int mouse_is_in_rect(const SDL_FRect *rect) {
    int x, y;
    SDL_GetMouseState(&x, &y);
    SDL_FPoint point = {x * window_width_ratio, y * window_height_ratio};
    return SDL_PointInFRect(&point, rect);
}

void process_events_menu() {
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
                        go_to_state(STATE_READY);
                        reset();
                    default:
                        break;
                }
                break;
            case SDL_MOUSEBUTTONUP: {
                SDL_FRect rect;
                get_rect_menu_button(&rect);
                if (mouse_is_in_rect(&rect)) {
                    go_to_state(STATE_READY);
                }
                break;
            }
            case SDL_WINDOWEVENT:
                SDL_GetRendererOutputSize(renderer, &window_width, &window_height);
                break;
            default:
                break;
        }
    }

    mouse_button_down = SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_LMASK;
}

void process_events_ready() {
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
                        go_to_state(STATE_MENU);
                        break;
                    case SDLK_SPACE:
                        go_to_state(STATE_PLAY);
                        break;
                    default:
                        break;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                go_to_state(STATE_PLAY);
                break;
            case SDL_WINDOWEVENT:
                SDL_GetRendererOutputSize(renderer, &window_width, &window_height);
                break;
            default:
                break;
        }
    }

    mouse_button_down = SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_LMASK;
}

void process_events_play() {
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
                        go_to_state(STATE_MENU);
                        break;
                    case SDLK_SPACE:
                        jump();
                        break;
                    default:
                        break;
                }
                break;
            case SDL_MOUSEBUTTONDOWN: {
                SDL_FRect rect;
                get_rect_play_pause(&rect);
                if (mouse_is_in_rect(&rect)) {
                    pause = !pause;
                    break;
                }

                jump();
                break;
            }
            case SDL_WINDOWEVENT:
                SDL_GetRendererOutputSize(renderer, &window_width, &window_height);
                break;
            default:
                break;
        }
    }

    mouse_button_down = SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_LMASK;
}

void process_events_game_over() {
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
                        go_to_state(STATE_MENU);
                        break;
                    case SDLK_SPACE:
                        go_to_state(STATE_MENU);
                        break;
                    default:
                        break;
                }
                break;
            case SDL_MOUSEBUTTONUP: {
                SDL_FRect rect;
                get_rect_game_over_button(&rect);
                if (mouse_is_in_rect(&rect)) {
                    go_to_state(STATE_MENU);
                }
                break;
            }
            case SDL_WINDOWEVENT:
                SDL_GetRendererOutputSize(renderer, &window_width, &window_height);
                break;
            case SDL_USEREVENT:
                switch (event.user.code) {
                    case USER_CODE_SAVE_SUCCESS:
                        printf("successful file save, max_score = %d\n", max_score);
                        break;
                    case USER_CODE_SAVE_ERROR:
                        printf("error saving file: %s\n", (const char *)event.user.data1);
                        break;
                    default:
                        break;
                }
            default:
                break;
        }
    }

    mouse_button_down = SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_LMASK;
}

void update_menu(float dt) {
    logo_offset = sin(ticks / 200.0) * 10;

    player_sprite = &SPRITE_PLAYERS[ticks % 300 / 100];

    ground_offset += pipe_velocity_x * dt;
    ground_offset = fmodf(ground_offset, SPRITE_GROUND.w * SPRITE_SCALE);

    menu_button_offset_y = mouse_button_down * 1 * SPRITE_SCALE;
}

void update_ready(float dt) {
    logo_offset = sin(ticks / 200.0) * 10;

    player_sprite = &SPRITE_PLAYERS[ticks % 300 / 100];

    ground_offset += pipe_velocity_x * dt;
    ground_offset = fmodf(ground_offset, SPRITE_GROUND.w * SPRITE_SCALE);
}

void update_play(float dt) {
    if (pause) {
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

    // If the last pipe is futher away than PIPE_SPACING, create a new one
    if (pipes_len < max_pipes && pipes[pipes_len - 1].x < window_width - PIPE_SPACING) {
        pipes[pipes_len].x = window_width;
        pipes[pipes_len].gap_y = get_gap_y();
        pipes_len++; 
    }

    // Remove pipes than the player has passed
    while (pipes[0].x + sprite_width(&SPRITE_PIPE) < 0) {
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

    get_rect_player_collider(&a);

    get_bottom_rect(&b);
    if (SDL_HasIntersectionF(&a, &b)) {
        go_to_state(STATE_GAME_OVER);
        return;
    }

    for (int i = 0; i < pipes_len; i++) {
        get_pipe_top_rect(i, &b);
        if (SDL_HasIntersectionF(&a, &b)) {
            go_to_state(STATE_GAME_OVER);
            return;
        }

        get_pipe_bottom_rect(i, &b);
        if (SDL_HasIntersectionF(&a, &b)) {
            go_to_state(STATE_GAME_OVER);
            return;
        }
    }
#endif // COLLISION_DETECTION

    if (PLAYER_X > pipes[pipe_to_pass].x) {
        Mix_PlayChannel(-1, sfx_point, 0);
        score++;
        pipe_to_pass++;
    }
}

void update_game_over(float dt) {
    game_over_button_offset_y = mouse_button_down * 1 * SPRITE_SCALE;
}

void render_menu() {
    SDL_FRect rect;

    get_rect_background(&rect);
    SDL_RenderCopyF(renderer, texture, &SPRITE_BACKGROUND, &rect);

    get_rect_menu_logo(&rect);
    SDL_RenderCopyF(renderer, texture, &SPRITE_LOGO, &rect);

    get_rect_menu_player(&rect);
    SDL_RenderCopyF(renderer, texture, player_sprite, &rect);

    draw_ground(&rect);

    get_rect_menu_button(&rect);
    SDL_RenderCopyF(renderer, texture, &SPRITE_BUTTON_START, &rect);

    SDL_RenderPresent(renderer);
}

void render_ready() {
    SDL_FRect rect;

    get_rect_background(&rect);
    SDL_RenderCopyF(renderer, texture, &SPRITE_BACKGROUND, &rect);

    get_rect_ready_get_ready(&rect);
    SDL_RenderCopyF(renderer, texture, &SPRITE_GET_READY, &rect);

    get_rect_player(&rect);
    SDL_RenderCopyF(renderer, texture, player_sprite, &rect);

    draw_ground(&rect);

    get_rect_ready_tap(&rect);
    SDL_RenderCopyF(renderer, texture, &SPRITE_TAP, &rect);

    draw_score(&rect);

    SDL_RenderPresent(renderer);
}

void render_play() {
    SDL_FRect rect;

    get_rect_background(&rect);
    SDL_RenderCopyF(renderer, texture, &SPRITE_BACKGROUND, &rect);

    draw_ground(&rect);

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

#if DRAW_PLAYER_COLLIDER
    get_rect_player_collider(&rect);
    SDL_SetRenderDrawColor(renderer, 200, 100, 100, 100);
    SDL_RenderFillRectF(renderer, &rect);
#endif // DRAW_PLAYER_COLLIDER

    get_rect_player(&rect);
    SDL_RenderCopyF(renderer, texture, player_sprite, &rect);

    get_rect_play_pause(&rect);
    if (pause) {
        SDL_RenderCopyF(renderer, texture, &SPRITE_BUTTON_PLAY, &rect);
    } else {
        SDL_RenderCopyF(renderer, texture, &SPRITE_BUTTON_PAUSE, &rect);
    }

    draw_score(&rect);

    SDL_RenderPresent(renderer);
}

void render_game_over() {
    SDL_FRect rect;

    get_rect_background(&rect);
    SDL_RenderCopyF(renderer, texture, &SPRITE_BACKGROUND, &rect);

    draw_ground(&rect);

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

#if DRAW_PLAYER_COLLIDER
    get_rect_player_collider(&rect);
    SDL_SetRenderDrawColor(renderer, 200, 100, 100, 100);
    SDL_RenderFillRectF(renderer, &rect);
#endif // DRAW_PLAYER_COLLIDER

    get_rect_player(&rect);
    SDL_RenderCopyF(renderer, texture, player_sprite, &rect);

    draw_score(&rect);

    get_rect_game_over(&rect);
    SDL_RenderCopyF(renderer, texture, &SPRITE_GAME_OVER, &rect);

    get_rect_game_over_board(&rect);
    SDL_RenderCopyF(renderer, texture, &SPRITE_BOARD, &rect);

    draw_score_small(&rect);
    draw_max_score_small(&rect);

    get_rect_game_over_button(&rect);
    SDL_RenderCopyF(renderer, texture, &SPRITE_BUTTON_OK, &rect);

    SDL_RenderPresent(renderer);
}

void go_to_state(game_state_t state) {
    game_state = state;

    printf("changing state to %d\n", state);

    switch (state) {
        case STATE_MENU:
            process_events = process_events_menu;
            update = update_menu;
            render = render_menu;
            reset();
            break;
        case STATE_READY:
            process_events = process_events_ready;
            update = update_ready;
            render = render_ready;
            Mix_PlayChannel(-1, sfx_swooshing, 0);
            break;
        case STATE_PLAY:
            process_events = process_events_play;
            update = update_play;
            render = render_play;
            reset();
            jump();
            break;
        case STATE_GAME_OVER: {
            process_events = process_events_game_over;
            update = update_game_over;
            render = render_game_over;

            Mix_PlayChannel(-1, sfx_hit, 0);

            if (score <= max_score) {
                break;
            }

            new_max_score = 1;
            max_score = score;

            save_thread = SDL_CreateThread(save_file, "save_file", NULL);
            if (!save_thread) {
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", SDL_GetError(), window);
            }
            SDL_DetachThread(save_thread);
            break;
        }
        default:
            break;
    }
}

void run() {
    running            = 1;
    uint64_t lastTicks = SDL_GetTicks64();

    go_to_state(STATE_MENU);

    while (running) {
        ticks     = SDL_GetTicks64();
        float dt  = (ticks - lastTicks) / 1000.0f;
        lastTicks = ticks;

        process_events();
        update(dt);
        render();
    }
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    const size_t errlen = 128;
    char errstr[errlen];

#if defined(__IPHONEOS__)
    snprintf(save_file_path, save_file_path_len, "%s/Documents/data.txt", getenv("HOME"));
#else
    snprintf(save_file_path, save_file_path_len, "data.txt");
#endif

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        snprintf(errstr, errlen, "Error initialising SDL: %s", SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", errstr, window);
        return 1;
    }

    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        snprintf(errstr, errlen, "Error initialising SDL_image: %s", IMG_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", errstr, window);
        return 1;
    }

    if (Mix_Init(0) != 0) {
        snprintf(errstr, errlen, "Error initialising SDL_mixer: %s", Mix_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", errstr, window);
        return 1;
    }

    window = SDL_CreateWindow("Flappy Bird", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH_INITIAL, WINDOW_HEIGHT_INITIAL, WINDOW_FLAGS);
    if (!window) {
        snprintf(errstr, errlen, "Error creating window: %s", SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", errstr, window);
        return 1;
    }

    SDL_GetWindowSize(window, &window_width_real, &window_height_real);
    printf("window_width_real = %d, window_height_real = %d\n", window_width_real, window_height_real);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        snprintf(errstr, errlen, "Error creating renderer: %s", SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", errstr, window);
        return 1;
    }

    if (SDL_GetRendererOutputSize(renderer, &window_width, &window_height) != 0) {
        snprintf(errstr, errlen, "Error getting renderer output size: %s", SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", errstr, window);
        return 1;
    }
    printf("window_width = %d, window_height = %d\n", window_width, window_height);

    window_width_ratio = (float)window_width / (float)window_width_real;
    window_height_ratio = (float)window_height / (float)window_height_real;

    texture = IMG_LoadTexture(renderer, "spritesheet.png");
    if (!texture) {
        snprintf(errstr, errlen, "Error loading sprite sheet: %s", IMG_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", errstr, window);
        return 1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0) {
        snprintf(errstr, errlen, "Error opening audio device: %s", Mix_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", errstr, window);
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

    if (load_file() != 0) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", strerror(errno), window);
        return 1;
    }

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
