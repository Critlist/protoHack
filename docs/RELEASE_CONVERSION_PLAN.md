# EXP1 Conversion Plan — Fenlason Hack "Release" Version

## Overview

Convert the 12 source files + 2 headers + hack.vars of Fenlason's exp1
(the version submitted to the USENIX distribution tapes) from K&R C
to compilable ANSI C on modern Linux/gcc.

**Source**: `original/exp/exp1/` (5,545 lines across 15 files)
**Target**: `src/release/` (new directory alongside existing `src/root/`)
**Shared**: `src/compat.h` (BSD→POSIX shim, reused)

The existing `src/root/` (hack-root, mklev-root) is untouched. The new
`src/release/` produces a single `hack-release` binary. Shell wrappers
select which version to launch.

**Documentation precision**: Only use the label "USENIX 82-1" if the tree
explicitly proves that designation. Otherwise refer to it as "USENIX tape
submission" or "exp1 submission."

---

## Critical Architectural Difference: Single Binary

Unlike hack-root (which exec's a separate `mklev` binary due to PDP-11
memory constraints), the release version compiles as a **single binary**.
`mklev()` is a function inside `hack.levl.c` called directly from
`main()`. The original makefile confirms:

```
ld -X -i -o hack /lib/crt0.o ../rnd.o *.o -ltermlib -lc
```

This means:

- No separate mklev-release binary needed
- All 12 .c files + rnd.c link into one executable: `hack-release`
- hackdir-release/ still needed for level files, record, news, moves

---

## Header Situation (Anomaly)

exp1 ships with **two** headers that serve different roles:

| Header | Defines | Purpose |
|--------|---------|---------|
| `hack.h` (301 lines) | `NONUM`, no SMALL/VTONL | Full version: includes, bwrite/mread/cm macros, all extern declarations |
| `hackfoo.h` (249 lines) | `SMALL`, `VTONL`, `V6` | Stripped-down: getx/gety, north/south/east/west macros, no externs |

**hackfoo.h is identical to the parent `exp/hackfoo.h`** — it appears to be
an older/alternative configuration header. Only `hack.h` is `#include`'d by
the source files.

**Decision**: Use `hack.h` as the canonical header. hackfoo.h is preserved
as a historical artifact but not included in the build. We define VTONL via
CMake (as with hack-root) to select ANSI escape code paths.

---

## hack.vars — The "Source Header" Pattern

`hack.vars` (368 lines) is `#include`'d by `hack.main.c` only. It contains
all global variable definitions:

- `levl[80][22]`, monster/object linked list heads
- Monster table `mon[8][7]` with full initialization
- All string constant arrays (weapon names, potion colors, scroll names)
- VTONL escape sequences (HO, CL, CE, BC, ND, UP as char arrays)
- Non-VTONL termcap variables (char pointers + xbuf[45])

This unusual pattern works because only hack.main.c includes it, making it
a single translation unit for all globals.

**Rule**: Keep `hack.vars` included only by `hack.main.c`. All other files
see globals via `extern` declarations in `hack.h`.

---

## Historical Artifacts Found

### bugs (3 lines)

Build log fragment — `cc` and `ld` commands. Not a bug list.

### bugs1 (179 lines)

**1982 lint output** — invaluable historical document. Captures every
warning V7 `lint` produced. Key findings:

- 84 "illegal pointer combination" / subtraction warnings
- `hack.mon.c line 38: lvalue required` (assignment bug)
- `getlev value is used, but none returned`
- `_mon defined but never used` (hack.vars:159)
- Multiple fread/fwrite casting inconsistencies

### record (215 bytes)

High score file template — copy into hackdir-release/.

### tags (2.9KB)

1982 ctags output — function-to-file mapping.

---

## File-by-File Conversion Breakdown

### Phase 1: Headers & Globals (Build Foundation)

#### 1A. `hack.h` → `src/release/hack.h`

**What it does**: Master header. Struct definitions (lev, permonst, monst,
stole, gen, obj, flag, you), all bit-packing macros, bwrite/mread, cl_end,
mfree, extern declarations, game constants.

**Conversion needed**:

- Remove `#include <sgtty.h>` (line 3) — replaced by compat.h termios
- Remove `char *index(),*getlogin(),*getenv();` (line 4) — compat.h provides
- Remove `int done1(),done2();` (line 5) — replace with proper prototypes
- Add `#include "../compat.h"` after struct definitions
- Fix `extern multi;` → `extern int multi;` (line 234, implicit int)
- Add function prototypes for all converted functions
- Preserve all macro definitions exactly (bit-packing, screen macros)
- Keep NONUM define (controls numeric keypad parsing)

**Anomaly — gminus parenthesization**:

- hack.h: `(obj->quanmin&0200)` (parenthesized — correct)
- hackfoo.h: `obj->quanmin&0200` (bare — operator precedence risk)
- Use hack.h version.

#### 1B. `hack.vars` → `src/release/hack.vars`

**What it does**: All global variable definitions. Monster table, string
constants, escape sequences.

**Conversion needed**:

- Remove `FILE *fopen();` (line 4) — provided by stdio.h via compat.h
- Fix implicit types where needed
- `char dirs[9]={-21,1,23,...}` — signed char issue on platforms where
  char is unsigned. Use a `typedef signed char schar;` and apply to the
  direction arrays (do not rely on default `char` signedness).
