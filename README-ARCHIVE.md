# protoHack (Archive Readme)

This archive contains the original 1982 source alongside the restored sources
needed to build on a modern system.

## Contents

- `original/` — unmodified 1982 source
- `src/` — restored/ported sources
- `CMakeLists.txt` — build system
- `docs/` — project notes

## Build (Linux)

Requires: `cmake`, `make` or `ninja`, `ncurses`, and `crypt` (libxcrypt on many distros).

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Outputs:

- `build/hack-root`
- `build/mklev-root`
- `build/hackdir/` (runtime data files)

Run:

```sh
cd build
./hack-root
```

## Notes

- This is a two-binary design: `hack-root` execs `mklev-root`.
- `original/` is never modified; all changes live under `src/`.

## License

This source code was created primarily by Jay Fenlason, with additional
contributions from Kenny Woodland, Mike Thome, and Jon Payne. It is being
shared here under a CC-BY-NC-SA 4.0 license, as that is the closest modern
license to the original distribution license.
