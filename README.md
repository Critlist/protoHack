# protoHack

**Restoring Jay Fenlason's original 1982 Hack to run on modern Linux.**

protoHack applies the same preservation-first philosophy as
[restoHack](https://github.com/Critlist/restoHack), but to an earlier,
pre-distribution working tree. This source predates every currently known distributed
Hack variant. While restoHack restored the 1985 release, protoHack reaches
further back to the original 1981-82 codebase.

![Welcome screen](docs/media/fenlason_welcome.png)
*"Mail bugs to jf." — Jay Fenlason's welcome message, running on a modern terminal in 2026.*

![Gameplay](docs/media/rendering_works.png)
*Dungeon rendering and status line working. Note the scroll labeled "Andova Begarin" —
one of Fenlason's original randomized scroll names.*

## Lost and Found

This source was widely considered lost for over 40 years. Jay Fenlason put Hack on
the USENIX 82-1 distribution tape and, by his own account, "forgot about it."
By the time he
[spoke to Julie Bresnick in 2000](https://www.linux.com/news/train-life-nethacks-papa/),
he had long since moved on; he still played his original version at home, but
had "voluntarily avoided participation pretty much since spawning the original
Hack almost 20 years ago." Andries Brouwer's near-total rewrite as Hack 1.0
(1984) became the version the world knew, and Fenlason's original source
largely dropped out of the historical record.

It resurfaced in 2025 when Brian Harvey, who had been Computer Director at
Lincoln-Sudbury Regional High School during Hack's development, provided his
preserved copy of the school's PDP-11 backups to Dan Stormont for the
[Snap!Hack](https://github.com/Sustainable-Games/snaphack) educational
project. Dan published the complete original working tree at
[Sustainable-Games/fenlason-hack](https://github.com/Sustainable-Games/fenlason-hack).

Chain of custody:

1. **Jay Fenlason** — original author, 1981-82 (with Kenny Woodland, Mike
   Thome, and Jon Payne)
2. **Brian Harvey** — preserved from LSRHS PDP-11 backups, 1982-2024
3. **Dan Stormont** — Snap!Hack project, 2024-present

## What Makes This Hack Different

This is Hack before it became Hack. Before Andries Brouwer rewrote it as
Hack 1.0 at CWI Amsterdam, before Huisjes and de Wilde expanded it at VU
Amsterdam, before NetHack; this is Jay Fenlason's high school original from
Lincoln-Sudbury Regional High School:

- **Amulet of Frobozz** — not yet renamed to "Amulet of Yendor"
- **No shops** — shopkeepers were added later by the Dutch developers
- **No starting pet** — you're on your own down there
- **Displacer beast** — appears to be the only known Hack variant to include one (the 'd' slot
  was later reassigned to "dog" when pets were added)
- **56 monsters** across 8 depth levels — including unique creatures like
  "ugod", "xerp", and "zelomp" that appear nowhere else
- **Authentic misspellings** — "homonculous", "gelatenous cube" (preserved,
  not fixed)
- **8 source files** — the entire game, written on a PDP-11/70 running
  V7 Unix (2.8BSD alpha test site)

## Goals

- Preserve original behavior and structure
- Make the code buildable on modern POSIX systems
- Avoid gameplay or design changes beyond what is required for portability
- Document provenance and historical context

## Building

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

Requires: `ncurses`, `crypt` (libxcrypt on modern distros)

Produces: `hack-root` and `mklev-root` in the build directory, plus a
`hackdir/` with runtime data files.

## Status

The restoration converts K&R C (1978-era function definitions, implicit
types, V7 Unix system calls) to ANSI C with POSIX equivalents. All original
logic, bugs, and behavior are preserved as faithfully as possible.

### src/root/ (early development build)

| File | Status | Notes |
|------|--------|-------|
| hack.c | Done | 32 functions, termios rewrite for terminal I/O |
| hack.main.c | Done | 11 functions, V7 signal handler cleanup |
| mklev.c | Done | 19 functions, qsort/stdarg/signal fixes |
| rnd.c | Done | 4 functions |
| hack.h | Done | Modern prototypes |
| hack.pri.c | In progress | Display/rendering, termcap calls |
| hack.do.c | Not started | Player commands, pointer arithmetic issues |
| hack.do1.c | Not started | More player commands |
| hack.mon.c | Not started | Monster behavior |
| hack.lev.c | Not started | Level file I/O, exec's mklev |

### src/exp1/ (USENIX tape submission)

Not started. Will be ported after root is complete. This is the version
Fenlason refactored into 12 files with shared headers; likely the build
that was submitted for the USENIX 82-1 distribution tape. Root is the
earlier working copy — what we'd call an alpha or beta build today.

## How It Works

This is a two-binary game, a consequence of PDP-11 memory constraints.
`hack-root` is the main game. When it needs a new dungeon level, it `exec`s
`mklev-root` as a separate process to generate the level file, then reads the
result back. This architecture is preserved; the binaries are not merged.

A `compat.h` shim handles the BSD-to-POSIX translation: `index` becomes
`strchr`, `gtty`/`stty` become `tcgetattr`/`tcsetattr`, V7 variadic
conventions become `<stdarg.h>`, and hardcoded paths become a CMake-defined
`HACKDIR`.

## Lineage

```
Rogue (1980, Toy/Wichman/Arnold, UC Santa Cruz)
  |
  v
Fenlason Hack (1981-82, Lincoln-Sudbury)    <-- you are here
  |
  +---> PDP-11 Hack (~1983/84, Huisjes & de Wilde, VU Amsterdam)
  |       |
  |       +---> PC/IX Hack (1985, IBM PC UNIX port)
  |
  +---> Hack 1.0 (Dec 1984, Brouwer, CWI Amsterdam)
  |       |
  |       +---> Hack 1.0.1 (Jan 1985)
  |       +---> Hack 1.0.2 (Apr 1985)
  |       +---> Hack 1.0.3 (Jul 1985)   <-- restoHack restores this one
  |               |
  |               +---> NetHack 1.3d (Jul 1987)
  |                       |
  |                       +---> ... all NetHack versions
  |
  +---> Quest (Brouwer, undistributed — lost)
```

## Related Projects

[github.com/Sustainable-Games](https://github.com/Sustainable-Games) (Dan Stormont):

- **[fenlason-hack](https://github.com/Sustainable-Games/fenlason-hack)** —
  The unmodified original 1982 working tree as preserved by Brian Harvey
- **[Snap!Hack](https://github.com/Sustainable-Games/snaphack)** —
  Educational reimplementation in Snap!; the project that prompted Brian
  Harvey to share the preserved source in the first place

[github.com/Critlist](https://github.com/Critlist):

- **[restoHack](https://github.com/Critlist/restoHack)** — The sequel that
  came first: restoration of Hack v1.0.3 for modern systems
- **[hack-1.0](https://github.com/Critlist/hack-1.0)** — Andries Brouwer's
  Hack 1.0 (CWI Amsterdam, Dec 1984)
- **[hack-pdp11](https://github.com/Critlist/hack-pdp11)** — PDP-11/PC/IX
  Hack by Huisjes & de Wilde (VU Amsterdam)

## Research

See `docs/research/` for historical analysis:

- **TIMELINE.md** — Corrected chronology with primary sources (the NetHack
  Wiki's claim that Hack was inspired by the 1982 USENIX Rogue talk is
  disproven by *;login:* evidence showing the code was already on the
  distribution tape before the conference)
- **COMPARISON.md** — Detailed monster table and feature comparison across
  all known Hack variants
- **login_june-1982.pdf** — The *;login:* issue announcing the USENIX tape

## License

The licensing of this source is historically ambiguous and may be clarified 
as new primary sources emerge.

**What we know:**

This source code was created primarily by Jay Fenlason, with additional
contributions from Kenny Woodland, Mike Thome, and Jon Payne at
Lincoln-Sudbury Regional High School, 1981-82. It was distributed on the
USENIX 82-1 tape, which predates modern open-source licensing conventions.

**Current distribution terms:**

Dan Stormont, custodian of the Brian Harvey preservation copy, distributes
the [original unmodified source](https://github.com/Sustainable-Games/snaphack)
under **CC-BY-NC-SA 4.0**, described as "the closest modern license to the
original distribution license."

**What does *not* apply:**

The 3-clause BSD license issued by CWI (Stichting Centrum voor Wiskunde en
Informatica) covers Andries Brouwer's Hack 1.0 and later; code written at
CWI Amsterdam beginning in late 1984. That license has no bearing on
Fenlason's 1982 source, which predates Brouwer's involvement by two
years.<sup>[1]</sup>

**The open question:**

Jay Fenlason issued a retroactive 3-clause BSD license covering "all code he
wrote." This statement appears in Brouwer's published
[Hack history](https://homepages.cwi.nl/~aeb/games/hack/hack.html), where
it served to clarify the licensing of Jay's contributions as they flowed into
the Hack 1.0 lineage. Whether Jay intended that license to apply to *this
specific pre-distribution copy*, preserved independently by Brian Harvey and
never part of the Brouwer lineage, is an assumption, not a documented
fact.<sup>[2]</sup>

**In practice:**

This restoration (protoHack) follows the CC-BY-NC-SA 4.0 terms established
by the current custodian. If future clarification from Jay Fenlason or other
primary sources resolves the ambiguity, this section will be updated
accordingly.

---

<sup>[1]</sup> Brouwer himself wrote that Hack 1.0.3 "contains very little
if anything from the original sources." The CWI license covers a near-total
rewrite, not this code.

<sup>[2]</sup> The 1982 USENIX distribution tapes had no standardized
licensing. Software was submitted and shared under informal academic norms.
The concept of an "open-source license" as we understand it today did not
exist until the late 1980s.