- Preserve all string constants exactly: "homonculous", "gelatenous",
  "aggrivate monster", "black onix"

---

### Phase 2: Core Utilities

#### 2A. `hack.c` → `src/release/hack.c` (370 lines, ~21 functions)

**What it does**: Core utility procedures. Parser, game-over, terminal
mode switching, string helpers, memory allocation, panic handler.

**Functions** (21 total):

| Function | Lines | Conversion |
|----------|-------|------------|
| `pow(num)` | 5-12 | **Rename → `hack_pow()`** (math.h conflict) |
| `parse()` | 13-47 | K&R→ANSI, fix implicit int returns |
| `done(st1)` | 48-143 | K&R→ANSI, signal handling, record file I/O |
| `done1()` | 144-153 | **Signal handler → `void done1(int signum)`** |
| `done2()` | 154-172 | **Signal handler → `void done2(int signum)`** |
| `nomul(nval)` | 173-186 | K&R→ANSI |
| `g_at(loc,ptr)` | 187-197 | K&R→ANSI, needs `struct gen *` return type |
| `cbout()` | 198-206 | **gtty/stty → tcgetattr/tcsetattr** |
| `cbin()` | 207-215 | **gtty/stty → tcgetattr/tcsetattr** |
| `setan(str,buf)` | 216-221 | K&R→ANSI, index→strchr via compat.h |
| `getlin(str)` | 222-247 | K&R→ANSI |
| `ndaminc()` | 248-261 | K&R→ANSI |
| `shufl(base,num)` | 262-275 | K&R→ANSI |
| `alloc(num)` | 276-283 | K&R→ANSI, explicit `char *` return |
| `getret()` | 284-291 | K&R→ANSI |
| `kludge(str,arg)` | 292-297 | K&R→ANSI |
| `k1(str,arg)` | 298-303 | K&R→ANSI |
| `panic(str,...)` | 304-313 | **VARARGS → stdarg.h** |
| `getxy(x,y,loc)` | 314-324 | K&R→ANSI |
| `getdir()` | 325-336 | K&R→ANSI |
| `near(loc1,loc2,range)` | 337-347 | K&R→ANSI |
| `set1(str)` | 348-370 | K&R→ANSI, #ifndef SMALL block |

**Critical conversions**:

1. `cbout()`/`cbin()`: `struct sgttyb`+`gtty()/stty()` → `struct termios`+
   `tcgetattr()/tcsetattr()`. Pattern: `src/root/hack.c`.
2. `panic()`: `/*VARARGS1*/` → `va_list`/`vprintf`. Pattern: `src/root/mklev.c`.
3. `pow()` → `hack_pow()`: math.h collision. Same as hack-root. Replace all
   callers up front to avoid mixed naming.
4. `done1()`/`done2()`: Signal handlers need `void(int)` signature.

**index() calls** (5 instances, lines 70/136/219/354/365): Handled by
compat.h `#define index strchr`.

#### 2B. `hack.screen.c` → `src/release/hack.screen.c` (393 lines, ~20 functions)

**What it does**: All display operations. Cursor movement, character
drawing, screen redraw, status bar, terminal init, message line.

**Functions** (20 total):

| Function | Lines | Conversion |
|----------|-------|------------|
| `curs(x,y)` | 5-27 | K&R→ANSI, implicit int params |
| `cm(x,y)` | 29-35 | #ifndef SMALL, termcap tgoto() |
| `atl(loc,ch)` | 37-43 | K&R→ANSI |
| `on(loc)` | 44-61 | K&R→ANSI, dirty rectangle tracking |
| `at(x,y,ch)` | 62-71 | K&R→ANSI |
| `docrt()` | 72-96 | K&R→ANSI |
| `pru()` | 97-102 | K&R→ANSI |
| `prl(loc)` | 103-114 | K&R→ANSI |
| `newsym(loc)` | 115-144 | K&R→ANSI, implicit int `tmp` |
| `nosee(loc)` | 145-163 | K&R→ANSI |
| `prustr()` | 164-171 | K&R→ANSI |
| `pmon(mon)` | 172-177 | K&R→ANSI, param shadows global `mon[]` |
| `nscr()` | 178-199 | K&R→ANSI |
| `nocm(x,y)` | 201-220 | K&R→ANSI |
| `bot()` | 221-297 | K&R→ANSI |
| `startup(nam)` | 299-325 | #ifndef VTONL, full termcap init |
| `cls()` | 327-332 | K&R→ANSI |
| `home()` | 333-342 | K&R→ANSI |
| `pline(line,...)` | 344-393 | **VARARGS → stdarg.h** |

**Critical conversions**:

1. `pline()`: Called from everywhere. Original signature has a bug — args
   a5-a8 are `char` not `char *` (line 345). Convert to proper
   `void pline(const char *line, ...)` with `va_list` and `vsnprintf()`.
   Keep all cursor/screen logic intact; only change the varargs handling.
2. `startup()`: Compiled out by VTONL define, but convert for completeness.
   Uses tgetent/tgetstr/tgetnum/tgetflag — needs ncurses termcap.

