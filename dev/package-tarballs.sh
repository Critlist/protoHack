#!/bin/sh
set -e

ROOT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
DATE_STAMP="$(date +%Y%m%d)"

# Build static binary tarball first.
"$ROOT_DIR/dev/static-build.sh"

# Source tarball for review/compilation.
TAR_SRC="protoHack-source-$DATE_STAMP.tar.gz"
tar --numeric-owner -czf "$ROOT_DIR/$TAR_SRC" \
  -C "$ROOT_DIR" \
  original \
  src \
  CMakeLists.txt \
  README.md

echo "Created $TAR_SRC in repo root."
