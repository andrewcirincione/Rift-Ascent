# Rift Ascent 1.3.7

This patch adds combat animation and impact feedback to the 1.3.6 game. The existing card, menu, status and dynamic-description designs are retained.

## Changes

- Generated full-body knight poses for anticipation, sword attacks, guarding/parrying, and playing Powers. The accepted atlas has 12 unique poses arranged in 18 animation slots with consistent scale and anchors.
- Enemies anticipate attacks, lunge, recoil and briefly flash on contact. Their existing character art and facing are preserved. Enemy guard actions materialize an ornate blue shield.
- New generated impact fragments, shield materialization, a winged ruby Power crest, and organic fire sprites are embedded in the EBOOT.
- Strength adds two fire particles per point, from zero at 0 Strength to 40 at 20 Strength. Higher Strength still affects damage normally; only the visual intensity is capped. Flames stay inside the battlefield, clear of the health bars and hand.
- Individual hits use low (1–10), medium (11–20), or high (21+) impact. The calculation uses resolved damage after Strength/Weak/Vulnerable and other modifiers, before Block absorbs it. Contact size, recoil, sound weight and duration scale with the tier.
- The battlefield framebuffer holds for 1, 3 or 6 rendered frames at contact. The HUD, hand, audio and controller polling continue. Multi-hit attacks schedule separate contacts; area attacks share a beat across targets.
- Four independent impact voices preserve overlapping hits. Guard sounds and the new Power chime have separate voices. Original music is preserved byte for byte.
- Played cards still rise to the center, then fade quickly during contact so attacks remain visible. Normal and boss finishing blows, and lethal enemy hits, finish their presentation before the next screen appears.
- Save format and combat rules are unchanged from 1.3.6. Existing SAVE.DAT and PROFILE.DAT remain compatible.

## Validation

The final source was run with AddressSanitizer and UndefinedBehaviorSanitizer. Tests cover 39,608 arithmetic/resolution/RNG cases, 1,472 dynamic description layouts, all 184 base/upgraded card variants and 1,104 comparisons between combat with and without the animation observer. They also verify tier boundaries, blocked hits, repeated hits, player Powers, enemy attacks and guards, exact framebuffer holds, HUD isolation, the Strength particle cap, final-blow transitions, and independent overlapping audio. Audio energy and decay duration increase with impact tier without clipping the test output.

The native visual run renders complete action sequences, Strength levels 0/5/10/15/20, finishing blows, all enemy sprites, all card previews, status panels, and menus. Generated player poses were inspected and the first atlas was rejected because swords crossed cell boundaries. The accepted atlas is extracted at a shared scale without limb clipping. Visual revisions also moved flames clear of the cape, reduced played-card opacity at contact, and removed the screen-transition darkening that obscured finishing hits.

The release EBOOT is tested in PPSSPP 1.20.4 using deterministic saved combat fixtures, real button input, actual game-state checks, and PPSSPP's built-in PNG screenshot function. See `evidence/qa-ppsspp-results.txt` for completed cases. These are emulator tests, not a claim of physical PSP testing.

## Build

From the repository root, with PSPSDK on PATH:

```sh
python3 qa/v137/build_source.py
make -C portable_build -j2
python3 qa/v115/import_114_art.py GAME/src portable_build/EBOOT.PBP
python3 tools/audit_psp_imports.py portable_build/EBOOT.PBP
python3 GAME/verify_eboot.py portable_build/EBOOT.PBP
python3 qa/v134/pack_music.py /path/to/music-bearing-EBOOT.PBP portable_build/EBOOT.PBP /path/to/release/EBOOT.PBP
```

Use the delivered release EBOOT or a prior 1.3.6 music-bearing EBOOT as the soundtrack source. Source archives do not redistribute the private soundtrack separately. The Makefile explicitly tracks the new include/header dependencies to prevent stale renderer objects.

Native QA (the output directory must exist):

```sh
cc -O1 -g -fsanitize=address,undefined -I GAME/src qa/v137/qa.c GAME/src/game.c GAME/src/artwork.c GAME/src/enemy_art.c GAME/src/motion_art.c GAME/src/sound.c -o ../evidence137/qa
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 ../evidence137/qa
```

Artwork provenance and accepted original atlases are included in this folder. The source archive contains the assets and extraction scripts needed to reproduce the embedded art.