**extern xbuf[] mystery** (line 4): Defined in hack.vars:46 as part of
the non-VTONL path (`char *CE,*HO,...,xbuf[45];`).

---

### Phase 3: Entry Point & Level Generation

**Sequencing note**: Create `hack.lock.c` early (even stubbed) so
`hack.main.c` refactors don't get redone later when modern locking is
introduced.

#### 3A. `hack.main.c` → `src/release/hack.main.c` (239 lines)

**What it does**: `main()` function — initialization, save restoration,
game loop. `#include "hack.vars"` pulls in all globals.

**Conversion needed**:

- K&R→ANSI: `main()` → `int main(void)`
- Add `#include "../compat.h"`
- Signal handlers need proper types
- Add SIGTERM/SIGHUP save handlers (modern addition from hack-root)
- Add SIGWINCH resize handler (modern addition from hack-root)
- `index()` call (line 28) — compat.h
- `alloc()` returns `char *` assigned to struct pointers — needs casts
- `/*V7*/` struct assignments — valid ANSI C, keep as-is

**Key difference from hack-root**: No `execl()` to mklev. `mklev()` called
directly as a function (line 150). No `fork()`. Simpler conversion.

#### 3B. `hack.levl.c` → `src/release/hack.levl.c` (541 lines, ~15 functions)

**What it does**: Level generation, room creation, maze generation,
object/monster/trap placement. Equivalent of hack-root's separate `mklev`
binary, but compiled into the main executable.

**Functions** (15+):
`mklev()`, `mkobj()`, `comp()`, `mkpos()`, `dodoor()`, `newloc()`,
`makemaz()`, `move()`, `okay()`, `mktrap()`, `mkgold()`, `mmon()`,
`maker()`, `mkmim()`

**Critical**:

- `comp()` for `qsort()` needs `int comp(const void *a, const void *b)`
- `move()` and `dir` param shadow global — preserve original naming

**Anomaly**: "this is all Kenny's fault. He seems to have his x and y
reversed" (~line 343). Preserve comment and reversed coords.

**>>> MILESTONE: After Phases 1-3, should compile and generate levels <<<**

---

### Phase 4: Game Logic Files

#### 4A. `hack.files.c` (176 lines, 5 functions)

Level file save/load, lock files, staircase transitions.
`glo()`, `dodown()`, `doup()`, `savelev()`, `getlev()`

- K&R→ANSI, bwrite/mread macros unchanged
- Keep original flat binary save format
- `getlev` needs explicit return type and clear return value (1 success / 0 fail)

#### 4B. `hack.see.c` (143 lines, 4 functions)

Visibility system. Room lighting, line-of-sight.
`litroom()`, `unsee()`, `seeoff()`, `setsee()`

- K&R→ANSI, heavy pointer arithmetic with north/south/east/west macros
- Preserve "maximium" typo (lines 18, 46)

#### 4C. `hack.move.c` (316 lines, ~8 functions)

Player movement, direction parsing, teleportation.
`movecm()`, `domove()`, `tele()`, `prl1()`, helpers

- K&R→ANSI. No terminal I/O or signals — straightforward.

#### 4D. `hack.fight.c` (215 lines, ~6 functions)

Combat system. Hit/miss, damage, monster death.
`abon()`, `amon()`, `attmon()`, `killed()`, `hit()`, `miss()`

- K&R→ANSI, index() calls handled by compat.h

#### 4E. `hack.mon.c` (672 lines, ~20 functions)

Monster AI and behavior. Movement, combat, special abilities, polymorphing.
`delmon()`, `losexp()`, `rloc()`, `movemon()`, `justswld()`, `youswld()`,
`dochug()`, `mhit()`, `inrange()`, `m_move()`, `mnexto()`, `poisoned()`,
`steal()`, `newcham()`, `makemon()`, `hitu()`, `swallowed()`

- K&R→ANSI for ~20 functions
- `float` arithmetic in `newcham()` (line 578) — preserve

**ANOMALY — Comma operator bug** (line 584):

```c
newcham(mtmp,&mon[7,6]);
```

C comma operator evaluates `7,6` → `6`. This is `&mon[6]` (row 7, the
invisible stalker tier), not `&mon[7][6]` (the chameleon entry). Authentic
1982 bug. **PRESERVE** with documenting comment.

**Lint anomaly** (bugs1 line 47): `hack.mon.c line 38: lvalue required` —
investigate during conversion.

#### 4F. `hack.obj.c` (268 lines, ~10 functions)

Object handling. Naming, inventory, item selection, weight.
`doname()`, `useup()`, `getobj()`, `prinv()`, `weight()`, `gobj()`, `doinv()`

- K&R→ANSI, sprintf/strcpy formatting, index() via compat.h

#### 4G. `hack.cmd.c` (866 lines, largest file)

Command dispatcher. Massive `rhack()` switch statement.
`rhack()`, `drink1()`, `read1()`, `dosearch()`

- K&R→ANSI
- `execl()` bare 0 terminators → `(char *)NULL` (lines 47, 64, 65)
- Hardcoded `/usr/bin/cr3`, `/bin/sh` paths
- Signal handler type fixes
- `wait(0)` calls may need `(int *)NULL` cast

