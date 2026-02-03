#!/bin/sh
set -e

ROOT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"

rm -rf "$ROOT_DIR/ubsan-build"
mkdir -p "$ROOT_DIR/ubsan-build"
cd "$ROOT_DIR/ubsan-build"

JOBS="$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)"

echo "== configuring (UBSan) =="
CFLAGS="-O1 -g3 -fno-omit-frame-pointer -fsanitize=undefined -fno-sanitize-recover=undefined"
LDFLAGS="-fsanitize=undefined"

cmake "$ROOT_DIR" \
  -G "Unix Makefiles" \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_C_FLAGS="$CFLAGS" \
  -DCMAKE_EXE_LINKER_FLAGS="$LDFLAGS" \
  -DHACKDIR_OVERRIDE=hackdir

echo "== building =="
make -j"$JOBS"

echo "UBSan build ready in ubsan-build/"
