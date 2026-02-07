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
SOURCE_TAR="protoHack-${VERSION}-source.tar.gz"

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
{
  printf "# protoHack %s\n\n" "$VERSION"
  printf "## Downloads\n\n"
  printf -- "- **Linux Static Binary**: %s\n" "$BINARY_TAR"
  printf "  - Statically linked, runs on x86_64 Linux\n"
  printf "  - No dependencies required\n\n"
  printf -- "- **Source Code**: %s\n" "$SOURCE_TAR"
  printf "  - Full source for building on any platform\n"
  printf "  - Requires CMake, C compiler, and ncurses\n\n"
  printf "## Verification\n\n"
  printf "Verify downloads with SHA256 checksums:\n\n"
  printf '%s\n' '```bash'
  printf '%s\n' "sha256sum -c SHA256SUMS"
  printf '%s\n\n' '```'
  printf '%s\n\n' "## What's New"
  printf '%s\n\n' "${CHANGELOG_CONTENT:-See docs/CHANGELOG.md}"
  printf '%s\n\n' "## Building from Source"
  printf '%s\n' '```bash'
  printf 'tar xzf %s\n' "$SOURCE_TAR"
  printf 'cd protoHack-%s\n' "$VERSION"
  printf '%s\n' "cmake -B build -DCMAKE_BUILD_TYPE=Release"
  printf '%s\n' "cmake --build build"
  printf '%s\n' "./build/hack"
  printf '%s\n' '```'
} > "$RELEASE_DIR/RELEASE_NOTES.md"

printf "%b\n" "${GREEN}=== Release Assets Created Successfully ===${NC}"
printf "%b\n" "Release directory: ${BLUE}${RELEASE_DIR}/${NC}"
printf "%b\n" "\nAssets ready for GitHub release:"
printf "%b\n" "  • ${BINARY_TAR}"
printf "%b\n" "  • ${SOURCE_TAR}"
printf "%b\n" "  • SHA256SUMS"
printf "%b\n" "  • RELEASE_NOTES.md"

printf "%b\n" "\nTo create GitHub release:"
printf "%b\n" "  ${YELLOW}gh release create ${VERSION} ${RELEASE_DIR}/* \\\n  --title \"protoHack ${VERSION}\" \\\n  --notes-file ${RELEASE_DIR}/RELEASE_NOTES.md${NC}"