#### 4H. `hack.cmdsub.c` (429 lines, ~17 functions)

Command subroutines. Equipment, leveling, hunger, save game.
`losestr()`, `were()`, `ringoff()`, `bhit()`, `buzz()`, `zhit()`,
`chwepon()`, `pluslvl()`, `nothin()`, `lesshungry()`, `plusone()`,
`minusone()`, `docall()`, `more()`, `dodr1()`, `dosave()`, `rept()`

- K&R→ANSI, bwrite/mread for save, index() via compat.h

**>>> MILESTONE: After Phase 4, should compile, link, and be playable <<<**

---

### Phase 5: Build Integration

#### 5A. CMakeLists.txt — Add hack-release target

Append to existing CMakeLists.txt (hack-root/mklev-root targets unchanged):

```cmake
# --- hack-release (USENIX tape submission, single binary) ---
add_executable(hack-release
    src/release/hack.c
    src/release/hack.cmd.c
    src/release/hack.cmdsub.c
    src/release/hack.fight.c
    src/release/hack.files.c
    src/release/hack.levl.c
    src/release/hack.main.c
    src/release/hack.mon.c
    src/release/hack.move.c
    src/release/hack.obj.c
    src/release/hack.screen.c
    src/release/hack.see.c
    src/release/hack.save.c
    src/release/hack.lock.c
    src/root/rnd.c              # shared random number generator
)
target_include_directories(hack-release PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/release
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CURSES_INCLUDE_DIR}
)
target_compile_definitions(hack-release PRIVATE
    HACKDIR="${CMAKE_BINARY_DIR}/hackdir-release"
    VTONL
)
target_link_libraries(hack-release ${CURSES_LIBRARIES} crypt)
```

**Linking pitfalls**:

- Do not hardcode `-lcrypt`; probe it and link only if found.
- Termcap symbols may require `tinfo` on some distros; probe and link as needed.

#### 5B. hackdir setup target for release

