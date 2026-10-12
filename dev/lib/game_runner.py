#!/usr/bin/env python3
"""
protoHack (1982 Hack) game automation for testing.

Ported from restoHack's dev/lib/game_runner.py: same interface
(run_session(), resizes, resizes_seen, fail_reason, child), different game.
protoHack has no character prompt, no wizard flag, and its own pager and
resize screens, so the session flow is its own:

- reads the screen with pyte and walks to '>' (BFS on the visible map) or
  to the nearest unexplored edge, so runs get below level 1;
- presses keys aimed at the S1 fixes: ^R with no message, 'o' and 'c' with
  long / comma / '%' text (getlin cap, name=, pline buffer, format strings);
- in enhanced mode, saves or hangs up and checks the restore, and resizes
  the terminal below 80x24;
- starts a fresh game when one ends with steps left (after checking it).

Every choice comes from self.rng, so a seed replays the same session.
"""

import os
import re
import signal
import sys
import time
import random
from pathlib import Path
from typing import Tuple, Optional

import pexpect
import pyte

COLS, ROWS = 80, 24
# After each key, read output until the game is quiet this long (seconds)
KEY_SETTLE = 0.03
# Enhanced mode: per-step chance of a save/hangup + restore and of a resize
RESTART_CHANCE = 0.002
RESIZE_CHANCE = 0.002
# A game that prints nothing for this long while we send keys is hung
HANG_SECS = 20
# Level files are <lock>.<N>; the save is save/<login>
_LEVEL_FILE_RE = re.compile(r"^.+\.\d+$")
_SANITIZER_RE = re.compile(rb"ERROR: AddressSanitizer|runtime error:|ERROR: LeakSanitizer")
MOVES = {(-1, 0): "h", (1, 0): "l", (0, -1): "k", (0, 1): "j",
         (-1, -1): "y", (1, -1): "u", (-1, 1): "b", (1, 1): "n"}
# Text for option/naming prompts: long (getlin cap), ',' (record file), '%'
NASTY_TEXT = ["%s%s%s%s", "%n%n", "%x%x%x%x%x", "a,b,c", "x" * 120,
              "name=Jay,the%sHacker", "name=" + "%d" * 30, "bogus%s"]


