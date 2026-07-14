#ifndef FRUIT_GAME_H
#define FRUIT_GAME_H

#include "fruit/config.h"

#include <stdint.h>

typedef enum fruit_kind {
    FRUIT_APPLE = 0,
    FRUIT_BANANA,
    FRUIT_COCONUT,
    FRUIT_ORANGE,
    FRUIT_PINEAPPLE,
    FRUIT_WATERMELON,
    FRUIT_BOMB,
    FRUIT_KIND_COUNT
} fruit_kind_t;

typedef enum fruit_entity_state {
    FRUIT_ENTITY_WHOLE = 0,
    FRUIT_ENTITY_SLICED,
    FRUIT_ENTITY_EXPLODING
} fruit_entity_state_t;

typedef enum fruit_game_phase {
    FRUIT_GAME_READY = 0,
    FRUIT_GAME_PLAYING,
    FRUIT_GAME_OVER
} fruit_game_phase_t;

typedef struct fruit_entity {
    int active;
    fruit_kind_t kind;
    fruit_entity_state_t state;
    float x;
    float y;
    float vx;
    float vy;
    float angle;
    float angular_velocity;
    float half1_x;
    float half1_y;
    float half1_vx;
    float half1_vy;
    float half1_angle;
    float half2_x;
    float half2_y;
    float half2_vx;
    float half2_vy;
    float half2_angle;
    float effect_time;
} fruit_entity_t;

typedef struct fruit_game {
    fruit_entity_t entities[FRUIT_MAX_ENTITIES];
    fruit_game_phase_t phase;
    uint32_t random_state;
    int score;
    int lives;
    float spawn_timer;
    float elapsed_time;
} fruit_game_t;

void fruit_game_init(fruit_game_t *game, uint32_t seed);
void fruit_game_start(fruit_game_t *game);
void fruit_game_update(fruit_game_t *game, float delta_seconds);

int fruit_game_slice(
    fruit_game_t *game,
    float start_x,
    float start_y,
    float end_x,
    float end_y);

int fruit_segment_hits_circle(
    float start_x,
    float start_y,
    float end_x,
    float end_y,
    float center_x,
    float center_y,
    float radius);

const char *fruit_kind_name(fruit_kind_t kind);

#endif

