# 1.8.6-r4 five-win access and one-time shared finale

Base: `31fec696da6a7c2da89aa7f45bc7774e917d6ce7` (1.8.6-r3). Canonical source remains `qa/v186/source`, mirrored in `GAME/src` and `portable_build`. In-game version remains 1.8.6; the new immutable release tag is `v1.8.6-r4`.

## Behavior

Secret Act 4 access and the character-selection golden A use the same five-Act-3-win predicate. The fifth victory can continue into Act 4 immediately. Earlier endings retain their resolution. Card collection masks do not substitute for wins. The unlock allocator is unchanged: two different-rarity character cards per victory through ten wins, twenty cards total. Act 4 victories do not increment that counter.

The account's existing three-bit Act 4 completion mask remains separate from the five-win badge. Only a transition from two distinct completed characters to all three prepares the new shared finale. It follows the third character's three-page individual ending, contains five pages and generated artwork, then returns to the existing Victory/endless choice. All six completion orders work. Custom runs cannot count toward this milestone. An older profile already at all three completions skips retroactive playback.

## Durability

`Game`, `AscentRun`, `AscentReceipt` and `AscentAccount` retain their exact layouts. The new screen constant is appended, preserving all older screen numbers. Two checksummed `ASCENT18_FINALE` generations store a separate 20-byte record: owning run, slot, character, state and page.

The prepared receipt is written and verified before the account commits the third first-clear. A failed account write leaves an inert prepared receipt; it becomes eligible only with the matching committed run receipt and all three completion bits. A power loss between account and run writes can retry the killing outcome without missing or duplicating the finale. The active record advances durably with each page. The done record prevents older scene snapshots or subsequent victories from starting it again. A partially watched finale resumes at its saved page. Write failures retain the current page and allow retry.

## Tests

`revision.c`, under ASan/UBSan, exercises all characters' first/fourth/fifth/sixth/tenth/eleventh Act 3 routes, actual distinct-rarity unlocks through ten, unchanged counts on repeat receipts, imported collection independence, older cached ending repair, all six Act 4 completion orders and all four slots, each finale page through cold reopen, stale standalone/run snapshots, repeated later wins, legacy all-three profiles and custom exclusion. Injected failures cover receipt preparation, account commitment, scene entry, page advancement and final completion. The previous reward and damage regression functions are retained.

`test.sh` also runs the existing seven card/combat/inventory/save suites and the adapted 1.8.6 content suite. `timeline.c` retains the full single-fade pixel regression and all twelve existing scene mappings; it checks the shared image and measured text fit on every finale page, and verifies the golden A is absent at four wins and present at five without needing an Act 4 victory.

`qa_ppsspp.py` uses the exact complete packaged EBOOT in PPSSPP 1.20.4 with a PSP-1000 memory profile. Explicit assisted combat fixtures supply valid card actions, account snapshots and legal near-win positions. Actual inputs resolve the fights and scene transitions. Functional checks may skip repeated finisher presentation after resolution; separate full-frame checks retain unmodified Normal/Fast timelines and record the individual-ending-to-shared-finale transition. Scene screenshots are read after the renderer returns, avoiding partial software backbuffers; every finale capture is compared pixel-for-pixel with the packed scene artwork. The gates phase checks fourth/fifth/tenth/eleventh victories for all characters and captures both badge states. The orders phase checks all six clear orders, each first/second/third ending, all five finale pages and later repeats. Two additional emulator processes cold-resume page three and then the completed Victory screen. No physical PSP test is claimed.

## Build and reproduce

```sh
PSPDEV=/path/to/pspdev bash qa/v186/build.sh
bash qa/v186r4/test.sh
gcc -O2 -std=gnu11 qa/v186r4/fixtures.c -o /tmp/r186r4-fixtures
/tmp/r186r4-fixtures
cp qa/v186/evidence/EBOOT.PBP qa/v186r4/evidence/EBOOT.PBP
RIFT_TOOLCHAIN=/path/to/toolchain RIFT_INSTANCE=186r4gate RIFT_PREFIX=gates- RIFT_DISPLAY=611 RIFT_PORT=19611 RIFT_DIRECT=1 RIFT_PHASE=gates python3 -u qa/v186r4/qa_ppsspp.py
RIFT_TOOLCHAIN=/path/to/toolchain RIFT_INSTANCE=186r4orders RIFT_PREFIX=orders- RIFT_DISPLAY=612 RIFT_PORT=19612 RIFT_DIRECT=1 RIFT_PHASE=orders python3 -u qa/v186r4/qa_ppsspp.py
# Preserve the orders instance's real save directory for the two cold boots.
RIFT_TOOLCHAIN=/path/to/toolchain RIFT_INSTANCE=186r4orders RIFT_PREFIX=resume- RIFT_DISPLAY=612 RIFT_PORT=19612 RIFT_DIRECT=1 RIFT_KEEP=1 RIFT_PHASE=resume python3 -u qa/v186r4/qa_ppsspp.py
RIFT_TOOLCHAIN=/path/to/toolchain RIFT_INSTANCE=186r4orders RIFT_PREFIX=done- RIFT_DISPLAY=612 RIFT_PORT=19612 RIFT_DIRECT=1 RIFT_KEEP=1 RIFT_PHASE=done python3 -u qa/v186r4/qa_ppsspp.py
RIFT_TOOLCHAIN=/path/to/toolchain RIFT_INSTANCE=186r4fade RIFT_PREFIX=fade- RIFT_DISPLAY=613 RIFT_PORT=19613 RIFT_DIRECT=1 RIFT_FADE_ONLY=1 python3 -u qa/v186r4/qa_ppsspp.py
```

Toolchain setup matches `qa/v186r2/README.md`. New art, references and the exact built-in generation prompt are retained under `art/` and `references/`. Packing appends one 336×126 RGB565 scene; existing artwork and soundtrack bytes are preserved. The release workflow pins PSPDEV v20261001 and Pillow 12.3.0 and requires the rebuilt complete EBOOT to match the playtested SHA-256 before pushing the binary to main and publishing.

## Final results

All nine game suites and the full renderer regression passed under ASan/UBSan. The exact EBOOT passed all twelve PSP gate cases, all six completion orders and their repeat victories, both cold restarts, six combat ending sequences and the shared-finale transition. All old artwork bytes and twelve complete soundtrack streams are preserved. Screenshots were visually reviewed; finale crops also match the packed pixels exactly.

Complete EBOOT: 99225495 bytes; SHA-256 `d144cea99ae30e0e7c571e06515454ff253b3a284450d69296472c00e9a847eb`. The public release workflow must reproduce this digest before publication.
