#include "fruit/config.h"
#include "fruit/game.h"
#include "fruit/resources.h"

#include <SDL.h>
#include <SDL_image.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct blade_point {
    float x;
    float y;
    float age;
} blade_point_t;

typedef struct blade_trail {
    blade_point_t points[FRUIT_MAX_TRAIL_POINTS];
    size_t count;
    int active;
} blade_trail_t;

typedef struct program_options {
    int verify_assets;
    int smoke_frames;
    const char *screenshot_path;
} program_options_t;

static const char *glyph_for(char character)
{
    switch (character) {
    case 'A':
        return "01110100011000111111100011000110001";
    case 'B':
        return "11110100011000111110100011000111110";
    case 'C':
        return "01111100001000010000100001000001111";
    case 'D':
        return "11110100011000110001100011000111110";
    case 'E':
        return "11111100001000011110100001000011111";
    case 'F':
        return "11111100001000011110100001000010000";
    case 'G':
        return "01111100001000010111100011000101111";
    case 'H':
        return "10001100011000111111100011000110001";
    case 'I':
        return "11111001000010000100001000010011111";
    case 'J':
        return "00111000100001000010000101001001100";
    case 'K':
        return "10001100101010011000101001001010001";
    case 'L':
        return "10000100001000010000100001000011111";
    case 'M':
        return "10001110111010110101100011000110001";
    case 'N':
        return "10001110011010110011100011000110001";
    case 'O':
        return "01110100011000110001100011000101110";
    case 'P':
        return "11110100011000111110100001000010000";
    case 'Q':
        return "01110100011000110001101011001001101";
    case 'R':
        return "11110100011000111110101001001010001";
    case 'S':
        return "01111100001000001110000010000111110";
    case 'T':
        return "11111001000010000100001000010000100";
    case 'U':
        return "10001100011000110001100011000101110";
    case 'V':
        return "10001100011000110001100010101000100";
    case 'W':
        return "10001100011000110101101011101110001";
    case 'X':
        return "10001100010101000100010101000110001";
    case 'Y':
        return "10001100010101000100001000010000100";
    case 'Z':
        return "11111000010001000100010001000011111";
    case '0':
        return "01110100011001110101110011000101110";
    case '1':
        return "00100011000010000100001000010001110";
    case '2':
        return "01110100010000100010001000100011111";
    case '3':
        return "11110000010000101110000010000111110";
    case '4':
        return "00010001100101010010111110001000010";
    case '5':
        return "11111100001000011110000010000111110";
    case '6':
        return "01110100001000011110100011000101110";
    case '7':
        return "11111000010001000100010000100001000";
    case '8':
        return "01110100011000101110100011000101110";
    case '9':
        return "01110100011000101111000010000101110";
    case ':':
        return "00000001000010000000001000010000000";
    case '-':
        return "00000000000000011111000000000000000";
    case '.':
        return "00000000000000000000000000011000110";
    default:
        return "00000000000000000000000000000000000";
    }
}

static void draw_text(
    SDL_Renderer *renderer,
    int x,
    int y,
    int scale,
    const char *text,
    SDL_Color color)
{
    const size_t length = strlen(text);
    size_t character_index;

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (character_index = 0u; character_index < length; ++character_index) {
        const char *glyph = glyph_for(text[character_index]);
        int row;
        int column;

        for (row = 0; row < 7; ++row) {
            for (column = 0; column < 5; ++column) {
                if (glyph[(row * 5) + column] == '1') {
                    SDL_Rect pixel = {
                        x + ((int)character_index * 6 * scale) +
                            (column * scale),
                        y + (row * scale),
                        scale,
                        scale};
                    SDL_RenderFillRect(renderer, &pixel);
                }
            }
        }
    }
}

static void render_texture_centered(
    SDL_Renderer *renderer,
    SDL_Texture *texture,
    float x,
    float y,
    float size,
    float angle)
{
    SDL_FRect destination = {
        x - (size * 0.5f), y - (size * 0.5f), size, size};
    if (texture != NULL) {
        SDL_RenderCopyExF(
            renderer,
            texture,
            NULL,
            &destination,
            (double)angle,
            NULL,
            SDL_FLIP_NONE);
    }
}

