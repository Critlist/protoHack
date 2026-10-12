# protoHack dev tools

## Sanitizer and soak harness

Ported from restoHack's `dev/` harness, with the same lanes, options and logs.
It needs [uv](https://docs.astral.sh/uv/), which provides `pexpect` and `pyte`.
Run everything from `dev/` through `uv run`:

```bash
cd dev
uv run ./run-all.py                              # all lanes, 50 runs x 40 steps
uv run ./run-asan-ubsan.py --runs 5 --steps 300 --enhanced
uv run ./run-all.py --hardened --test-only       # one lane, reuse the build
```

The four lanes are ASan, UBSan, ASan+UBSan and hardened. The hardened lane
uses `-O2 -fstack-protector-strong -D_FORTIFY_SOURCE=3` and full RELRO, but not
PIE: `CMakeLists.txt` builds protoHack `-fno-pie` on purpose. Each lane builds
into `build-<lane>/` and adds `-DHACK_TEST_SEED`, so `HACK_SEED` fixes the
dungeon. Release builds don't read it.

A run fails on any sanitizer report (on the terminal or in a `log_path`
file, LeakSanitizer included), a signal, a bad exit status, a hang (no
output for 20 s while keys are sent), a save or hangup the game can't
restore, or a "TERMINAL TOO SMALL" screen left up after the size is restored.

### What a run does

`lib/game_runner.py` reads the screen with pyte. Most steps walk toward a
visible `>`, or toward the nearest unexplored edge, using BFS on the map, so
runs get below level 1. The rest of the steps are random moves and keys
aimed at the S1 fixes:

- ^R with no previous message
- `o` (options) and `c` (call) with long, comma and `%` text
- `--enhanced` adds save (`S`) or hangup followed by a restore, plus terminal
  resizes below 80x24

When a game ends by death or escape with steps left, the runner checks it
and starts a fresh game in the same run. Each run's line shows the deepest
level and the targeted actions it took.

### Soak

```bash
uv run ./run-all.py --runs 100 --steps 1000 --enhanced --seed-offset $(date +%s)
```

Change `--seed-offset` for each soak, or every soak replays the same
dungeons. Expect roughly 0.1-0.2 s per step on sanitizer builds. With 4
lanes, `--runs 100 --steps 1000` is an overnight job. Logs go to
`dev/test-logs/<lane>-<stamp>/`.

### Replaying a failure

Failure lines print the seed and offset, for example
`❌ asan-ubsan 007 FAILED (...) seed=... seed_offset=1760000000`. To re-run
that one run with the same seed and keys, pass the same lane, offset, steps
and mode:

```bash
uv run ./run-asan-ubsan.py --test-only --only-run 7 --seed-offset 1760000000 --steps 1000 --enhanced --debug
```

Replays follow the same keys as long as the game's output timing is
similar. A heavily loaded machine can make a key land before a prompt
appears.

## Other scripts

`sanitize-build.sh`, `ubsan-build.sh` and `valgrind-build.sh` make one-off
instrumented builds. `static-build.sh`, `package-tarballs.sh` and
`prepare-release.sh` are for releases.
