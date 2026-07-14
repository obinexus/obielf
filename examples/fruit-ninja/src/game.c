#include "fruit/game.h"

#include <math.h>
#include <stddef.h>
#include <string.h>

#define FRUIT_GRAVITY 980.0f
#define FRUIT_RADIUS 40.0f
#define BOMB_RADIUS 43.0f

static uint32_t next_random(fruit_game_t *game)
{
    uint32_t value = game->random_state;
    value ^= value << 13u;
    value ^= value >> 17u;
    value ^= value << 5u;
    game->random_state = value == 0u ? UINT32_C(0x9e3779b9) : value;
    return game->random_state;
}

static float random_unit(fruit_game_t *game)
{
    return (float)(next_random(game) & UINT32_C(0x00ffffff)) /
           16777215.0f;
}

static float random_range(fruit_game_t *game, float minimum, float maximum)
{
    return minimum + ((maximum - minimum) * random_unit(game));
}

static fruit_entity_t *acquire_entity(fruit_game_t *game)
{
    size_t index;
    for (index = 0u; index < FRUIT_MAX_ENTITIES; ++index) {
        if (!game->entities[index].active) {
            return &game->entities[index];
        }
    }
    return NULL;
}

static void spawn_wave(fruit_game_t *game)
{
    const int count = 1 + (int)(next_random(game) % 3u);
    int wave_index;

    for (wave_index = 0; wave_index < count; ++wave_index) {
        fruit_entity_t *entity = acquire_entity(game);
        const int is_bomb = random_unit(game) < 0.12f;

        if (entity == NULL) {
            return;
        }
        memset(entity, 0, sizeof(*entity));
        entity->active = 1;
        entity->kind = is_bomb
                           ? FRUIT_BOMB
                           : (fruit_kind_t)(next_random(game) %
                                            (uint32_t)FRUIT_BOMB);
        entity->state = FRUIT_ENTITY_WHOLE;
        entity->x = random_range(game, 90.0f, 870.0f);
        entity->y = (float)FRUIT_LOGICAL_HEIGHT + 55.0f;
        entity->vx = random_range(game, -130.0f, 130.0f);
        entity->vy = random_range(game, -860.0f, -690.0f);
        entity->angle = random_range(game, 0.0f, 360.0f);
        entity->angular_velocity =
            random_range(game, -190.0f, 190.0f);
    }
}

void fruit_game_init(fruit_game_t *game, uint32_t seed)
{
    if (game == NULL) {
        return;
    }
    memset(game, 0, sizeof(*game));
    game->random_state = seed == 0u ? UINT32_C(0x6d2b79f5) : seed;
    game->phase = FRUIT_GAME_READY;
    game->lives = FRUIT_STARTING_LIVES;
}

void fruit_game_start(fruit_game_t *game)
{
    uint32_t random_state;

    if (game == NULL) {
        return;
    }
    random_state = game->random_state;
    memset(game, 0, sizeof(*game));
    game->random_state =
        random_state == 0u ? UINT32_C(0x6d2b79f5) : random_state;
    game->phase = FRUIT_GAME_PLAYING;
    game->lives = FRUIT_STARTING_LIVES;
    game->spawn_timer = 0.35f;
}

static void update_whole(
    fruit_game_t *game,
    fruit_entity_t *entity,
    float delta_seconds)
{
    entity->vy += FRUIT_GRAVITY * delta_seconds;
    entity->x += entity->vx * delta_seconds;
    entity->y += entity->vy * delta_seconds;
    entity->angle += entity->angular_velocity * delta_seconds;

    if (entity->y > (float)FRUIT_LOGICAL_HEIGHT + 110.0f) {
        entity->active = 0;
        if (entity->kind != FRUIT_BOMB && game->lives > 0) {
            --game->lives;
            if (game->lives == 0) {
                game->phase = FRUIT_GAME_OVER;
            }
        }
    }
}

static void update_sliced(
    fruit_entity_t *entity,
    float delta_seconds)
{
    entity->half1_vy += FRUIT_GRAVITY * delta_seconds;
    entity->half2_vy += FRUIT_GRAVITY * delta_seconds;
    entity->half1_x += entity->half1_vx * delta_seconds;
    entity->half1_y += entity->half1_vy * delta_seconds;
    entity->half2_x += entity->half2_vx * delta_seconds;
    entity->half2_y += entity->half2_vy * delta_seconds;
    entity->half1_angle -= 230.0f * delta_seconds;
    entity->half2_angle += 230.0f * delta_seconds;
    entity->effect_time -= delta_seconds;

    if (entity->half1_y > (float)FRUIT_LOGICAL_HEIGHT + 120.0f &&
        entity->half2_y > (float)FRUIT_LOGICAL_HEIGHT + 120.0f &&
        entity->effect_time <= 0.0f) {
        entity->active = 0;
    }
}

