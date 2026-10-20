# protoHack in the browser

This port uses the `unix-tui-to-wasm` template in the `wasm/utw` submodule.
It provides an 80×24 xterm.js terminal, Asyncify terminal input, and
IndexedDB persistence for savegames.

## Build and run

Install and activate Emscripten **6.0.13** (pinned by the template), with
`emcc` and `emcmake` on PATH. CMake, a C compiler, Node.js, curl, and
`sha256sum` are also required.

Install the pinned SDK once:

```sh
git clone https://github.com/emscripten-core/emsdk.git ~/emsdk
~/emsdk/emsdk install 6.0.13
~/emsdk/emsdk activate 6.0.13
source ~/emsdk/emsdk_env.sh
```

Run `source ~/emsdk/emsdk_env.sh` in each new shell before building; it adds
`emcc`, `emcmake`, and the SDK's bundled Node.js to PATH. If emsdk is already
installed, use its existing directory and skip the clone step.

From the protoHack repository root:

```sh
git submodule update --init wasm/utw
./wasm/build-wasm.sh
python3 -m http.server 8000 --directory build/wasm
# Open http://localhost:8000/
```

The wrapper fetches checksum-verified xterm.js/xterm-pty assets and stages
a self-contained bundle in `build/wasm/`, with compiler output in
`build/wasm-build/`. Once built, no external assets or special HTTP headers
are needed. Serve over HTTP, not `file://`. `WEB_DIR` and `OBJ_DIR` override
the output locations; `ALLOW_UNTESTED_EMSDK=1` permits another SDK version.

Use vi keys (`hjklyubn`) to move, `?` for paginated help, and `S` to save 
and exit. Reload to resume. The stable browser identity is `player`; saves 
and scores persist in the game-specific IDBFS mount `/protohack-save`. 
As with the template, use one active game per browser origin to avoid 
competing writes. Native saves are *not* compatible with the Wasm level format.

## How the port stays small

- `protohack.cmake` configures the template and stages both Wasm modules.
- `browser.c` replaces level-generation process launches and external paging.
- `mklev.c` is unchanged: each level runs in a fresh `mklev.js`/`mklev.wasm`
  instance. Its level file is copied into the game's filesystem. This
  preserves its separate globals, heap, and binary level format.
- `generator.c` replaces generator re-exec with exit status 75; the browser
  retries with a fresh module and random seed (up to 100 attempts).
- `identity.c` provides a stable login, time-based process seed, and private
  filesystem lock shim. `pre-protohack.js` connects the existing save/score
  paths to persistent storage.
- Existing C edits are limited to browser branches for news, help, shell,
  and level launching, plus display prototypes and real variadic `pline`
  and `panic` functions. Wasm requires matching call signatures; the old
  implicit declarations otherwise link to trap stubs.

The native build still uses curses, real Unix processes, and native locks:

```sh
cmake -S . -B build/native -DCMAKE_BUILD_TYPE=Release
cmake --build build/native
./build/native/hack
```