static SDL_Texture *splash_for_kind(
    const fruit_resources_t *resources,
    fruit_kind_t kind)
{
    switch (kind) {
    case FRUIT_ORANGE:
        return resources->splash_orange;
    case FRUIT_BANANA:
    case FRUIT_PINEAPPLE:
        return resources->splash_yellow;
    case FRUIT_COCONUT:
        return resources->splash_transparent;
    default:
        return resources->splash_red;
    }
}

static void render_entity(
    SDL_Renderer *renderer,
    const fruit_resources_t *resources,
    const fruit_entity_t *entity)
{
    if (!entity->active) {
        return;
    }
    if (entity->state == FRUIT_ENTITY_WHOLE) {
        SDL_Texture *texture =
            entity->kind == FRUIT_BOMB
                ? resources->bomb
                : resources->fruit[entity->kind].whole;
        render_texture_centered(
            renderer, texture, entity->x, entity->y, 86.0f, entity->angle);
    } else if (entity->state == FRUIT_ENTITY_SLICED) {
        SDL_Texture *splash = splash_for_kind(resources, entity->kind);
        uint8_t alpha = 0u;

        render_texture_centered(
            renderer,
            resources->fruit[entity->kind].half1,
            entity->half1_x,
            entity->half1_y,
            70.0f,
            entity->half1_angle);
        render_texture_centered(
            renderer,
            resources->fruit[entity->kind].half2,
            entity->half2_x,
            entity->half2_y,
            70.0f,
            entity->half2_angle);

        if (entity->effect_time > 0.0f) {
            float normalized = entity->effect_time / 0.5f;
            if (normalized > 1.0f) {
                normalized = 1.0f;
            }
            alpha = (uint8_t)(normalized * 210.0f);
            SDL_SetTextureAlphaMod(splash, alpha);
            render_texture_centered(
                renderer, splash, entity->x, entity->y, 130.0f, 0.0f);
            SDL_SetTextureAlphaMod(splash, UINT8_MAX);
        }
    } else {
        render_texture_centered(
            renderer,
            resources->explosion,
            entity->x,
            entity->y,
            190.0f,
            0.0f);
    }
}

static void blade_add_point(blade_trail_t *trail, float x, float y)
{
    if (trail->count == FRUIT_MAX_TRAIL_POINTS) {
        memmove(
            &trail->points[0],
            &trail->points[1],
            (FRUIT_MAX_TRAIL_POINTS - 1u) * sizeof(trail->points[0]));
        --trail->count;
    }
    trail->points[trail->count].x = x;
    trail->points[trail->count].y = y;
    trail->points[trail->count].age = 0.0f;
    ++trail->count;
}

static void blade_update(blade_trail_t *trail, float delta_seconds)
{
    size_t index;
    size_t first_visible = 0u;

    for (index = 0u; index < trail->count; ++index) {
        trail->points[index].age += delta_seconds;
        if (trail->points[index].age > 0.22f) {
            first_visible = index + 1u;
        }
    }
    if (first_visible > 0u) {
        memmove(
            &trail->points[0],
            &trail->points[first_visible],
            (trail->count - first_visible) * sizeof(trail->points[0]));
        trail->count -= first_visible;
    }
}

