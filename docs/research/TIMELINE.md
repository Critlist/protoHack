# Hack Development Timeline

## Corrected Chronology

Primary source evidence from *;login:* (Vol. 7, No. 3, June 1982) shows the
code was already complete and submitted for the distribution tapes **before**
the Summer 1982 USENIX conference took place.

### Timeline

- **1980** — **Rogue** released for Unix by Michael Toy and Glenn Wichman at
  UC Santa Cruz, with later contributions by Ken Arnold.

- **1981** — **Jay Fenlason** begins Hack development at Lincoln-Sudbury
  Regional High School, with help from Kenny Woodland, Mike Thome, and Jon
  Payne. The development environment is a PDP-11/70 running V7 Unix (alpha
  test site for 2.9BSD), administered by a student-run Computer Center Users
  Society with ~50 members who have keys and unsupervised access. The game is
  an implementation of Rogue with 56 monster types (vs. Rogue's 26) and
  expanded dungeon features. Jonathan Payne (JP), who contributed the lock
  file system and CURS() to Hack, is also the author of JOVE.

- **First half of 1982** — Brian Harvey, Computer Director at Lincoln-Sudbury
  (1979-1982), submits student projects — including Hack and JOVE — for
  inclusion on the USENIX distribution tapes (82-1). Harvey, whose background
  was in the MIT and Stanford AI labs, had built the school's computing
  environment to resemble those labs: "a powerful computer system, with lots
  of software tools, an informal community spirit, and not much formal
  curriculum."

- **June 1982** — *;login:* Vol. 7, No. 3 reports that the first 1982 USENIX
  distribution tape has been completed, noting the Lincoln-Sudbury Regional
  High School submission containing "quite a few games."

- **July 8, 1982 (Boston USENIX)** — Michael C. Toy and Kenneth C. R. C.
  Arnold present "Rogue: Where It Has Been, Why It Was There, and Why It
  Shouldn't Have Been There in the First Place." The 82-1 distribution tapes
  — with Hack already on them — are distributed at the same conference.

- **~1983/84** — **Michiel Huisjes** and **Fred de Wilde** at Vrije Universiteit,
  Amsterdam produce a PDP-11 version based on pre-1.0 Hack that Andries
  Brouwer later described (in an April 1985 Usenet reply) as copied from his
  directory without permission while 1.0 was still in development; he noted it
  was not in shape for distribution and lacked many features present in Hack
  1.0.

- **Dec 17, 1984** — **Andries Brouwer** at Stichting Mathematisch Centrum
  (CWI), Amsterdam distributes **Hack 1.0** to `net.sources` in 15 parts.
  Sender: `play@mcvax.UUCP (funhouse)`. The announcement promised 10 parts
  but there were actually 15, all sent on the same day. Near-complete rewrite
  of Fenlason's code — Brouwer later wrote that 1.0.3 "contains very little
  if anything from the original sources."

- **~Dec 1984** — `net.games.hack` newsgroup created by Gene Spafford due to
  the volume of Hack traffic on net.games and net.games.rogue.

- **Jan 1985** — **Hack 1.0.1** patch adding a few features. Sender:
  `play@turing.UUCP`.

- **Feb 1985** — **Hack for PDP-11** published on Usenet (`net.sources`) by
  Huisjes. Five shar parts.

- **Apr 1-14, 1985** — **Hack 1.0.2** re-distributed as a fresh copy of the
  full source, spread over two weeks. Sender: `aeb@mcvax.UUCP`. The 1.0
  distribution's single-day dump had overwhelmed many sites, and the 1.0.1
  patch required the `patch` utility which was not universally available. Part
  2 was famously missing from Google Groups until Ray Chason located it in
  2005 in the DECUS library at `vmsone.com/~decuslib/unixsig/uni87a/hack/`.

- **May 1985** — **PC/IX Hack** published on Usenet (`net.sources.games`).
  Port of the PDP-11 version to IBM PC UNIX. Five shar parts.

- **Jul 23, 1985** — **Hack 1.0.3** distributed as an ed script against 1.0.2.
  Last version distributed by Brouwer. Well preserved — copies found across
  the net.

- **Jul 28, 1987** — **NetHack 1.3d** released. Mike Stephenson, Izchak
  Miller, and Janet Walz fork Hack 1.0.3 and begin independent development.

### Quest (Brouwer, undistributed)

Brouwer also wrote **Quest**, a game sharing most of Hack's source but using
its own level generator (`quest.mklev.c`) that produced more interesting cave
shapes instead of "boring rectangles." Quest was never officially distributed
but leaked — a copy appeared at Vrije Universiteit, and it was listed among
evidence seized in the 1990 Secret Service raids documented in Bruce Sterling's
*The Hacker Crackdown*. Brouwer's own copy was lost when an email transfer
from Amsterdam to Denmark was silently discarded by a gateway for exceeding
100 KB.

## Key Evidence

The *;login:* evidence shows Hack was already submitted for the distribution
tapes before the conference. Jay likely encountered Rogue through the SFSU Logo
Workshop (as stated in his Original_READ_ME: "This entire program would not have
been possible without the SFSU Logo Workshop ... without whom I would never have
seen Rogue").

## Licensing

Both original authors have issued BSD-type licenses allowing free
redistribution and modification:

**Jay Fenlason** (covers all code he wrote):
> It is shared under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License (CC-BY-NC-SA 4.0), as specified by Jay Fenlason when the source was archived by the Snap!Hack project. A copy of the license is available at: https://creativecommons.org/licenses/by-nc-sa/4.0/

**CWI** (covers Brouwer's additions, formerly "Stichting Mathematisch
Centrum"):
> Copyright (c) 1985, Stichting Centrum voor Wiskunde en Informatica,
> Amsterdam — 3-clause BSD license

Statement issued by Dick Broekhuis, controller CWI.

Full license texts preserved in Brouwer's published account.

## Primary Sources

- *;login:* Vol. 7, No. 3 (June 1982) — USENIX distribution tape announcement
  - [Archive.org scan](https://archive.org/details/login_june-1982/page/n13/mode/2up)
  - [Full text](https://archive.org/stream/login_june-1982/login_june-1982_djvu.txt)
  - Local copy: `login_june-1982.pdf` (in this directory)
- Jay Fenlason's `Original_READ_ME` (preserved in Brouwer's Hack 1.0)
- Jay Fenlason's `READ_Me` (preserved in this repository at `original/READ_Me`)
- Andries Brouwer's [Hack history page](https://homepages.cwi.nl/~aeb/games/hack/hack.html)
- Brian Harvey, ["Computer Hacking and Ethics" — A Case Study: The Lincoln-Sudbury
  Regional High School](https://people.eecs.berkeley.edu/~bh/lsrhs.html) (appendix
  to ACM Select Panel on Hacking position paper, 1985)
- Neozeed, "While hunting for Hack 1.0 in usenet" (reproduces Brouwer's 1985
  Usenet response about the PDP-11 version)
- Usenet archives via SuperGlobalMegaCorp Altavista Archive

## Lineage

```
Rogue (1980, Toy/Wichman/Arnold)
  |
  v
Fenlason Hack (1981-82, Lincoln-Sudbury)
  |
  +---> PDP-11 Hack (~1983, Huisjes & de Wilde, VU Amsterdam)
  |       |
  |       +---> PC/IX Hack (1985, IBM PC UNIX port)
  |
  +---> Hack 1.0 (Dec 1984, Brouwer, CWI Amsterdam)
  |       |
  |       +---> Hack 1.0.1 (Jan 1985)
  |       +---> Hack 1.0.2 (Apr 1985)
  |       +---> Hack 1.0.3 (Jul 1985)
  |               |
  |               +---> NetHack 1.3d (Jul 1987, Stephenson/Miller/Walz)
  |
  +---> Quest (Brouwer, undistributed — lost)
```
