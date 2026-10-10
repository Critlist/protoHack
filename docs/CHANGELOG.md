# Changelog

All notable changes to protoHack will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [Unreleased]

### Fixed

- Restored the 1982 `makefile`, `exp/makefile`, `exp/exp1/makefile` and empty `perm` to `original/`; `.gitignore` had silently dropped them when `hack/` was renamed. `original/` is again byte-identical to Sustainable-Games/fenlason-hack.
- README: corrected Dan Stormont's name, the static-binary install path (`~/Games/protohack`) and download step, the source file count (9), the chain-of-custody dates (2025, per Dan Stormont), and attributed the mklev split to Fenlason's own words.
- README: 2.8BSD → 2.9BSD (per Brian Harvey's account); Bresnick's narration no longer reads as a Fenlason quote.
- TIMELINE: `READ_Me` → `READ_ME`; the 82-1 tape entry no longer asserts Harvey was the submitter (*;login:* doesn't say); licensing intro no longer calls CC-BY-NC-SA "BSD-type".

### Documentation

- COMPARISON.md audit (three source audits plus an adversarial review): fixed three repo line refs (:91-93), the "levels 1-4 identical in all variants" claim (Hack 1.0 changes leprechaun, nymph, killer bee), the Hack 1.0 monster count (62), the README-credits row, the step counts and ELBIB YLOH note; paired Hack 1.0 by letter; added umber hulk/demon damage rows, `mstole`, the full VU README list; tempered the NOWORM inference; added "Stat lines that survived" (48/56; displacer beast → large dog incl. attack code; reused slots into NetHack 3.6) and "Mechanics dated by the source" (bear traps, mklev merge in 1.0.2).
- README: displacer beast / zelomp wording; the archived tree is Jay's working directory that matches the tape packaging, not the tape itself.
- TIMELINE: Harvey's account of sending JOVE to USENIX (jonmacs/jove#34); Bresnick vs Craddock on Jay's school year.

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