static void render_blade(SDL_Renderer *renderer, const blade_trail_t *trail)
{
    size_t index;

    if (trail->count < 2u) {
        return;
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    for (index = 1u; index < trail->count; ++index) {
        const blade_point_t *previous = &trail->points[index - 1u];
        const blade_point_t *current = &trail->points[index];
        const float fade = 1.0f - (current->age / 0.22f);
        const uint8_t alpha =
            (uint8_t)((fade > 0.0f ? fade : 0.0f) * 220.0f);
        int thickness;

        SDL_SetRenderDrawColor(renderer, 0u, 190u, 235u, alpha);
        for (thickness = -3; thickness <= 3; ++thickness) {
            SDL_RenderDrawLineF(
                renderer,
                previous->x,
                previous->y + (float)thickness,
                current->x,
                current->y + (float)thickness);
        }
        SDL_SetRenderDrawColor(renderer, 255u, 255u, 255u, alpha);
        SDL_RenderDrawLineF(
            renderer,
            previous->x,
            previous->y,
            current->x,
            current->y);
    }
}

static void render_game(
    SDL_Renderer *renderer,
    const fruit_resources_t *resources,
    const fruit_game_t *game,
    const blade_trail_t *blade)
{
    SDL_FRect background = {
        0.0f,
        0.0f,
        (float)FRUIT_LOGICAL_WIDTH,
        (float)FRUIT_LOGICAL_HEIGHT};
    SDL_Color dark = {20u, 28u, 34u, 255u};
    SDL_Color red = {205u, 43u, 58u, 255u};
    SDL_Color cyan = {0u, 130u, 165u, 255u};
    char score[32];
    char lives[32];
    size_t index;

    SDL_SetRenderDrawColor(renderer, 242u, 239u, 231u, 255u);
    SDL_RenderClear(renderer);
    SDL_RenderCopyF(renderer, resources->background, NULL, &background);

    for (index = 0u; index < FRUIT_MAX_ENTITIES; ++index) {
        render_entity(renderer, resources, &game->entities[index]);
    }
    render_blade(renderer, blade);

    (void)snprintf(score, sizeof(score), "SCORE %04d", game->score);
    (void)snprintf(lives, sizeof(lives), "LIVES %d", game->lives);
    draw_text(renderer, 24, 22, 3, score, dark);
    draw_text(renderer, 760, 22, 3, lives, red);
    draw_text(
        renderer,
        24,
        505,
        2,
        resources->embedded ? "OBIELF EMBEDDED" : "DIRECT PATH",
        cyan);

    if (game->phase == FRUIT_GAME_READY) {
        SDL_SetRenderDrawColor(renderer, 255u, 255u, 255u, 210u);
        {
            SDL_Rect panel = {190, 175, 580, 185};
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_RenderFillRect(renderer, &panel);
        }
        draw_text(renderer, 270, 205, 5, "SLICE FRUIT", red);
        draw_text(renderer, 285, 275, 3, "DRAG TO SLICE", dark);
        draw_text(renderer, 300, 315, 2, "SPACE TO START", cyan);
    } else if (game->phase == FRUIT_GAME_OVER) {
        SDL_SetRenderDrawColor(renderer, 18u, 18u, 18u, 215u);
        {
            SDL_Rect panel = {220, 185, 520, 165};
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_RenderFillRect(renderer, &panel);
        }
        draw_text(renderer, 305, 215, 5, "GAME OVER", red);
        draw_text(renderer, 315, 290, 2, "R TO RESTART", (SDL_Color){
            245u, 245u, 245u, 255u});
    }
}

static int save_screenshot(SDL_Renderer *renderer, const char *path)
{
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(
        0,
        FRUIT_LOGICAL_WIDTH,
        FRUIT_LOGICAL_HEIGHT,
        32,
        SDL_PIXELFORMAT_ARGB8888);
    int result;

    if (surface == NULL) {
        return 0;
    }
    result = SDL_RenderReadPixels(
        renderer,
        NULL,
        SDL_PIXELFORMAT_ARGB8888,
        surface->pixels,
        surface->pitch);
    if (result == 0) {
        result = SDL_SaveBMP(surface, path);
    }
    SDL_FreeSurface(surface);
    return result == 0;
}

static int parse_positive_integer(const char *text)
{
    char *end = NULL;
    const long value = strtol(text, &end, 10);
    if (text == end || end == NULL || *end != '\0' || value <= 0L ||
        value > 100000L) {
        return 0;
    }
    return (int)value;
}

static int parse_options(
    int argc,
    char **argv,
    program_options_t *options)
{
    int index;

    memset(options, 0, sizeof(*options));
    for (index = 1; index < argc; ++index) {
        if (strcmp(argv[index], "--verify-assets") == 0) {
            options->verify_assets = 1;
        } else if (strcmp(argv[index], "--smoke-test") == 0 &&
                   index + 1 < argc) {
            options->smoke_frames =
                parse_positive_integer(argv[++index]);
            if (options->smoke_frames == 0) {
                return 0;
            }
        } else if (strcmp(argv[index], "--screenshot") == 0 &&
                   index + 1 < argc) {
            options->screenshot_path = argv[++index];
        } else if (strcmp(argv[index], "--help") == 0) {
            printf("usage: fruit-ninja-obielf [--verify-assets] "
                   "[--smoke-test FRAMES] [--screenshot FILE]\n");
            exit(EXIT_SUCCESS);
        } else {
            return 0;
        }
    }
    return 1;
}

static void window_to_logical(
    SDL_Renderer *renderer,
    int window_x,
    int window_y,
    float *logical_x,
    float *logical_y)
{
    SDL_RenderWindowToLogical(
        renderer, window_x, window_y, logical_x, logical_y);
}

static int push_smoke_mouse_swipe(void)
{
    SDL_Event event;

    memset(&event, 0, sizeof(event));
    event.type = SDL_MOUSEBUTTONDOWN;
    event.button.button = SDL_BUTTON_LEFT;
    event.button.state = SDL_PRESSED;
    event.button.x = 350;
    event.button.y = 270;
    if (SDL_PushEvent(&event) < 0) {
        return 0;
    }

    memset(&event, 0, sizeof(event));
    event.type = SDL_MOUSEMOTION;
    event.motion.state = SDL_BUTTON_LMASK;
    event.motion.x = 610;
    event.motion.y = 270;
    if (SDL_PushEvent(&event) < 0) {
        return 0;
    }

    memset(&event, 0, sizeof(event));
    event.type = SDL_MOUSEBUTTONUP;
    event.button.button = SDL_BUTTON_LEFT;
    event.button.state = SDL_RELEASED;
    event.button.x = 610;
    event.button.y = 270;
    return SDL_PushEvent(&event) >= 0;
}

int main(int argc, char **argv)
{
    program_options_t options;
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    fruit_resources_t resources;
    fruit_game_t game;
    blade_trail_t blade;
    uint64_t previous_counter;
    uint64_t frequency;
    int running = 1;
    int rendered_frames = 0;
    int exit_code = EXIT_FAILURE;
    uint32_t window_flags = SDL_WINDOW_RESIZABLE;

    if (!parse_options(argc, argv, &options)) {
        fprintf(stderr, "invalid arguments; use --help\n");
        return EXIT_FAILURE;
    }
    if (options.verify_assets || options.smoke_frames > 0) {
        window_flags |= SDL_WINDOW_HIDDEN;
    }

    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) != 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }
    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
        fprintf(stderr, "IMG_Init: %s\n", IMG_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }
    frequency = SDL_GetPerformanceFrequency();

    window = SDL_CreateWindow(
        FRUIT_WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        FRUIT_LOGICAL_WIDTH,
        FRUIT_LOGICAL_HEIGHT,
        window_flags);
    if (window == NULL) {
        fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        goto cleanup;
    }
    renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (renderer == NULL) {
        fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
        goto cleanup;
    }
    SDL_RenderSetLogicalSize(
        renderer, FRUIT_LOGICAL_WIDTH, FRUIT_LOGICAL_HEIGHT);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    if (!fruit_resources_init(&resources, renderer, window)) {
        fprintf(stderr, "resource initialization failed: %s\n",
                IMG_GetError());
        goto cleanup;
    }
    printf("resource mode: %s\n", fruit_resources_mode(&resources));
    if (!fruit_resources_verify(&resources)) {
        fprintf(stderr, "resource verification failed\n");
        fruit_resources_destroy(&resources);
        goto cleanup;
    }
    if (options.verify_assets) {
        printf("verified all native game assets\n");
        fruit_resources_destroy(&resources);
        exit_code = EXIT_SUCCESS;
        goto cleanup;
    }

    fruit_game_init(
        &game,
        options.smoke_frames > 0
            ? UINT32_C(0x00c0ffee)
            : (uint32_t)time(NULL));
    memset(&blade, 0, sizeof(blade));
    if (options.smoke_frames > 0) {
        fruit_game_start(&game);
        game.entities[0].active = 1;
        game.entities[0].kind = FRUIT_APPLE;
        game.entities[0].state = FRUIT_ENTITY_WHOLE;
        game.entities[0].x = 480.0f;
        game.entities[0].y = 270.0f;
    }
    previous_counter = SDL_GetPerformanceCounter();

    while (running) {
        SDL_Event event;
        float delta_seconds;

        if (options.smoke_frames > 0 && rendered_frames == 0 &&
            !push_smoke_mouse_swipe()) {
            fprintf(stderr, "failed to queue smoke-test mouse input\n");
            running = 0;
            exit_code = EXIT_FAILURE;
        }
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                running = 0;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = 0;
                } else if (
                    event.key.keysym.sym == SDLK_SPACE &&
                    game.phase == FRUIT_GAME_READY) {
                    fruit_game_start(&game);
                } else if (
                    event.key.keysym.sym == SDLK_r &&
                    game.phase == FRUIT_GAME_OVER) {
                    fruit_game_start(&game);
                }
            } else if (event.type == SDL_MOUSEBUTTONDOWN &&
                       event.button.button == SDL_BUTTON_LEFT) {
                float x;
                float y;
                if (game.phase != FRUIT_GAME_PLAYING) {
                    fruit_game_start(&game);
                }
                blade.active = 1;
                blade.count = 0u;
                window_to_logical(
                    renderer, event.button.x, event.button.y, &x, &y);
                blade_add_point(&blade, x, y);
            } else if (
                event.type == SDL_MOUSEMOTION && blade.active) {
                float x;
                float y;
                window_to_logical(
                    renderer, event.motion.x, event.motion.y, &x, &y);
                if (blade.count > 0u) {
                    const blade_point_t *previous =
                        &blade.points[blade.count - 1u];
                    (void)fruit_game_slice(
                        &game, previous->x, previous->y, x, y);
                }
                blade_add_point(&blade, x, y);
            } else if (
                event.type == SDL_MOUSEBUTTONUP &&
                event.button.button == SDL_BUTTON_LEFT) {
                blade.active = 0;
            } else if (event.type == SDL_FINGERDOWN) {
                const float x =
                    event.tfinger.x * (float)FRUIT_LOGICAL_WIDTH;
                const float y =
                    event.tfinger.y * (float)FRUIT_LOGICAL_HEIGHT;
                if (game.phase != FRUIT_GAME_PLAYING) {
                    fruit_game_start(&game);
                }
                blade.active = 1;
                blade.count = 0u;
                blade_add_point(&blade, x, y);
            } else if (
                event.type == SDL_FINGERMOTION && blade.active) {
                const float x =
                    event.tfinger.x * (float)FRUIT_LOGICAL_WIDTH;
                const float y =
                    event.tfinger.y * (float)FRUIT_LOGICAL_HEIGHT;
                if (blade.count > 0u) {
                    const blade_point_t *previous =
                        &blade.points[blade.count - 1u];
                    (void)fruit_game_slice(
                        &game, previous->x, previous->y, x, y);
                }
                blade_add_point(&blade, x, y);
            } else if (event.type == SDL_FINGERUP) {
                blade.active = 0;
            }
        }

        if (options.smoke_frames > 0) {
            delta_seconds = 1.0f / 60.0f;
            if (rendered_frames > 0 && rendered_frames % 20 == 0) {
                const float y =
                    160.0f + (float)((rendered_frames * 13) % 260);
                (void)fruit_game_slice(
                    &game, 20.0f, y, 940.0f, y);
            }
        } else {
            const uint64_t current_counter = SDL_GetPerformanceCounter();
            delta_seconds =
                (float)(current_counter - previous_counter) /
                (float)frequency;
            previous_counter = current_counter;
        }

        fruit_game_update(&game, delta_seconds);
        blade_update(&blade, delta_seconds);
        render_game(renderer, &resources, &game, &blade);
        SDL_RenderPresent(renderer);
        ++rendered_frames;

        if (options.smoke_frames > 0 &&
            rendered_frames >= options.smoke_frames) {
            running = 0;
        }
    }

    render_game(renderer, &resources, &game, &blade);
    SDL_RenderPresent(renderer);
    if (options.screenshot_path != NULL &&
        !save_screenshot(renderer, options.screenshot_path)) {
        fprintf(stderr, "screenshot failed: %s\n", SDL_GetError());
        fruit_resources_destroy(&resources);
        goto cleanup;
    }
    fruit_resources_destroy(&resources);
    printf("smoke frames: %d, score: %d, lives: %d\n",
           rendered_frames,
           game.score,
           game.lives);
    exit_code =
        options.smoke_frames > 0 && game.score == 0
            ? EXIT_FAILURE
            : EXIT_SUCCESS;

cleanup:
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return exit_code;
}
