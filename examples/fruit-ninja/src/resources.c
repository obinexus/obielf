#include "fruit/resources.h"

#include "fruit/config.h"

#include <SDL_image.h>

#include <limits.h>
#include <stdio.h>
#include <string.h>

#ifndef FRUIT_OBIELF_EMBED_ASSETS
#define FRUIT_OBIELF_EMBED_ASSETS 0
#endif

#if FRUIT_OBIELF_EMBED_ASSETS
#include "fruit_assets_embed.h"
#endif

static SDL_Surface *load_surface(
    fruit_resources_t *resources,
    const char *logical_name)
{
#if FRUIT_OBIELF_EMBED_ASSETS
    obielf_resource_view_t view;
    SDL_RWops *stream;

    if (obielf_resource_pack_find(
            &resources->pack, logical_name, &view) != OBIELF_OK ||
        view.size > (size_t)INT_MAX) {
        return NULL;
    }
    stream = SDL_RWFromConstMem(view.data, (int)view.size);
    if (stream == NULL) {
        return NULL;
    }
    return IMG_Load_RW(stream, 1);
#else
    char path[512];
    const int written = snprintf(
        path, sizeof(path), "%s/%s", FRUIT_ASSET_ROOT, logical_name);
    (void)resources;
    if (written < 0 || (size_t)written >= sizeof(path)) {
        return NULL;
    }
    return IMG_Load(path);
#endif
}

static SDL_Texture *load_texture(
    fruit_resources_t *resources,
    const char *logical_name)
{
    SDL_Surface *surface = load_surface(resources, logical_name);
    SDL_Texture *texture;

    if (surface == NULL) {
        SDL_Log("asset %s failed: %s", logical_name, IMG_GetError());
        return NULL;
    }
    texture = SDL_CreateTextureFromSurface(resources->renderer, surface);
    SDL_FreeSurface(surface);
    if (texture != NULL) {
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    }
    return texture;
}

static int load_fruit_set(
    fruit_resources_t *resources,
    fruit_kind_t kind)
{
    char name[96];
    fruit_sprite_set_t *set = &resources->fruit[kind];
    const char *base = fruit_kind_name(kind);

    if (snprintf(name, sizeof(name), "%s_small.png", base) < 0) {
        return 0;
    }
    set->whole = load_texture(resources, name);
    if (snprintf(name, sizeof(name), "%s_half_1_small.png", base) < 0) {
        return 0;
    }
    set->half1 = load_texture(resources, name);
    if (snprintf(name, sizeof(name), "%s_half_2_small.png", base) < 0) {
        return 0;
    }
    set->half2 = load_texture(resources, name);
    return set->whole != NULL && set->half1 != NULL && set->half2 != NULL;
}

int fruit_resources_init(
    fruit_resources_t *resources,
    SDL_Renderer *renderer,
    SDL_Window *window)
{
    fruit_kind_t kind;
    SDL_Surface *icon;

    if (resources == NULL || renderer == NULL || window == NULL) {
        return 0;
    }
    memset(resources, 0, sizeof(*resources));
    resources->renderer = renderer;

#if FRUIT_OBIELF_EMBED_ASSETS
    {
        const obielf_status_t status = obielf_resource_pack_open_memory(
            &resources->pack,
            fruit_assets_obielf,
            fruit_assets_obielf_size);
        if (status != OBIELF_OK ||
            obielf_resource_pack_verify(&resources->pack) != OBIELF_OK) {
            SDL_Log("embedded OBIELF pack failed: %s",
                    obielf_status_string(status));
            return 0;
        }
        resources->embedded = 1;
    }
#endif

    resources->background = load_texture(resources, "background.png");
    resources->bomb = load_texture(resources, "bomb_small.png");
    resources->explosion = load_texture(resources, "explosion_small.png");
    resources->splash_red =
        load_texture(resources, "splash_red_small.png");
    resources->splash_orange =
        load_texture(resources, "splash_orange_small.png");
    resources->splash_yellow =
        load_texture(resources, "splash_yellow_small.png");
    resources->splash_transparent =
        load_texture(resources, "splash_transparent_small.png");

    for (kind = FRUIT_APPLE; kind < FRUIT_BOMB;
         kind = (fruit_kind_t)((int)kind + 1)) {
        if (!load_fruit_set(resources, kind)) {
            fruit_resources_destroy(resources);
            return 0;
        }
    }
    if (resources->background == NULL || resources->bomb == NULL ||
        resources->explosion == NULL || resources->splash_red == NULL ||
        resources->splash_orange == NULL ||
        resources->splash_yellow == NULL ||
        resources->splash_transparent == NULL) {
        fruit_resources_destroy(resources);
        return 0;
    }

#if FRUIT_OBIELF_EMBED_ASSETS
    icon = load_surface(resources, FAVICON_RESOURCE);
#else
    icon = IMG_Load(FAVICON);
#endif
    if (icon != NULL) {
        SDL_SetWindowIcon(window, icon);
        SDL_FreeSurface(icon);
    } else {
        SDL_Log("window icon unavailable; direct path is %s", FAVICON);
    }
    return 1;
}

void fruit_resources_destroy(fruit_resources_t *resources)
{
    fruit_kind_t kind;

    if (resources == NULL) {
        return;
    }
    for (kind = FRUIT_APPLE; kind < FRUIT_BOMB;
         kind = (fruit_kind_t)((int)kind + 1)) {
        SDL_DestroyTexture(resources->fruit[kind].whole);
        SDL_DestroyTexture(resources->fruit[kind].half1);
        SDL_DestroyTexture(resources->fruit[kind].half2);
    }
    SDL_DestroyTexture(resources->background);
    SDL_DestroyTexture(resources->bomb);
    SDL_DestroyTexture(resources->explosion);
    SDL_DestroyTexture(resources->splash_red);
    SDL_DestroyTexture(resources->splash_orange);
    SDL_DestroyTexture(resources->splash_yellow);
    SDL_DestroyTexture(resources->splash_transparent);
    obielf_resource_pack_close(&resources->pack);
    memset(resources, 0, sizeof(*resources));
}

int fruit_resources_verify(const fruit_resources_t *resources)
{
    fruit_kind_t kind;

    if (resources == NULL || resources->background == NULL ||
        resources->bomb == NULL || resources->explosion == NULL) {
        return 0;
    }
    for (kind = FRUIT_APPLE; kind < FRUIT_BOMB;
         kind = (fruit_kind_t)((int)kind + 1)) {
        if (resources->fruit[kind].whole == NULL ||
            resources->fruit[kind].half1 == NULL ||
            resources->fruit[kind].half2 == NULL) {
            return 0;
        }
    }
    return !resources->embedded ||
           obielf_resource_pack_verify(&resources->pack) == OBIELF_OK;
}

const char *fruit_resources_mode(const fruit_resources_t *resources)
{
    return resources != NULL && resources->embedded
               ? "embedded OBIELF linkable pack"
               : "direct asset paths";
}
