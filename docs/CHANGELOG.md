# Changelog

All notable changes to protoHack will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [Unreleased]

### Added

- `dev/` sanitizer and soak harness, ported from restoHack: ASan, UBSan, ASan+UBSan and hardened lanes (`dev/run-all.py`), seeded and replayable (`--seed-offset`, `--only-run`). protoHack's runner explores toward the stairs and aims at the S1 fixes (^R, option and naming input, save/hangup and restore, resize). Test builds (`-DHACK_TEST_SEED`) seed the RNG from `HACK_SEED`; release builds don't.

### Fixed

- Maze levels (the bottom of the dungeon): `makemaz()` overflowed its fixed `int stack[200]` on about 7% of seeds (stack smashing / SIGABRT, or silent corruption). The maze grid has 8×37 = 296 cells, each pushed at most once, so the stack is now sized `8*37+1`. Maze output is byte-identical for every seed that didn't overflow.
- Linux ARM64 (and any unsigned-`char` ABI): `mklev` crashed with SIGFPE/SIGSEGV on every normal level because Jay's `croom->hx = -1` end-of-rooms sentinel became 255. The build now passes `-fsigned-char`, matching the PDP-11's signed `char`. Same bug class as restoHack #15.
- After the "TERMINAL TOO SMALL" screen, the map wasn't redrawn if the terminal went back to the same size it had before the shrink (`check_resize()` only redrew on a size change), so the warning stayed on screen. It now always redraws after that screen.
- Fixed a segfault on Ctrl-R (repeat last message) when there was no previous message: the 1982 `pline(0)` path fell through to `index(NULL,'%')`. Both the normal and SMALL builds now print "No message.". (#11)
- A comma in the player name (set via `name=` in options) split the high-score record line and wiped the record file. Commas in names are now replaced with `_`, and the record reader bounds its fields to their buffers. (#11)
- Fixed a crash at death from a wraith's level drain: the 1982 code never set `killer` there, so `done()` hit `strcpy(..., NULL)`. `done()` now also defaults `killer` for any other death path that misses it. (#12)
- Fixed a use-after-free when a freezing sphere or a yellow light explodes: `dochug()` kept using the monster after `delmon()` freed it, and `movemon()` followed its freed `nmon` link. (#12)
- `getlin()` no longer overflows its caller's 100-byte buffer: input stops at 99 characters, and EOF (a hung-up tty) takes the save-and-exit hangup path instead of being stored forever. (#12)
- `pline()` formatted into a 60-byte buffer with `sprintf`; it now uses a 256-byte buffer and `snprintf`. (#12)
- Object names containing `%` (user-named items, or the player's name) were passed to `pline()` as the format string; every `pline(buf)` call now uses `pline("%s",buf)`. (#12)
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
