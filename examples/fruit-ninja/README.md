# Fruit Ninja with OBIELF

A native C11 Fruit Ninja-style game using SDL2 and an OBIELF resource pack.
The existing browser prototype remains under `www/`; the native implementation
lives in `src/`.

## Gameplay

- Drag the mouse, stylus, or touch pointer through fruit.
- Each fruit scores one point and separates into two animated halves.
- Missing fruit costs one of three lives.
- Slicing a bomb ends the round.
- Press `Space` to start, `R` to restart, and `Escape` to quit.

## Windows build

Prerequisites already used by this repository:

- CMake
- MSYS2 MinGW64 GCC
- SDL2
- SDL2_image

```powershell
cd fruit-ninja-with-obielf
.\scripts\build.ps1
.\scripts\run.ps1
```

The default build embeds all required PNGs into the executable through
`libfruit_assets_obielf.a`.

To exercise ordinary filesystem paths instead:

```powershell
.\scripts\build.ps1 -Mode Direct
.\scripts\run.ps1 -Mode Direct
```

## Portable CMake build

On a system where SDL2 and SDL2_image provide CMake packages:

```bash
cmake -S . -B build -DFRUIT_OBIELF_EMBED_ASSETS=ON
cmake --build build
ctest --test-dir build --output-on-failure
./build/fruit-ninja-obielf
```

## Resource pipeline

The build creates:

- `generated/fruit_assets.obielf`: standalone `OBIRPACK` v1 file;
- `generated/fruit_assets_embed.c`: identical bytes as a C symbol;
- `libfruit_assets_obielf.a`: static linkable resource library;
- `obielf-pack`: pack creation and inspection CLI.

Inspect the generated archive:

```powershell
.\build-embedded\obielf-pack.exe --inspect `
    .\build-embedded\generated\fruit_assets.obielf
```

The format is documented in
[`docs/resource_pack_format.md`](docs/resource_pack_format.md). The full
milestone and its boundaries are reviewed in
[`PROJECT_SCOPE.md`](PROJECT_SCOPE.md).

## Automated runtime checks

```powershell
$env:Path = "C:\msys64\mingw64\bin;$env:Path"
.\build-embedded\fruit-ninja-obielf.exe --verify-assets
$env:SDL_VIDEODRIVER = "dummy"
.\build-embedded\fruit-ninja-obielf.exe `
    --smoke-test 180 --screenshot fruit-smoke.bmp
```