void fruit_game_update(fruit_game_t *game, float delta_seconds)
{
    size_t index;

    if (game == NULL || game->phase != FRUIT_GAME_PLAYING) {
        return;
    }
    if (delta_seconds < 0.0f) {
        delta_seconds = 0.0f;
    } else if (delta_seconds > 0.05f) {
        delta_seconds = 0.05f;
    }
    game->elapsed_time += delta_seconds;
    game->spawn_timer -= delta_seconds;

    for (index = 0u; index < FRUIT_MAX_ENTITIES; ++index) {
        fruit_entity_t *entity = &game->entities[index];
        if (!entity->active) {
            continue;
        }
        if (entity->state == FRUIT_ENTITY_WHOLE) {
            update_whole(game, entity, delta_seconds);
        } else if (entity->state == FRUIT_ENTITY_SLICED) {
            update_sliced(entity, delta_seconds);
        } else {
            entity->effect_time -= delta_seconds;
            if (entity->effect_time <= 0.0f) {
                entity->active = 0;
            }
        }
    }

    if (game->phase == FRUIT_GAME_PLAYING && game->spawn_timer <= 0.0f) {
        spawn_wave(game);
        game->spawn_timer = random_range(game, 0.72f, 1.18f);
    }
}

int fruit_segment_hits_circle(
    float start_x,
    float start_y,
    float end_x,
    float end_y,
    float center_x,
    float center_y,
    float radius)
{
    const float dx = end_x - start_x;
    const float dy = end_y - start_y;
    const float length_squared = (dx * dx) + (dy * dy);
    float parameter;
    float nearest_x;
    float nearest_y;
    float distance_x;
    float distance_y;

    if (length_squared < 0.0001f) {
        return 0;
    }
    parameter =
        (((center_x - start_x) * dx) + ((center_y - start_y) * dy)) /
        length_squared;
    if (parameter < 0.0f) {
        parameter = 0.0f;
    } else if (parameter > 1.0f) {
        parameter = 1.0f;
    }
    nearest_x = start_x + (parameter * dx);
    nearest_y = start_y + (parameter * dy);
    distance_x = center_x - nearest_x;
    distance_y = center_y - nearest_y;
    return ((distance_x * distance_x) + (distance_y * distance_y)) <=
           (radius * radius);
}

static void slice_fruit(fruit_game_t *game, fruit_entity_t *entity)
{
    entity->state = FRUIT_ENTITY_SLICED;
    entity->half1_x = entity->x;
    entity->half1_y = entity->y;
    entity->half1_vx = entity->vx - 210.0f;
    entity->half1_vy = entity->vy * 0.62f;
    entity->half1_angle = entity->angle;
    entity->half2_x = entity->x;
    entity->half2_y = entity->y;
    entity->half2_vx = entity->vx + 210.0f;
    entity->half2_vy = entity->vy * 0.62f;
    entity->half2_angle = entity->angle;
    entity->effect_time = 0.5f;
    ++game->score;
}

int fruit_game_slice(
    fruit_game_t *game,
    float start_x,
    float start_y,
    float end_x,
    float end_y)
{
    size_t index;
    int sliced = 0;

    if (game == NULL || game->phase != FRUIT_GAME_PLAYING) {
        return 0;
    }
    for (index = 0u; index < FRUIT_MAX_ENTITIES; ++index) {
        fruit_entity_t *entity = &game->entities[index];
        const float radius =
            entity->kind == FRUIT_BOMB ? BOMB_RADIUS : FRUIT_RADIUS;

        if (!entity->active || entity->state != FRUIT_ENTITY_WHOLE ||
            !fruit_segment_hits_circle(
                start_x,
                start_y,
                end_x,
                end_y,
                entity->x,
                entity->y,
                radius)) {
            continue;
        }
        ++sliced;
        if (entity->kind == FRUIT_BOMB) {
            entity->state = FRUIT_ENTITY_EXPLODING;
            entity->effect_time = 1.0f;
            game->phase = FRUIT_GAME_OVER;
            break;
        }
        slice_fruit(game, entity);
    }
    return sliced;
}

const char *fruit_kind_name(fruit_kind_t kind)
{
    static const char *const names[FRUIT_KIND_COUNT] = {
        "apple",
        "banana",
        "coconut",
        "orange",
        "pineapple",
        "watermelon",
        "bomb"};

    if ((unsigned int)kind >= (unsigned int)FRUIT_KIND_COUNT) {
        return "unknown";
    }
    return names[kind];
}