Separate from existing hackdir (hack-root's `hackdir/` is untouched):

```cmake
add_custom_target(setup_hackdir_release ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/hackdir-release
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/hackdir-release/save
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_BINARY_DIR}/hackdir-release/perm
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/original/exp/exp1/record
        ${CMAKE_BINARY_DIR}/hackdir-release/record
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/original/news
        ${CMAKE_BINARY_DIR}/hackdir-release/news
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/original/moves
        ${CMAKE_BINARY_DIR}/hackdir-release/moves
)
add_dependencies(hack-release setup_hackdir_release)
```

---

## Anomalies Catalog

| # | File | Line | Description | Action |
|---|------|------|-------------|--------|
| 1 | hack.mon.c | 584 | `&mon[7,6]` comma operator — evaluates to `&mon[6]` | **PRESERVE** |
| 2 | hack.mon.c | 38 | "lvalue required" per 1982 lint | Investigate |
| 3 | hack.mon.c | 578 | `register float mp` in integer game | Preserve |
| 4 | hack.screen.c | 345 | pline() a5-a8 are `char` not `char *` | Moot after stdarg |
| 5 | hack.see.c | 18,46 | "maximium" typo | **PRESERVE** |
| 6 | hack.levl.c | ~343 | "Kenny's x/y reversed" comment | **PRESERVE** |
| 7 | hack.vars | 159 | `_mon` defined but never used | **PRESERVE** |
| 8 | hack.h | 234 | `extern multi;` missing type | Fix to `extern int multi;` |
| 9 | hack.vars | 323 | "aggrivate monster" misspelling | **PRESERVE** |
| 10 | hack.vars | 350 | "black onix" misspelling | **PRESERVE** |
| 11 | hackfoo.h | all | Duplicate header, unused | Preserve as artifact |
| 12 | hack.h/hackfoo.h | 155 | gminus macro parens differ | Use hack.h version |
| 13 | hack.vars | 19-20 | "unsigned chars will screw up" comment | Note: signed char |
| 14 | bugs/bugs1 | all | 1982 build log and lint output | Preserve in src/release/ |

---

## Conversion Order Summary

```
Phase 1: Headers & globals
  1A. hack.h         (sgtty removal, compat.h, prototypes)
  1B. hack.vars      (implicit type fixes)

Phase 2: Core utilities (+ Phase 6 modern logic applied inline)
  2A. hack.c         (terminal I/O, panic, pow→hack_pow + 6F: tcgetattr guards, ONLCR)
  2B. hack.screen.c  (pline varargs, display + 6A: mapok, cl_end, pru, prl, nocm)

Phase 3: Entry point & levels (+ Phase 6 modern logic applied inline)
  3A. hack.main.c    (main loop, signals + 6E: SIGHUP/SIGTERM, SIGWINCH resize)
  3B. hack.levl.c    (mklev, qsort, maze gen)
  >>> Should compile and generate levels

Phase 4: Game logic (+ Phase 6 modern logic applied inline)
  4A. hack.files.c   (level I/O + 6B: versioned format, pointer relocation)
  4B. hack.see.c     (visibility)
  4C. hack.move.c    (movement)
  4D. hack.fight.c   (combat)
  4E. hack.mon.c     (monsters — comma bug preserved + 6F: tcflush)
  4F. hack.obj.c     (objects)
  4G. hack.cmd.c     (commands — largest, execl fixes)
  4H. hack.cmdsub.c  (command helpers, save)
  4I. hack.save.c    (6C: new file — versioned save, adapted to release structs)
  4J. hack.lock.c    (6D: new file — flock() locking, copied from hack-root)
  >>> Should compile, link, and be playable

Phase 5: Build integration (hack-root untouched)
  5A. hack-release CMake target (appended to CMakeLists.txt)
  5B. hackdir-release/ setup target
  >>> Both hack-root and hack-release build independently

Phase 6 is integrated into Phases 2-4 (documented separately for reference):
  6A in 2B: hack.screen.c — mapok, cl_end, pru tracking, prl refresh, nocm ONLCR
  6B in 4A: hack.files.c  — versioned levels, monbegin relocation, legacy fallback
  6C in 4I: hack.save.c   — new file: versioned save system
  6D in 4J: hack.lock.c   — new file: flock()-based locking
  6E in 3A: hack.main.c   — SIGHUP/SIGTERM save, SIGWINCH resize
  6F in 2A: hack.c        — tcgetattr guards, ONLCR, record locking
  6F in 4E: hack.mon.c    — tcflush
  >>> Full modern runtime parity with hack-root
```

---

## Phase 6: Modern Portability Logic (Ported from hack-root)

The hack-root version accumulated several modern additions beyond K&R→ANSI
conversion during its restoration. These are functional improvements needed
for the game to run correctly on modern 64-bit Linux with large terminals,
modern terminal emulators, and POSIX signal semantics. The release version
needs equivalent logic, adapted to its different file structure.

### File Mapping (hack-root → release)

| hack-root File | Release Equivalent | Scope |
|----------------|-------------------|-------|
| `hack.pri.c` | `hack.screen.c` | Display, cursor, status bar, pline |
| `hack.lev.c` (save/load) | `hack.files.c` | Level file I/O (savelev/getlev) |
| `hack.lev.c` (mklev/fork/exec) | `hack.levl.c` | Level generation (mklev is internal) |
| `hack.main.c` | `hack.main.c` | Entry point, signals, game loop |
| `hack.c` | `hack.c` | Core utilities, terminal I/O |
| `hack.save.c` (new) | `hack.save.c` (new) | Modern versioned save system |
| `hack.lock.c` (new) | `hack.lock.c` (new) | flock()-based locking |
| `hack.do.c` | `hack.cmd.c` | Command dispatcher, execl fixes |
| `hack.mon.c` | `hack.mon.c` | Monster AI, tcflush |

---

### 6A. Screen Hardening → `hack.screen.c`

**Source**: `src/root/hack.pri.c` (commits 076c25c, 2b65081, 80bf576)

These additions prevent crashes and rendering artifacts on modern terminals
that exceed the original 80×24 assumption.

#### mapok() bounds guard

```c
/* Modern: guard against out-of-bounds screen updates on large terminals */
static int mapok(int x, int y)
{
    return(x>=0 && x<80 && y>=0 && y<22);
}
```

**Where needed in release**: `atl()`, `on()`, `at()`, `pru()`, `prl()`,
`docrt()` — same functions exist in `hack.screen.c` at similar line ranges.
Add mapok() and insert guard checks at the same positions as hack-root.

#### cl_end() fallback

hack-root replaced the `#define cl_end() fputs(CE,stdout)` macro with a
function that falls back to space-filling when CE is unavailable:

```c
/* Modern: use termcap-aware clear-to-end-of-line with fallback */
void cl_end(void)
{
    if(CE) { xputs(CE); return; }
    /* Modern: fallback when CE missing (clear to column 80) */
    { int cx=curx, cy=cury;
      while(curx<80) { putchar(' '); curx++; }
      curs(cx,cy);
    }
}
```

**Where needed in release**: `hack.h` defines `#define cl_end() fputs(CE,stdout)`.
Remove macro, add `void cl_end(void)` prototype to hack.h, implement in
hack.screen.c. `pline()` in hack.screen.c already calls `cl_end()`.

#### pru() player tracking

hack-root added static variables to track the last displayed `@` position,
preventing stale player symbols when the player moves:

```c
/* Modern: track last displayed @ to keep redraws in sync on modern terminals */
static char pudx = -1;
static char pudy = -1;
static char pudis = 0;
```

**Where needed in release**: `pru()` in hack.screen.c (line ~97). The
original is only 5 lines; expand with the same tracking logic as hack-root.

#### prl() unseen tile refresh

hack-root added a refresh path for tiles that haven't been seen or show
stale space symbols:

```c
else if(!room->seen || room->scrsym==' ') {
    /* Modern: refresh unseen tiles to avoid stale symbols on modern terminals */
    room->new=room->seen=1;
    newsym(x,y);
    on(x,y);
}
```

**Where needed in release**: End of `prl()` in hack.screen.c (line ~114).

#### nocm() ONLCR column tracking

hack-root fixed column tracking when using newlines for downward cursor
movement:

```c
curx=1; /* Modern: newline returns to column 1 with ONLCR */
```

**Where needed in release**: `nocm()` in hack.screen.c (line ~210), inside
the `cury < y` loop after `putchar('\n')`.

#### tputs() wrapper for termcap padding

hack-root replaced direct `fputs()` for termcap strings with `tputs()` to
handle padding sequences (`$<..>`) correctly:

```c
/* Modern: use tputs() so termcap padding ($<..>) isn't printed literally */
static int hack_putc(int c) { return putchar(c); }
static void xputs(char *s) { if(s) tputs(s,1,hack_putc); }
```

**Where needed in release**: Only matters under `#ifndef VTONL`. Since we
compile with VTONL, the release `xputs()` uses `fputs()` (which is correct
for literal ANSI escapes). Include the tputs path in the `#ifndef VTONL`
block for completeness, matching hack-root.

---

### 6B. Versioned Level Format → `hack.files.c`

**Source**: `src/root/hack.lev.c` (commits f5b141b, 9c4fd28)

The original savelev/getlev write raw structs with embedded pointers. On
64-bit systems, `struct permonst *data` in `struct monst` becomes 8 bytes
instead of the original 2, and the pointer value changes between runs.
hack-root added a versioned level header with pointer relocation.

#### savelev() versioned header + cache-next-before-free

```c
#define LEVEL_MAGIC "FLEV"
#define LEVEL_VERSION 1

void savelev(FILE *fp)
{
    unsigned short lver=LEVEL_VERSION;
    struct permonst *monbegin=&mon[0][0];

    bwrite(fp,LEVEL_MAGIC,4);
    bwrite(fp,&lver,sizeof(lver));
    bwrite(fp,&monbegin,sizeof(monbegin));
    bwrite(fp,levl,sizeof(levl));
    bwrite(fp,&moves,sizeof(moves));
    /* ... linked list iteration with cache-next-before-free ... */
```

The cache-next pattern prevents use-after-free when freeing list nodes:

```c
for(stmp=fstole;stmp;) {
    struct stole *stnext=stmp->nstole; /* Modern: cache next before free */
    /* ... write and free stmp ... */
    stmp=stnext;
}
```

**Where needed in release**: `savelev()` in `hack.files.c` (line ~130).
The release savelev has the same linked-list-free pattern as hack-root. Add
level magic header, monbegin pointer write, sizeof-based writes, and
cache-next-before-free for all five linked list loops (fstole, fmon,
fgold, ftrap, fobj).

#### getlev() pointer relocation + legacy fallback

```c
int getlev(FILE *fp)
{
    ptrdiff_t differ=0;
    struct permonst *saved_monbegin=0;
    int legacy=0;

    /* Read magic header, fall back to legacy if missing */
    if(memcmp(lmagic,LEVEL_MAGIC,4)) { legacy=1; fseek(fp,0,SEEK_SET); }

    /* For versioned format: relocate permonst pointers */
    if(saved_monbegin) differ=(char *)&mon[0][0] - (char *)saved_monbegin;

    /* Apply relocation when reading monsters */
    if(saved_monbegin && mbuf.data)
        mbuf.data=(struct permonst *)((char *)mbuf.data + differ);
    if(!mbuf.data || !mbuf.data->mlet) { /* null-safe check */ }
```

**Where needed in release**: `getlev()` in `hack.files.c` (line ~147).
Same pattern — add magic check, monbegin read, pointer relocation,
null guards on `mbuf.data` before dereferencing `mbuf.data->mlet`.

**Note on release's getlev**: The original has no return value per the
1982 lint output ("getlev value is used, but none returned"). hack-root
fixed this to return int. Release needs the same fix.

---

### 6C. Modern Save System → `hack.save.c` (new file)

**Source**: `src/root/hack.save.c` (commit f5b141b, 535 lines)

hack-root's `hack.save.c` is an entirely new file that replaces the
original raw-pointer save format with a versioned, pointer-safe system
using fixed-width integers. It includes:

- **Save header**: `SAVE_MAGIC "FHCK"`, `SAVE_VERSION 1`, endian tag
- **Fixed-width I/O**: `sw_u8/sw_u16/sw_u32` write helpers, `sr_u8/sr_u16/sr_u32` read helpers
- **String serialization**: `save_string()/load_string()` with length prefix
- **Array serialization**: `save_string_array()/load_string_array()` for potcol/scrnam/wannam/rinnam
- **Struct serializers**: `save_flags()/load_flags()`, `save_you()/load_you()`, `save_obj()/load_obj()`
- **Inventory chain**: `save_invent()/load_invent()` with worn-item bitmask
- **ustuck tracking**: `ustuck_index()/restore_ustuck()` by monster list position
- **dosave0()**: Atomic save (write to .tmp, rename), signal masking
- **dorecover()**: Full restore with error diagnostics
- **hangup()/modern_save_handler()**: SIGHUP/SIGTERM save handlers

**What changes for release**: The release version's struct layouts differ
from hack-root (release has `struct you` with different fields, `struct flag`
with different bitfields). The serializers must be adapted:

- `save_you()/load_you()`: Map to release's `struct you` fields
- `save_flags()/load_flags()`: Map to release's `struct flag` fields
- `save_obj()/load_obj()`: Map to release's `struct obj` fields
- String arrays: Release uses same potcol/scrnam/wannam/rinnam names

**dosave() in hack.cmdsub.c**: The original release `dosave()` (line ~400
of hack.cmdsub.c) is a thin wrapper. Keep it calling into `dosave0()` in
the new hack.save.c, same as hack-root.

**File list for CMake**: Add `src/release/hack.save.c` to hack-release
sources in Phase 5A.

---

### 6D. Modern Lock System → `hack.lock.c` (new file)

**Source**: `src/root/hack.lock.c` (commit f5b141b, 117 lines)

hack-root replaced the original `link(perm,safelock)` lock mechanism with
`flock()`-based locking. The original link-based locks are fragile on
modern filesystems and leave stale locks after crashes.

**Functions**:

- `modern_lock_game()` — exclusive flock on `game.lock`, with retry/timeout
- `modern_unlock_game()` — release game lock
- `modern_lock_record()` — exclusive flock on `record.lock`, for high score updates
- `modern_unlock_record()` — release record lock
- `modern_cleanup_locks()` — probe and release stale locks

**What changes for release**: The lock system is filesystem-only, no
struct dependencies. Can be copied directly from hack-root to
`src/release/hack.lock.c` with no changes.

**Integration points**:

- `hack.main.c lockcheck()`: Replace `link(perm,safelock)` with
  `modern_cleanup_locks()` + `modern_lock_game()`
- `hack.main.c leave()`: Replace `unlink(safelock)` with
  `modern_unlock_game()`
- `hack.c done()`: Wrap record file I/O with
  `modern_lock_record()`/`modern_unlock_record()`
- `hack.c done2()`: Add `modern_unlock_game()` before exit

**Prototypes in hack.h**: Add same 5 prototypes as hack-root:

```c
int modern_lock_game(void);
void modern_unlock_game(void);
int modern_lock_record(void);
void modern_unlock_record(void);
void modern_cleanup_locks(void);
```

---

### 6E. Signal Handling → `hack.main.c`

**Source**: `src/root/hack.main.c` (commits af6a821, 9c4fd28)

#### SIGHUP/SIGTERM save handler

hack-root added `hangup()` (defined in hack.save.c) as handler for both
SIGHUP and SIGTERM, ensuring the game saves when the terminal closes
or the process is terminated:

```c
signal(SIGTERM,hangup); /* Modern: save on terminate */
signal(SIGHUP,hangup);  /* Modern: save on hangup */
```

**Where needed in release**: All signal setup points in hack.main.c
(both the lock-file path and the restore path). Replace the original
`signal(15,fooexit)` / bare SIGTERM handling.

#### SIGWINCH deferred resize

hack-root added a deferred-redraw system to handle terminal resizing
without corrupting game state mid-turn:

```c
#ifdef SIGWINCH
static volatile int resize_pending=0;
static int resize_warned=0;
void handle_resize(int signum) {
    (void)signum;
    resize_pending=1;
    signal(SIGWINCH,handle_resize);
}
static void check_resize(void) {
    struct winsize ws;
    if(!resize_pending) return;
    resize_pending=0;
    if(ioctl(0,TIOCGWINSZ,&ws)==0) {
        if(ws.ws_col<80 || ws.ws_row<24) {
            /* warn and wait for resize */
        }
    }
    resize_warned=0;
    docrt();
}
#endif
```

**Where needed in release**: Top of hack.main.c (before main), with
`check_resize()` called in the game loop. The release game loop is
in hack.main.c around line 150-230. Insert `check_resize()` at the
same position as hack-root (inside the `#else` / `flags.move=1` block).

**compat.h**: Needs `#include <sys/ioctl.h>` for TIOCGWINSZ (already
present in current compat.h).

---

### 6F. Terminal Tolerance → `hack.c`, `hack.files.c`

**Source**: `src/root/hack.c` (commit 9c4fd28)

#### cbout()/cbin() tcgetattr guards

hack-root added error tolerance for missing TTYs (e.g., running under a
test harness or redirected I/O):

```c
if(tcgetattr(0,&ttyp)!=0) return; /* Modern: tolerate missing tty */
```

**Where needed in release**: `cbout()` and `cbin()` in hack.c (lines
198-215 of original). Apply same guard after the termios conversion.

#### OPOST|ONLCR in raw mode

hack-root explicitly enables output post-processing in cbin() to ensure
newlines produce carriage returns for screen positioning:

```c
ttyp.c_oflag |= OPOST|ONLCR; /* Modern: keep CRLF in raw mode for screen positioning */
```

**Where needed in release**: `cbin()` in hack.c, after clearing ICANON/ECHO.

#### tcflush() for input flushing

hack-root replaced the V7 `ioctl(0,TIOCFLUSH,...)` with POSIX `tcflush()`:

```c
if(flags.flush) tcflush(0, TCIFLUSH); /* Modern: POSIX termios */
```

**Where needed in release**: `movemon()` in hack.mon.c (around line 112
of hack-root, equivalent position in release). The original uses
`ioctl(0,TIOCFLUSH,&in)` — replace with `tcflush(0, TCIFLUSH)`.

#### Record file locking and error handling in done()

hack-root added record file locking and zero-init for safety:

```c
rec_locked=modern_lock_record();
if(!rec_locked) puts("Warning: record file lock unavailable.");
/* ... record I/O ... */
if(rec_locked) modern_unlock_record();
modern_unlock_game();
```

**Where needed in release**: `done()` in hack.c (lines 48-143 of original).

---

### 6G. Architectural Note: What Does NOT Port

Some hack-root modern additions are **not needed** in release due to
architectural differences:

| hack-root Feature | Why Not Needed in Release |
|-------------------|--------------------------|
| `-fno-pie` / `-no-pie` compiler flags | hack-root needs stable addresses between hack-root and mklev-root (two binaries sharing data via exec+level files). Release is a single binary. |
| `execl("./mklev",...)` in hack.lev.c | Release calls `mklev()` directly as a function. No fork/exec. |
| `mklev.c` versioned level header | Release's mklev logic is in hack.levl.c, writes via hack.files.c's savelev(). One set of level I/O code. |
| `mklev.c sig_abort()` wrapper | Release doesn't run mklev as a separate process with separate signal handlers. |
| `hack.do1.c` (partially converted) | Release has no hack.do1.c — that functionality is in hack.cmd.c and hack.cmdsub.c. |
| `&levl[0][0]` pointer arithmetic fix in hack.do.c | Release's equivalent code is in hack.cmd.c — investigate if same pattern exists. |

---

### Phase 6 Summary

```
Phase 6: Modern portability logic
  6A. hack.screen.c — mapok(), cl_end(), pru() tracking, prl() refresh, nocm() ONLCR
  6B. hack.files.c  — versioned level format (LEVEL_MAGIC, monbegin, legacy fallback)
  6C. hack.save.c   — new file: versioned save system (adapted from hack-root structs)
  6D. hack.lock.c   — new file: flock()-based locking (copy from hack-root, unchanged)
  6E. hack.main.c   — SIGHUP/SIGTERM handlers, SIGWINCH deferred resize
  6F. hack.c + hack.mon.c — tcgetattr guards, ONLCR, tcflush, record locking
```

**Implementation order**: Phase 6 work is integrated during Phases 2-4
as each file is converted. It is documented separately here for tracking
but should be applied file-by-file, not as a separate pass:

- 6A applied during Phase 2B (hack.screen.c conversion)
- 6B applied during Phase 4A (hack.files.c conversion)
- 6C created during Phase 4H (after hack.cmdsub.c, dosave integration)
- 6D created during Phase 3A (hack.main.c needs lock functions)
- 6E applied during Phase 3A (hack.main.c conversion)
- 6F applied during Phase 2A (hack.c) and Phase 4E (hack.mon.c)

---

## Conversion Patterns Reference (from hack-root)

| Pattern | Original | Modern | Reference |
|---------|----------|--------|-----------|
| Terminal I/O | gtty/stty + sgttyb | tcgetattr/tcsetattr + termios | src/root/hack.c |
| VARARGS | /\*VARARGS1\*/ + fixed args | stdarg.h + va_list/vprintf | src/root/mklev.c |
| Signal handler | `done1()` implicit int | `void done1(int signum)` | src/root/hack.main.c |
| pow() rename | `pow(num)` | `hack_pow(num)` | src/root/hack.c |
| execl() term | `execl(p,a,0)` | `execl(p,a,(char *)NULL)` | src/root/hack.main.c |
| qsort comp | `comp(x,y) int *x,*y;` | `int comp(const void *a,b)` | src/root/mklev.c |
| index/rindex | `index(s,c)` | strchr via compat.h | all files |

---

## Verification Plan

### Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build 2>&1 | tee warnings-release.log
```

- Both `hack-root` and `hack-release` binaries produced
- hackdir/ and hackdir-release/ created with runtime files
- Review warning counts for each target

**Order**: First reach a clean compile and playable run (no versioned levels
or modern save system). After that baseline is stable, layer in Phase 6B/6C
and re-verify. This keeps risk isolated and avoids masking core conversion
errors.

### Runtime (hack-release)

- Game starts, level displays, status bar renders
- Movement (hjklyubn) works
- Staircase descent triggers mklev() (level generation)
- Monsters spawn, move, attack
- Combat hit/miss messages display
- Inventory works
- Save/restore cycle (#ifndef SMALL)
- Quit sequence (done/done1/done2)
- Terminal restores on exit (cbout)

### Modern portability (Phase 6 verification)

- Terminal resize (SIGWINCH): shrink below 80×24, get warning, expand back, game redraws
- Hangup save (SIGHUP): `kill -HUP <pid>` produces save file
- Terminate save (SIGTERM): `kill <pid>` produces save file
- Versioned levels: descend stairs, ascend back — level data preserved correctly
- Save/restore: save game, restart, restore — all state intact (inventory, position, monsters)
- Record locking: two simultaneous hack-release instances don't corrupt record file
- Game locking: second instance gets "Try again in a minute" message
- Screen bounds: large terminal doesn't crash on out-of-bounds screen writes
- Missing CE: if CE unavailable, pline() still clears message line (space fallback)

### Regression (hack-root)

- Still builds and runs unchanged (no modifications to src/root/ or existing CMake targets)
- No modifications to original/ directory
- Same gameplay behavior as before

### Preservation checks

- All misspellings intact
- Comma operator bug intact in hack.mon.c
- "maximium" typos intact
- "Kenny's fault" comment intact
