#!/bin/sh
set -e

ROOT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
VERSION="${1:-$(date +%Y%m%d)}"

# Build static binary tarball first.
"$ROOT_DIR/dev/static-build.sh" "$VERSION"

# Source tarball for review/compilation.
TAR_SRC="protoHack-$VERSION-source.tar.gz"
tar --numeric-owner -czf "$ROOT_DIR/$TAR_SRC" \
  -C "$ROOT_DIR" \
  original \
  src \
  docs \
  CMakeLists.txt \
  README.md

echo "Created $TAR_SRC in repo root."
