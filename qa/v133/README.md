# Rift Ascent 1.3.3

This patch fixes combat previews and readability, replaces the menu buttons, card frames, hand tray and energy orb with newly generated Gothic artwork, removes the enemy sprite overlays, and expands each act to 15 floors.

## Changes

- Selected and played cards stay over their position in the hand, including scrolling ten-card hands. Card paintings occupy the full illustration area. The redundant total-damage banner is removed.
- Health and block displays move together above overlapping previews. Enemy intents sit below the fight log. End Turn no longer has the obsolete illuminated potion squares.
- All four UI treatments are embedded bitmap artwork. The blue energy orb turns grey at zero. The discard animation uses the same card artwork and frame.
- Card rules, including Headbutt and its upgrade, state their complete effects. Corrected the swapped Iron Wave+/Rupture+ text and missing Exhaust wording on Impervious+/Offering+.
- Removed the procedural circles, sigils and strokes from all derived enemy sprites at their generator. Wound Collector faces the player. Fixed signed recoil arithmetic that could throw the hero off-screen while taking damage.
- Status scrolls through active powers, buffs and debuffs. Store Square opens the current deck; Triangle opens current potions. Browsing does not buy, remove or consume anything.
- Relics from treasure, events, elites and bosses can be taken or skipped. Relic stat effects apply only after taking the relic.
- Removed the Armory Spoils explanation and map preview-only labels/notifications.
- Each act has 14 rooms and a floor-15 boss. Routes have no crossing diagonals or dead ends. Shops and mid-act rests occupy staggered floors; each lane has one shop and two rests, with fights between consecutive mystery/shop/rest nodes.
- Enemy health increases about 20%; base attack scaling increases from 85% to 100% of the roster value (about 18% more, before modifiers and rounding).
- Sound selection happens after resolving the action; attack and damage sounds are no longer overwritten by block, transition or cursor sounds in the same action. Cursor feedback cannot cut a fresh impact sound short.
- Version 16 and 17 saves migrate to version 18. Existing stats, card piles, enemies and powers are retained. Lifetime distance now supports all 45 floors.

## Rebuild

Use a fresh checkout. `GAME/` and `portable_build/` are generated directories, recreated by this command:

```sh
python3 qa/v133/build_source.py
make -C portable_build -j2
python3 qa/v115/import_114_art.py GAME/src portable_build/EBOOT.PBP
python3 tools/audit_psp_imports.py portable_build/EBOOT.PBP
python3 GAME/verify_eboot.py portable_build/EBOOT.PBP
```

Dependencies: Pillow, NumPy, DejaVu fonts, PSPSDK. The workflow pins pspdev v20261001 (GCC 15.2.0). `source/` contains the final game, renderer and PSP frontend. `assets/` contains four new UI bitmaps plus the exact 103 approved card/background paintings recovered and SHA-256 checked against the previous user build. `regenerate_enemies.py` recreates the clean 36-enemy roster.

Public GitHub builds contain sound effects only. The delivered private EBOOT preserves the previous build's three original music tracks byte-for-byte. Music is deliberately excluded from this repository.

## Validation

`qa.c` checks 2,000 seeds for planar routes and utility spacing, the 45-floor campaign, relic take/skip and cursed-relic side effects, store browsing/removal, descriptions, powers, save round trips and damage PCM. It renders 480×272 fixtures including all 36 enemies and the discard animation. Run with AddressSanitizer and UndefinedBehaviorSanitizer; set `UBSAN_OPTIONS=halt_on_error=1`.

`save_compat.c` loads actual legacy-layout map/combat fixtures. `test_full_roster.c` exercises the full card roster; its obsolete synthetic old-save layout was replaced by those authentic fixtures. Existing choice, card-audit, upgrade and 1.2.1 regressions were also checked.

PPSSPP testing uses the release EBOOT, debugger controller input and PPSSPP's built-in Screenshot function. Targeted saves seed rare encounters and inventory states; those checks are distinct from the automated 45-floor engine traversal. Physical PSP hardware is not available in this environment.
