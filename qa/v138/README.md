# Rift Ascent 1.3.8

## Changes

- Combat presentation advances three timeline ticks per rendered frame. Card windup resolves after six frame intervals; attack, Block, Power, discard and draw sequences are approximately three times faster. Idle breathing and Strength flames keep their calm original pace.
- Raised-sword poses use the available space below the HP strip, fixing the clipped sword tip. Incoming player recoil is limited to a few pixels.
- Every recorded contact triggers its sound exactly once, even if rendering skips ahead. Low hits are much louder, with a dry crunchy transient, a stronger bass body for larger hits, four overlapping voices, and brief music ducking. Original music is preserved byte for byte.
- Spoils selection has a generated ruby-wing crest, a raised selected card and dimmed alternatives. The map has a generated helmet pawn marking the current room, a CURRENT location readout, and a separate upcoming-room cursor.
- Maps retain 15 floors per act. Routes have two starts, three seed-dependent forks, two or three active lanes, sparse merges, no crossings and no dead ends. Shops, events, rest sites and treasure are separated by encounters along every route.
- Normal multi-enemy encounters are less common, with no consecutive-floor pairs. The first two floors of Act I are solo fights; elite and boss encounters remain solo. Group members have lower health than equivalent solo enemies.
- Early enemy damage/HP scaling is gentler; later acts and floors become harder. Difficulty was calibrated using full-run heuristic simulations described below.
- A new six-pose player collapse finishes and holds the fallen pose before the defeat screen, including deaths without an attack animation trace.
- A new silver/ivory player health frame differs from the enemy frames. The main menu has a generated cathedral scene and new sculpted button plates.
- Card art extends toward the trim with a fine picture outline. Titles use serif lettering; long hand titles fit in two lines, and description fonts/content remain intact.
- Player status lists each played Power once under POWERS, with its current effect. Ordinary statuses and temporary Skill effects remain above that section.

## Validation

The final native build passed AddressSanitizer and UndefinedBehaviorSanitizer checks: 39,608 arithmetic/resolution/RNG cases, 1,472 dynamic description layouts, all 184 card variants, 1,104 observer-invariance comparisons, 2,000 planar route graphs, save compatibility and a 45-floor campaign traversal. The added audio regression checks 37 attack cards for exactly-once contact sounds after skipped frames. Timing, music ducking, no-trace death holds and sword clearance also pass.

Full-run simulations use the actual game engine for all 45 floors. They draft cards, use potions, shop, upgrade, choose routes and resolve combat without changing HP, rewards or RNG. The heuristic evaluates resolved candidate actions, including draws, so this measures this policy rather than a universal human win rate. Calibration: 105/300 wins (35.0%). Separate seeds: 331/1,000 wins (33.1%), zero stalls. Defensive and aggressive policies on 300 separate-seed runs scored 39.7% and 35.3%. Act I deaths were 37/1,000 in the main held-out sample, with most losses in Act III. Multi-enemy encounters represented 10.8%, 16.5% and 20.5% of all encounters by act.

PPSSPP 1.20.4 testing uses the release EBOOT, deterministic save fixtures, real button input, memory assertions and PPSSPP's built-in PNG screenshots. See evidence for completed cases. No physical PSP test is claimed. Save ABI remains version 18, with a 3,144-byte Game structure; SAVE.DAT and PROFILE.DAT are retained.

## Reproduce

With PSPDEV on PATH, from the repository root:

```sh
python3 qa/v138/build_source.py
make -C portable_build -j2
python3 qa/v115/import_114_art.py GAME/src portable_build/EBOOT.PBP
python3 tools/audit_psp_imports.py portable_build/EBOOT.PBP
python3 GAME/verify_eboot.py portable_build/EBOOT.PBP
python3 qa/v134/pack_music.py /path/to/complete-1.3.7-EBOOT.PBP portable_build/EBOOT.PBP /path/to/release/EBOOT.PBP
```

Native QA:

```sh
mkdir -p ../evidence138
cc -O1 -g -fsanitize=address,undefined -I GAME/src qa/v138/qa.c GAME/src/game.c GAME/src/artwork.c GAME/src/enemy_art.c GAME/src/motion_art.c GAME/src/sound.c -o ../evidence138/qa
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 ../evidence138/qa
cc -O2 -I GAME/src qa/v138/playtest.c GAME/src/game.c -o ../evidence138/playtest
../evidence138/playtest 1000 10001 0
```

Generated art originals, extraction scripts, font license and provenance are included. Existing generated combat effects and card trim are retained from 1.3.7. The official Mega Crit map screenshot informed route readability: https://megacrit.com/news/2024-11-07-neowsletter-issue-4/
