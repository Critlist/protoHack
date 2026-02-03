#!/bin/sh
set -e

ROOT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"

rm -rf "$ROOT_DIR/sanitize-build"
mkdir -p "$ROOT_DIR/sanitize-build"
cd "$ROOT_DIR/sanitize-build"

JOBS="$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)"

echo "== configuring (ASan/UBSan) =="
CFLAGS="-O1 -g3 -fno-omit-frame-pointer -fsanitize=address,undefined"
LDFLAGS="-fsanitize=address,undefined"

cmake "$ROOT_DIR" \
  -G "Unix Makefiles" \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_C_FLAGS="$CFLAGS" \
  -DCMAKE_EXE_LINKER_FLAGS="$LDFLAGS" \
  -DHACKDIR_OVERRIDE=hackdir

echo "== building =="
make -j"$JOBS"

echo "Sanitized build ready in sanitize-build/"
