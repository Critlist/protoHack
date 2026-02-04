# protoHack (Static Binary)

This archive contains a prebuilt static Linux binary.

## Run

```sh
./run-hack.sh
```

## Files

- `hack-root` — the game binary
- `mklev` — level generator (invoked by hack-root)
- `hackdir/` — runtime data (news, moves, record, perm, save/)
- `run-hack.sh` — launcher

## Notes

If you see permission errors, make sure `hackdir/` and `hackdir/save/` are writable.

## License

This source code was created primarily by Jay Fenlason, with additional
contributions from Kenny Woodland, Mike Thome, and Jon Payne. It is being
shared here under a CC-BY-NC-SA 4.0 license, as that is the closest modern
license to the original distribution license.
