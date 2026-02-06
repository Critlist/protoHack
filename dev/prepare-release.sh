#!/bin/sh
set -euo pipefail

# Prepare all release assets for GitHub releases
# Creates: static binary, source tarball, and checksums

BLUE='\033[0;34m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

printf "%b\n" "${BLUE}=== protoHack Release Asset Generator ===${NC}"

PROJECT_ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"

# VERSION: arg > git tag > date
VERSION="${1:-$(git -C "$PROJECT_ROOT" describe --tags --abbrev=0 2>/dev/null || date +%Y%m%d)}"
RELEASE_DIR="$PROJECT_ROOT/release-${VERSION}"

printf "%b\n" "${YELLOW}Building release ${VERSION}...${NC}"

rm -rf "$RELEASE_DIR"
mkdir -p "$RELEASE_DIR"

# Build tarballs using existing helpers
"$PROJECT_ROOT/dev/package-tarballs.sh" "$VERSION"

BINARY_TAR="protoHack-${VERSION}-linux-x86_64-static.tar.gz"
SOURCE_TAR="protoHack-source-${VERSION}.tar.gz"

# Move artifacts into release directory
mv -f "$PROJECT_ROOT/$BINARY_TAR" "$RELEASE_DIR/"
mv -f "$PROJECT_ROOT/$SOURCE_TAR" "$RELEASE_DIR/"

printf "%b\n" "${BLUE}[1/2] Generating checksums...${NC}"
(
  cd "$RELEASE_DIR"
  sha256sum "$BINARY_TAR" "$SOURCE_TAR" > SHA256SUMS
  sha256sum "$BINARY_TAR" > "$BINARY_TAR.sha256"
  sha256sum "$SOURCE_TAR" > "$SOURCE_TAR.sha256"
)

# Extract changelog section if available
CHANGELOG_FILE="$PROJECT_ROOT/docs/CHANGELOG.md"
CHANGELOG_CONTENT=""

if [ -f "$CHANGELOG_FILE" ]; then
  CHANGELOG_VERSION="${VERSION#v}"
  DATE_VERSION=""
  if [ "${#CHANGELOG_VERSION}" -eq 8 ]; then
    DATE_VERSION="${CHANGELOG_VERSION%????}-${CHANGELOG_VERSION#????}"
    DATE_VERSION="${DATE_VERSION%??}-${DATE_VERSION#?????-}"
  fi
  CHANGELOG_CONTENT=$(awk -v v="$CHANGELOG_VERSION" -v d="$DATE_VERSION" '
    $0 ~ "^## "v"$" {found=1; next}
    d != "" && $0 ~ "^## "d"$" {found=1; next}
    found && /^## / {exit}
    found {print}
  ' "$CHANGELOG_FILE")
fi

printf "%b\n" "${BLUE}[2/2] Writing release notes template...${NC}"
cat > "$RELEASE_DIR/RELEASE_NOTES.md" << EOF
# protoHack ${VERSION}

## Downloads

- **Linux Static Binary**: ${BINARY_TAR}
  - Statically linked, runs on x86_64 Linux
  - No dependencies required

- **Source Code**: ${SOURCE_TAR}
  - Full source for building on any platform
  - Requires CMake, C compiler, and ncurses

## Verification

Verify downloads with SHA256 checksums:

```bash
sha256sum -c SHA256SUMS
```

## What's New

${CHANGELOG_CONTENT:-"- See docs/CHANGELOG.md"}

## Building from Source

```bash
tar xzf ${SOURCE_TAR}
cd protoHack-${VERSION}
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/hack
```
EOF

printf "%b\n" "${GREEN}=== Release Assets Created Successfully ===${NC}"
printf "%b\n" "Release directory: ${BLUE}${RELEASE_DIR}/${NC}"
printf "%b\n" "\nAssets ready for GitHub release:"
printf "%b\n" "  • ${BINARY_TAR}"
printf "%b\n" "  • ${SOURCE_TAR}"
printf "%b\n" "  • SHA256SUMS"
printf "%b\n" "  • RELEASE_NOTES.md"

printf "%b\n" "\nTo create GitHub release:"
printf "%b\n" "  ${YELLOW}gh release create ${VERSION} ${RELEASE_DIR}/* \\\n  --title \"protoHack ${VERSION}\" \\\n  --notes-file ${RELEASE_DIR}/RELEASE_NOTES.md${NC}"
