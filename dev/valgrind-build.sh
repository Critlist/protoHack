#!/bin/sh
set -e

ROOT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"

rm -rf "$ROOT_DIR/valgrind-build"
mkdir -p "$ROOT_DIR/valgrind-build"
cd "$ROOT_DIR/valgrind-build"

JOBS="$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)"

echo "== configuring (Valgrind) =="
CFLAGS="-O0 -g3 -fno-omit-frame-pointer"

cmake "$ROOT_DIR" \
  -G "Unix Makefiles" \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_C_FLAGS="$CFLAGS" \
  -DHACKDIR_OVERRIDE=hackdir

echo "== building =="
make -j"$JOBS"

echo "Valgrind build ready in valgrind-build/"
