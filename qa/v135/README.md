# Rift Ascent 1.3.5

- Removes Strength/Dexterity text beneath health. Generated icons restore Strength, Weak, Vulnerable and Poison beside the player and every enemy; Dexterity and energy drain appear beside the player too. Stack/turn counts and negative values remain visible. Zero values are hidden. The full power/status list remains in the inventory inspector.
- Restores Cross, Circle, Square and Start icons in lower control hints. Square still opens the deck; Start opens the combat inventory.
- Measures card title and description content to size the hand preview. All 184 base/upgraded variants retain the complete description and the same 29-pixel painting height. Previews range from 100 to 149 pixels tall instead of 166. Typography and card width are retained.
- Bold white enemy-intent numbers have a small black shadow for contrast over combat scenery, including multi-hit values.
- Card ascent, centered resolution and transparent fade use the compact layout too.

Build with the existing PSPSDK:

```
python3 qa/v135/build_source.py
make -C portable_build -j2
python3 qa/v115/import_114_art.py GAME/src portable_build/EBOOT.PBP
python3 tools/audit_psp_imports.py portable_build/EBOOT.PBP
python3 GAME/verify_eboot.py portable_build/EBOOT.PBP
python3 qa/v134/pack_music.py /path/to/previous/full/EBOOT.PBP portable_build/EBOOT.PBP EBOOT.PBP
```

Native regression checks:

```
mkdir -p ../evidence135
cc -O1 -g -fsanitize=address,undefined -I GAME/src qa/v135/qa.c GAME/src/game.c GAME/src/artwork.c GAME/src/enemy_art.c GAME/src/motion_art.c GAME/src/sound.c -o ../evidence135/qa
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 ../evidence135/qa
```

The QA executable includes render.c to inspect measured text layout; do not link render.c again. Runtime screenshots and controller testing use PPSSPP 1.20.4 at native 480x272. The reusable driver expects the runtime setup used for 1.3.4 (PSPSDK, PPSSPP AppDir, Xvfb and PulseAudio dependencies).

Game rules, save format 18, and the three original music tracks are unchanged. The image-generation prompt and processed asset paths are recorded in asset-provenance.json; the original generated sheet is retained in icons-original.png.
