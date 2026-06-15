#ifndef FRUIT_CONFIG_H
#define FRUIT_CONFIG_H

#ifndef FRUIT_ASSET_ROOT
#define FRUIT_ASSET_ROOT "./assets"
#endif

/*
 * Direct-path mode uses ordinary C string-literal concatenation. The same
 * logical resource is named FAVICON_RESOURCE inside the embedded OBIELF pack.
 */
#define FAVICON FRUIT_ASSET_ROOT "/apple_small.png"
#define FAVICON_RESOURCE "apple_small.png"

#define FRUIT_WINDOW_TITLE "Fruit Ninja with OBIELF"
#define FRUIT_LOGICAL_WIDTH 960
#define FRUIT_LOGICAL_HEIGHT 540
#define FRUIT_STARTING_LIVES 3
#define FRUIT_MAX_ENTITIES 32
#define FRUIT_MAX_TRAIL_POINTS 24

#endif
