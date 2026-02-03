#!/bin/sh
set -e

REAL_UID="$(id -u)"
REAL_GID="$(id -g)"
ROOT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"

# Clean previous build as root in container so host user does not need sudo.
docker run --rm \
  -v "$ROOT_DIR":/src \
  alpine:3.20 sh -eu -c "
    rm -rf /src/CMakeCache.txt /src/CMakeFiles /src/cmake_install.cmake /src/Makefile /src/static-build
    chown -R $REAL_UID:$REAL_GID /src/static-build 2>/dev/null || true
  "

docker run --rm --network=host \
  --dns 1.1.1.1 --dns 8.8.8.8 \
  -e ALPINE_MIRROR \
  -v "$ROOT_DIR":/src \
  -w /src \
  alpine:3.20 sh -eu -c '
    if [ -n "${ALPINE_MIRROR:-}" ]; then
        echo "${ALPINE_MIRROR}" > /etc/apk/repositories
    fi
    # Retry apk in case the host/network is temporarily unavailable.
    for i in 1 2 3 4 5; do
        if apk update >/dev/null 2>&1; then
            if apk add --no-cache \
              build-base cmake pkgconf \
              ncurses-dev ncurses-static; then
                break
            fi
        fi
        echo "apk failed (attempt $i). Retrying in 3s..."
        sleep 3
    done

    # Final check to fail loudly if packages still missing.
    apk add --no-cache \
      build-base cmake pkgconf \
      ncurses-dev ncurses-static

    rm -rf static-build
    mkdir static-build
    cd static-build

    JOBS="$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)"

    echo "== configuring =="

    FORM_A="$(ls /usr/lib/libformw.a /usr/lib/libform.a 2>/dev/null | head -n1 || true)"
    if [ -n "$FORM_A" ]; then
        CURSES_LIBS="/usr/lib/libncursesw.a;$FORM_A"
        CURSES_FORM_FLAG="-DCURSES_FORM_LIBRARY=$FORM_A"
    else
        CURSES_LIBS="/usr/lib/libncursesw.a"
        CURSES_FORM_FLAG="-DCURSES_FORM_LIBRARY="
    fi

    cmake .. \
      -G "Unix Makefiles" \
      -DCMAKE_BUILD_TYPE=Debug \
      -DBUILD_SHARED_LIBS=OFF \
      -DCMAKE_FIND_LIBRARY_SUFFIXES=".a" \
      -DCMAKE_EXE_LINKER_FLAGS="-static" \
      -DCMAKE_C_FLAGS="-D_STATIC_BUILD" \
      -DHACKDIR_OVERRIDE=hackdir \
      -DCURSES_LIBRARY=/usr/lib/libncursesw.a \
      -DCURSES_LIBRARIES="$CURSES_LIBS" \
      $CURSES_FORM_FLAG

    echo "== building =="
    make -j"$JOBS"

    chmod 775 hackdir
    install -d -m 2775 hackdir/save
    # Preserve original 1982-83 files for record/perm/news/moves
    cp -f /src/original/record hackdir/record
    cp -f /src/original/perm hackdir/perm
    cp -f /src/original/news hackdir/news
    cp -f /src/original/moves hackdir/moves
    chmod 664 hackdir/record hackdir/perm

    cat > run-hack.sh << "RUNEOF"
#!/bin/sh
chmod 664 hackdir/record hackdir/perm 2>/dev/null || true
chmod 2775 hackdir/save 2>/dev/null || true
exec ./hack-root "$@"
RUNEOF

    chmod +x run-hack.sh

    TAR=protoHack-static-$(date +%Y%m%d)-linux-x86_64.tar.gz
    tar --numeric-owner -czf "../$TAR" \
      hack-root mklev hackdir run-hack.sh /src/README.md

    echo "== fixing ownership =="
    chown -R '"$REAL_UID:$REAL_GID"' /src/static-build "/src/$TAR"

    echo "Created $TAR"
'