class HackGameRunner:
    """protoHack automation: exploration, targeted commands, save/restore."""

    def __init__(self, binary_path: str, debug: bool = False, enhanced_mode: bool = False,
                 seed: Optional[int] = None):
        self.binary_path = binary_path
        self.hackdir = Path(binary_path).resolve().parent / "hackdir"
        self.debug = debug
        self.enhanced_mode = enhanced_mode
        self.seed = seed
        # Private RNG so a given seed always yields the same key sequence
        self.rng = random.Random(seed)
        self.child: Optional[pexpect.spawn] = None
        self.fail_reason = ""        # Why the last session reported failure
        self.resizes = 0             # Resize cycles attempted this session
        self.resizes_seen = 0        # ...that reached "TERMINAL TOO SMALL"
        self.depth = 1               # Deepest dungeon level reached
        self.events = {}             # Counts of targeted actions, for the log
        self.screen = pyte.Screen(COLS, ROWS)
        self.stream = pyte.ByteStream(self.screen)
        self.tail = bytearray()      # recent output, for sanitizer text
        self.last_output = time.monotonic()
        self.visits = {}
        self.on_stairs = None
        self.last_at = None
        self.stuck = 0

    def log(self, msg: str) -> None:
        if self.debug:
            print(f"[GAME] {msg}", file=sys.stderr)

    def _count(self, event: str) -> None:
        self.events[event] = self.events.get(event, 0) + 1

    # ---- session -----------------------------------------------------

    def run_session(self, steps: int = 200, player_name: str = "TestPlayer") -> Tuple[bool, int]:
        """Play `steps` keys (0.5x-1.5x in enhanced mode), then quit."""
        try:
            if self.child is None:
                self.child = pexpect.spawn(self.binary_path, timeout=10, dimensions=(ROWS, COLS))
            self._drain(0.5)                       # news screen
            self._send(" ")                        # --Hit space to continue--

            if self.enhanced_mode:
                move_count = self.rng.randint(max(1, steps // 2), max(1, steps * 3 // 2))
            else:
                move_count = steps
            self.log(f"Session: {move_count} keys (steps={steps}, seed={self.seed})")

            restores = 0
            for _ in range(move_count):
                if not self.child.isalive():
                    if not self._game_over_ok():
                        return False, self._exit_status()
                    self._new_game()
                    restores = 0
                    continue
                roll = self.rng.random()
                if self.enhanced_mode and roll < RESTART_CHANCE and restores < 3:
                    if not self._save_and_restore():
                        return False, self._exit_status()
                    restores += 1
                elif self.enhanced_mode and roll < RESTART_CHANCE + RESIZE_CHANCE:
                    if not self._resize_cycle():
                        self._quit()
                        return False, self._exit_status()
                else:
                    self._step()
                if self.child.isalive() and time.monotonic() - self.last_output > HANG_SECS:
                    self.fail_reason = f"hang: no output for {HANG_SECS}s while sending keys"
                    self.child.kill(signal.SIGABRT)    # ASan prints the stack on abort
                    return False, self._exit_status()

            if not self.child.isalive():
                return True, self._exit_status()
            return self._quit()
        except pexpect.EOF:
            self.log("Game exited during play")
            return True, self._exit_status()
        except Exception as e:
            self.fail_reason = f"exception: {e}"
            self.log(f"Session failed: {e}")
            if self.child and self.child.isalive():
                self.child.terminate(force=True)
            return False, -1

    def _quit(self) -> Tuple[bool, int]:
        """Q, y, then page through the score list until the game exits."""
        for attempt in range(8):
            self._send("\x1b\r ", settle=0.1)     # end any text prompt or pager
            self._send("Q", settle=0.2)
            if "Really quit" in self.screen.display[0]:
                break
            if not self.child.isalive():
                return True, self._exit_status()
        else:
            self.fail_reason = "no quit prompt after retries"
            self.child.terminate(force=True)
            return False, -2
        self._send("y", settle=0.3)
        for _ in range(10):
            if not self.child.isalive():
                break
            self._send(" ", settle=0.2)
        try:
            self.child.expect(pexpect.EOF, timeout=10)
        except pexpect.TIMEOUT:
            self.fail_reason = "game did not exit after quit"
            self.child.terminate(force=True)
            return False, -2
        return True, self._exit_status()

    def _exit_status(self) -> int:
        """Reap the game and return its exit status (signals: see signalstatus)."""
        if not self.child.closed:
            try:
                self.child.expect(pexpect.EOF, timeout=10)
            except (pexpect.TIMEOUT, pexpect.EOF):
                pass
            self.child.close(force=True)
        return self.child.exitstatus if self.child.exitstatus is not None else 0

    def _game_over_ok(self) -> bool:
        """After a game ends mid-session (death, escape): was it clean?"""
        status = self._exit_status()
        if self.child.signalstatus:
            self.fail_reason = f"game died with signal {self.child.signalstatus}"
            return False
        if status != 0:
            self.fail_reason = f"game exited {status} mid-session"
            return False
        if _SANITIZER_RE.search(self.tail):
            self.fail_reason = "sanitizer report on the terminal"
            return False
        return True

    def _respawn(self) -> None:
        """Start the game again with the same command, env and output log."""
        old = self.child
        self.child = pexpect.spawn(old.command, args=old.args[1:], env=old.env, cwd=old.cwd,
                                   timeout=10, dimensions=(ROWS, COLS))
        self.child.logfile_read = old.logfile_read
        self.screen.reset()
        self._drain(0.5)

    def _clean_hackdir(self) -> None:
        """Remove the finished game's save and level files before a new game."""
        for p in (self.hackdir / "save").glob("*"):
            if p.is_file():
                p.unlink()
        for p in self.hackdir.iterdir():
            if p.is_file() and (_LEVEL_FILE_RE.match(p.name) or p.name == "rtmp"):
                p.unlink()

    def _new_game(self) -> None:
        self._count("new-game")
        self._clean_hackdir()
        self._respawn()
        self._send(" ")
        self.visits.clear()

    def _save_and_restore(self) -> bool:
        """Save ('S') or hang up, then start again: the game must restore."""
        how = "save" if self.rng.random() < 0.5 else "hangup"
        self._count(how)
        self._send("\x1b\r", settle=0.1)          # not mid-prompt: 'S' must be a command
        if how == "save":
            self._send("S", settle=0.3)
        else:
            self._hangup()
        status = self._exit_status()
        if self.child.signalstatus:
            self.fail_reason = f"signal {self.child.signalstatus} after {how}"
            return False
        if not list((self.hackdir / "save").glob("*")):
            # The game may have ended first (e.g. the 'S' answered a prompt
            # and the player died): only a clean exit 0 is acceptable then.
            if status == 0 and self._game_over_ok():
                self._new_game()
                return True
            self.fail_reason = f"no save file after {how} (exit {status})"
            return False
        if status != 1:                           # save and hangup both exit(1)
            self.fail_reason = f"exit {status} after {how}, expected 1"
            return False
        self.tail.clear()                         # look only at the new game's output
        self._respawn()
        self._send(" ", settle=0.5)               # the restore message follows the news pager
        if b"Restoring" not in self.tail:
            self.fail_reason = f"did not restore after {how}"
            return False
        self.visits.clear()
        return True

    def _hangup(self) -> None:
        """A real hangup: SIGHUP, then the tty goes away.  The game's handler
        only sets a flag (SA_RESTART); it acts on the next read, which returns
        EOF once the pty master is closed."""
        self.child.kill(signal.SIGHUP)
        self.child.delayafterclose = 0.2
        self.child.close(force=False)
        end = time.monotonic() + 15
        while self.child.isalive() and time.monotonic() < end:
            time.sleep(0.1)
        if self.child.isalive():
            self.child.kill(signal.SIGKILL)
            self.child.wait()

    def _resize_cycle(self) -> bool:
        """Shrink the pty below 80x24 and restore it.  protoHack shows its own
        "TERMINAL TOO SMALL" screen and must redraw the map once the size is
        back (#13 fixed a stale screen when the size returns to 80x24).
        False if the warning is still up after the restore."""
        rows, cols = self.rng.randint(5, 23), self.rng.randint(20, 79)
        self.resizes += 1
        self.child.setwinsize(rows, cols)
        self._send("s", settle=0.3)               # a turn, so the size check runs
        if "TERMINAL TOO SMALL" in "\n".join(self.screen.display):
            self.resizes_seen += 1
        self.child.setwinsize(ROWS, COLS)
        self._drain(0.5)                          # the loop polls the size every 200 ms
        self.log(f"Resize to {cols}x{rows}: seen={self.resizes_seen}/{self.resizes}")
        if self.child.isalive() and "TERMINAL TOO SMALL" in "\n".join(self.screen.display):
            self.fail_reason = f"stale TERMINAL TOO SMALL screen after restoring 80x24 (from {cols}x{rows})"
            return False
        return True

    # ---- I/O ---------------------------------------------------------

    def _drain(self, settle: float = KEY_SETTLE, limit: float = 3.0) -> None:
        """Read until the game is quiet for `settle` seconds."""
        end = time.monotonic() + limit
        while time.monotonic() < end:
            try:
                data = self.child.read_nonblocking(65536, timeout=settle)
            except pexpect.TIMEOUT:
                return
            except pexpect.EOF:
                return
            self.stream.feed(data)
            self.tail += data
            del self.tail[:-16384]
            self.last_output = time.monotonic()

    def _send(self, keys: str, settle: float = KEY_SETTLE) -> None:
        if not self.child.isalive():
            return
        try:
            self.child.send(keys)
        except OSError:
            return
        self._drain(settle)

    # ---- choosing keys -------------------------------------------------

    def _step(self) -> None:
        if self._answer_prompt():
            return
        r = self.rng.random()
        if r < 0.70:
            if not self._explore():
                self._count("search")
                self._send("5s")                  # hunt for hidden doors
        elif r < 0.80:
            self._send(self.rng.choice("hjklyubn"))
        elif r < 0.88:
            self._send(self.rng.choice("HJKLYUBN"))    # run until something
        elif r < 0.90:
            self._count("ctrl-r")
            self._send("\x12")
        elif r < 0.915:
            self._count("option")
            self._send("o" + self.rng.choice(NASTY_TEXT) + "\r")
        elif r < 0.93:
            self._count("call")
            self._send("c" + self.rng.choice("abcdefgh") + self.rng.choice(NASTY_TEXT) + "\r")
        elif r < 0.97:
            self._send(self.rng.choice("iswetrqdpWPTRI<s"))   # assorted commands
        elif r < 0.98:
            self._send(self.rng.choice("0123456789") + self.rng.choice("hjklyubns"))
        else:
            self._send("\x1b")
        status = self.screen.display[ROWS - 1]
        m = re.match(r"Level\s+(\d+)", status)
        if m:
            self.depth = max(self.depth, int(m.group(1)))

    def _answer_prompt(self) -> bool:
        """Answer pagers and prompts so the game is never left waiting."""
        top = self.screen.display[0].rstrip()
        text = "\n".join(self.screen.display)
        if "--More--" in top or "Hit space" in text:
            self._send(" ")
            return True
        if "TERMINAL TOO SMALL" in text:
            # Only _resize_cycle shrinks the pty, and it restores the size
            # before returning; a warning still up here is stale.  Redraw
            # (^L) so play continues; _resize_cycle reports the failure.
            self._send("\x0c", settle=0.2)
            return True
        if top.endswith("?") or top.endswith(":"):
            if "Really quit" in top:
                self._send("n")
                return True
            r = self.rng.random()
            if r < 0.35:
                self._send("\x1b")
            elif r < 0.7:
                self._send(self.rng.choice("abcdefghijklmnopqrstuvwxyz*"))
            else:
                self._send(self.rng.choice(NASTY_TEXT) + "\r")
            return True
        return False

    def _explore(self) -> bool:
        """One move along a BFS path on the visible map: to '>' if seen, else
        to the nearest unexplored edge.  False if there is nowhere to go."""
        grid = [row.ljust(COLS) for row in self.screen.display[1:ROWS - 1]]
        at = None
        for y, row in enumerate(grid):
            x = row.find("@")
            if x >= 0:
                at = (x, y)
                break
        if at is None:
            return False
        self.visits[at] = self.visits.get(at, 0) + 1
        if self.on_stairs == at:
            self.on_stairs = None
            self._count("descend")
            self._send(">")
            return True

        def ch(x, y):
            return grid[y][x] if 0 <= y < len(grid) and 0 <= x < COLS else " "

        def goal(x, y):
            c = ch(x, y)
            if c == ">":
                return 2
            if c in "#+." and self.visits.get((x, y), 0) < 2 and any(
                    ch(x + dx, y + dy) == " " for dx, dy in MOVES):
                return 1
            return 0

        prev = {at: None}
        queue = [at]
        best = None
        for cur in queue:
            g = goal(*cur) if cur != at else 0
            if g == 2:
                best = cur
                break
            if g == 1 and best is None:
                best = cur                        # keep looking for '>' a while
            if best is not None and len(queue) > 400:
                break
            for dx, dy in MOVES:
                nxt = (cur[0] + dx, cur[1] + dy)
                if nxt in prev or ch(*nxt) in " |-":
                    continue
                if dx and dy and "+" in (ch(*cur), ch(*nxt)):
                    continue                      # no diagonal moves through doors
                prev[nxt] = cur
                queue.append(nxt)
        if best is None:
            return False
        if ch(*best) == ">":
            self.on_stairs = best
        node = best
        while prev[node] not in (at, None):
            node = prev[node]
        if node == at:
            return False
        self.stuck = self.stuck + 1 if self.last_at == at else 0
        self.last_at = at
        if self.stuck > 4:
            self.stuck = 0
            self.visits[best] = 99                # give up on this goal
            self._send(self.rng.choice("hjklyubn"))
            return True
        self._send(MOVES[(node[0] - at[0], node[1] - at[1])])
        return True

    def __del__(self):
        if self.child and self.child.isalive():
            try:
                self.child.terminate(force=True)
            except Exception:
                pass
