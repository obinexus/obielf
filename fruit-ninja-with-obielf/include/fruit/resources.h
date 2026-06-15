#ifndef FRUIT_RESOURCES_H
#define FRUIT_RESOURCES_H

#include "fruit/game.h"
#include "obielf/resource_pack.h"

#include <SDL.h>

typedef struct fruit_sprite_set {
    SDL_Texture *whole;
    SDL_Texture *half1;
    SDL_Texture *half2;
} fruit_sprite_set_t;

typedef struct fruit_resources {
    SDL_Renderer *renderer;
    obielf_resource_pack_t pack;
    fruit_sprite_set_t fruit[FRUIT_BOMB];
    SDL_Texture *background;
    SDL_Texture *bomb;
    SDL_Texture *explosion;
    SDL_Texture *splash_red;
    SDL_Texture *splash_orange;
    SDL_Texture *splash_yellow;
    SDL_Texture *splash_transparent;
    int embedded;
} fruit_resources_t;

int fruit_resources_init(
    fruit_resources_t *resources,
    SDL_Renderer *renderer,
    SDL_Window *window);

void fruit_resources_destroy(fruit_resources_t *resources);

int fruit_resources_verify(const fruit_resources_t *resources);

const char *fruit_resources_mode(const fruit_resources_t *resources);

#endif

