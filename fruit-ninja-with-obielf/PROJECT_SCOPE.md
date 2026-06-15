# Project Scope Review

## Objective

Build a playable Fruit Ninja-style game in C using SDL2, while demonstrating
how OBIELF resources can exist both as a standalone file and as bytes linked
directly into an executable.

## Required deliverables

| Requirement | Implementation evidence |
|---|---|
| Native C game | `src/main.c`, `src/game.c` |
| Simple DirectMediaLayer | SDL2 renderer and SDL2_image PNG loading |
| Fruit slicing | Segment-circle collision, split sprites, score |
| Failure rules | Missed fruit consumes lives; bomb ends game |
| Pointer input | Mouse and SDL touch events |
| Direct resource paths | `include/fruit/config.h`, including `FAVICON` |
| OBIELF file artifact | Generated `fruit_assets.obielf` |
| Linkable embedding | Generated C symbol archived as `fruit_assets_obielf` |
| Integrity validation | Header, directory, and per-resource CRC-32 |
| Repeatable build | CMake and scripts in `scripts/` |
| Runtime evidence | Asset verification and headless smoke test |

## Resource modes

### Embedded linkable mode

This is the default. `obielf-pack` creates a byte-identical `.obielf` file and
C array. The C array is compiled into a static library, placed in the
`.obi.resources` section on GCC/Clang, and linked into the game. SDL_image reads
PNG data through `SDL_RWops` without requiring asset files at runtime.

### Direct-path mode

Configure with:

```powershell
.\scripts\build.ps1 -Mode Direct
```

Resources are loaded from `FRUIT_ASSET_ROOT`. The window icon demonstrates the
requested direct style:

```c
#define FRUIT_ASSET_ROOT "assets"
#define FAVICON FRUIT_ASSET_ROOT "/apple_small.png"
```

## Scope boundaries

- The `.obielf` resource pack is an application artifact governed by the
  OBIELF part-file rules. It is not yet a new ELF executable encoding.
- A NASM backend, linker plugin, loader ABI, and `.obi.*` ELF section type
  registry remain separate OBIELF work.
- The browser prototype in `www/` is preserved as reference material. Camera
  and MediaPipe hand tracking are not required for the native SDL milestone.
- SDL2_image is used because SDL2 itself does not decode PNG.

## Completion gates

The native milestone is complete only when both embedded and direct builds:

1. compile with warnings treated as errors;
2. pass resource format and gameplay unit tests;
3. load every required PNG;
4. complete a headless SDL smoke run;
5. produce a non-empty screenshot;
6. remain playable through mouse or touch input.

