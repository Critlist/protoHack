# Changelog

All notable changes to protoHack will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

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
