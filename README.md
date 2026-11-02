# Rift Ascent — distribution edition

A free PSP deckbuilding adventure about three people caught in a repeating ascent.
This branch contains the distribution ruleset. The development edition remains on
`main`; its decks and saves are separate.

## Install

Download the PSP ZIP from the `v1.8.6-dist2` release. Extract it to your memory
stick so the executable is `PSP/GAME/RIFTASCENT_DIST/EBOOT.PBP`. It includes the
complete soundtrack. The distribution edition starts a separate profile and
never imports development saves.

Warden earns Echo by using guarding Skills while threatened. Vesper earns it by
alternating Attacks and Skills. Proxy gains it from unused Energy at the next
turn. Echo persists between turns, is capped at nine, and can be released by
cards for additional effects. Wither damages its bearer and decreases by two
at each trigger. The in-game STATUS and card inspector explain current effects.

Five wins with a character open the secret boss and award the golden A. Cards
continue unlocking through ten wins. Clearing the secret boss with all three
characters reveals the shared finale once.

## Source and build

Use a shallow checkout to obtain this branch's current snapshot:

```sh
git clone --depth 1 --single-branch --branch distribution https://github.com/andrewcirincione/Rift-Ascent.git
cd Rift-Ascent
PSPDEV=/path/to/pspdev bash distribution/build.sh
bash distribution/test.sh
```

The toolchain is PSPDEV `v20261001`. The source is `qa/v186/source`; the complete
build is `distribution/evidence/EBOOT.PBP`. Declarative card and relic catalogs,
original rule generators, simulator source, validation records, and the unchanged
media banks are under `distribution/`. The media bootstrap checks a pinned digest
if banks are missing; released source archives include the banks directly.

`python3 distribution/tools/run_campaigns.py` repeats the large deterministic
balance study. `--quick` runs a smoke sample. Native simulations are comparative
balance probes, not estimates of every human player's win rate. Emulator fixtures
are explicitly assisted setups for checking real executable behavior and scene
routing. See `distribution/BALANCE.md` and `distribution/VALIDATION.md`.

## Distribution scope

This project is free of charge. It is independent and has no affiliation with
Mega Crit. See [the detailed IP review](distribution/LEGAL_REVIEW.md) for the
differentiation work, title-use collision and unresolved clearance questions.
This prerelease is not a legal clearance certificate for a broad public launch.
Use this edition's release downloads for sharing: the repository's older branches,
commits, and releases retain historical material and were not erased.

This notice does not grant rights in another party's work or certify legal
clearance. Existing component notices remain applicable. No new open-source
license or commercial permission is being asserted on the owner's behalf.
