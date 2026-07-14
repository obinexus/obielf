#!/usr/bin/env sh
set -eu

mode="${1:-embedded}"
case "$mode" in
    embedded) embed=ON ;;
    direct) embed=OFF ;;
    *)
        echo "usage: $0 [embedded|direct]" >&2
        exit 2
        ;;
esac

root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cmake -S "$root" -B "$root/build" \
    -DFRUIT_OBIELF_EMBED_ASSETS="$embed" \
    -DCMAKE_BUILD_TYPE=Debug
cmake --build "$root/build" --parallel
ctest --test-dir "$root/build" --output-on-failure

