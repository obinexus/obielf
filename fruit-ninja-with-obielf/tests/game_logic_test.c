#include "fruit/game.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int require(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        return 0;
    }
    return 1;
}

int main(void)
{
    fruit_game_t game;
    fruit_entity_t *entity;
    int passed = 1;

    passed &= require(
        fruit_segment_hits_circle(
            0.0f, 50.0f, 100.0f, 50.0f, 50.0f, 50.0f, 10.0f),
        "crossing segment hits circle");
    passed &= require(
        !fruit_segment_hits_circle(
            0.0f, 0.0f, 100.0f, 0.0f, 50.0f, 50.0f, 10.0f),
        "distant segment misses circle");
    passed &= require(
        !fruit_segment_hits_circle(
            50.0f, 50.0f, 50.0f, 50.0f, 50.0f, 50.0f, 10.0f),
        "zero-length segment is rejected");

    fruit_game_init(&game, 1u);
    passed &= require(
        game.phase == FRUIT_GAME_READY, "game initializes ready");
    fruit_game_start(&game);
    passed &= require(
        game.phase == FRUIT_GAME_PLAYING, "game starts");
    passed &= require(
        game.lives == FRUIT_STARTING_LIVES, "starting lives set");

    entity = &game.entities[0];
    memset(entity, 0, sizeof(*entity));
    entity->active = 1;
    entity->kind = FRUIT_APPLE;
    entity->state = FRUIT_ENTITY_WHOLE;
    entity->x = 100.0f;
    entity->y = 100.0f;
    passed &= require(
        fruit_game_slice(&game, 40.0f, 100.0f, 160.0f, 100.0f) == 1,
        "fruit can be sliced");
    passed &= require(game.score == 1, "slicing fruit increments score");
    passed &= require(
        entity->state == FRUIT_ENTITY_SLICED,
        "fruit transitions to sliced state");

    entity = &game.entities[1];
    memset(entity, 0, sizeof(*entity));
    entity->active = 1;
    entity->kind = FRUIT_BOMB;
    entity->state = FRUIT_ENTITY_WHOLE;
    entity->x = 300.0f;
    entity->y = 100.0f;
    passed &= require(
        fruit_game_slice(&game, 240.0f, 100.0f, 360.0f, 100.0f) == 1,
        "bomb collision is detected");
    passed &= require(
        game.phase == FRUIT_GAME_OVER, "bomb ends the game");

    fruit_game_start(&game);
    entity = &game.entities[0];
    memset(entity, 0, sizeof(*entity));
    entity->active = 1;
    entity->kind = FRUIT_BANANA;
    entity->state = FRUIT_ENTITY_WHOLE;
    entity->y = (float)FRUIT_LOGICAL_HEIGHT + 120.0f;
    fruit_game_update(&game, 1.0f / 60.0f);
    passed &= require(
        game.lives == FRUIT_STARTING_LIVES - 1,
        "missed fruit costs one life");

    if (passed) {
        puts("game logic conformance: PASS");
    }
    return passed ? EXIT_SUCCESS : EXIT_FAILURE;
}

