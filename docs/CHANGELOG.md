# Changelog

All notable changes to protoHack will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [Unreleased]

### Fixed

- Maze levels (the bottom of the dungeon): `makemaz()` overflowed its fixed `int stack[200]` on about 7% of seeds (stack smashing / SIGABRT, or silent corruption). The maze grid has 8×37 = 296 cells, each pushed at most once, so the stack is now sized `8*37+1`. Maze output is byte-identical for every seed that didn't overflow.
- Restored the 1982 `makefile`, `exp/makefile`, `exp/exp1/makefile` and empty `perm` to `original/`; `.gitignore` had silently dropped them when `hack/` was renamed. `original/` is again byte-identical to Sustainable-Games/fenlason-hack.
- README: corrected Dan Stormont's name, the static-binary install path (`~/Games/protohack`) and download step, the source file count (9), the chain-of-custody dates (2025, per Dan Stormont), and attributed the mklev split to Fenlason's own words.
- README: 2.8BSD → 2.9BSD (per Brian Harvey's account); Bresnick's narration no longer reads as a Fenlason quote.
- TIMELINE: `READ_Me` → `READ_ME`; the 82-1 tape entry no longer asserts Harvey was the submitter (*;login:* doesn't say); licensing intro no longer calls CC-BY-NC-SA "BSD-type".

## [0.1.2] - 2026-08-12

### Fixed

- Fixed an infinite `pru()` -> `newsym()` -> `pline()` -> `pru()` recursion (stack overflow) that could occur when taking stairs. The modern "track last displayed @" redraw-sync logic in `pru()` retained state from the previous level, causing it to redraw a foreign, often-invalid coordinate on the new level; a diagnostic message triggered by that invalid coordinate could then re-enter `pru()` before its tracking state was updated, recursing indefinitely. (#5)

## [0.1.1] - 2026-02-07

### Changed

- macOS compatibility: disable non-PIE compile/link flags on Apple platforms to avoid trace traps.
- Link `crypt` only when a separate `CRYPT_LIB` is found (supports platforms where `crypt()` is in libc).
- Increased lock/save path buffer sizes to support longer usernames.
- Increased directory buffer sizes to accommodate longer path components.

## [0.1.0] - 2026-02-06

### Added

- Linux CI workflow with GCC/Clang and Debug/Release matrix plus runtime smoke tests.
- Valgrind build helper script in `dev/valgrind-build.sh`.
- Quick Start and Troubleshooting sections in `README.md`.

### Changed

- Merged static and archive README content into `README.md` and removed the extra files.
- Source tarball packaging now includes `docs/` for README image assets.
- Static tarball packaging now uses the merged `README.md`.
- `hackdir` setup now uses configure-time CMake `file()` commands and clears stale `record/news/moves` paths.
- `--More--` prompts now tolerate Enter/EOF to avoid input wedges.
- README images now use inline HTML figures for consistent GitHub rendering.

### Removed

- `README-ARCHIVE.md` and `README-STATIC.md` (merged into `README.md`).
