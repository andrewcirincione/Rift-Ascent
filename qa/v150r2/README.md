# Rift Ascent 1.5 preview-height correction

The in-game version remains 1.5. The corrected executable is published separately as `v1.5-r2`; the original `v1.5` release is preserved.

Expanded hand previews already measured their wrapped descriptions, but a long modified description could still grow into the intent HUD when the insufficient-Energy notice was included. The layout now reduces illustration height only when needed, retaining at least 24 pixels for the modification seal. If large live numbers still require another line, the preview gains a small amount of width instead of becoming taller. Text sizes and complete descriptions are retained. Positioning remains relative to the selected card. Played cards use the same layout and keep their flight below the HUD.

The current card set stays between 100 and 162 pixels high in 6,280 static/live layout cases. The widest stress case is 136 pixels, versus the normal 126. The audit includes every base/upgraded card and eligible modification, conditional effects, repeat effects, 128 exhausted cards, large Strength/Dexterity/Vulnerable values, Unmovable, 1,000 Energy and insufficient Energy. All wrapped description lines fit inside the frame. A further 2,800 rendered hand and animation frames preserve every pixel above the battlefield, across all ten hand positions.

Native AddressSanitizer/UndefinedBehaviorSanitizer regressions also pass. The exact complete EBOOT is tested in PPSSPP with its native screenshot function: short, long, modified, extreme-number and insufficient-Energy previews, actual left/right navigation, and a long card's real play/Exhaust behavior. Starting fixtures are injected for focused testing. Physical PSP hardware was not available.

Canonical source is `source/`; existing generated art, fonts, sound effects, soundtrack and gameplay rules are retained from `qa/v150`. The initial failing height audit is preserved as diagnostic evidence.

From the repository root:

```sh
PSPDEV=/path/to/pspdev bash qa/v150r2/build_release.sh versions/1.5/EBOOT.PBP qa/v150r2/evidence/EBOOT.PBP
python3 qa/v150r2/verify_media.py
gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -I GAME/src qa/v150r2/preview_height_tests.c GAME/src/game.c GAME/src/artwork.c GAME/src/motion_art.c GAME/src/enemy_art.c GAME/src/sound.c -lm -o /tmp/preview-height
ASAN_OPTIONS=detect_leaks=0 /tmp/preview-height
python3 qa/v150r2/qa_ppsspp.py
```

The PPSSPP script expects the same sibling `runtime` installation documented in `qa/v150/README.md`. Native checks regenerate its fixtures. No additional game data is required beside the complete EBOOT.
