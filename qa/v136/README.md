# Rift Ascent 1.3.6

Start opens **Status / Inventory** on the PLAYER tab. It lists current player
effects, active powers, and a scrollable played-powers history for this combat.
ENEMIES shows both enemies' current effects, HP, Block, and intent. Negative
Strength, Weak, Vulnerable, Poison, and temporary Mangle reductions are included.
The previous character-side status icons remain intact.

Hand descriptions now show current damage and Block. The CARD tab provides
per-enemy damage for attacks affecting different targets. Random target/exhaust
outcomes show ranges; unresolved draw/autoplay chains say "variable" instead of
claiming a known result. The extra damage badge above card artwork is removed.
Played-card animations preserve the pre-play description and upgrade state.

## Calculation contract

The preview resolves a copied Game using the actual card resolver, observing
direct card hits and Block gains. It does not mutate the live Game or RNG.
Descriptions are cached until combat state or target changes. Reactive power
damage (for example Inferno and Juggernaut) remains a separate effect, rather
than being misleadingly counted as damage from the Attack itself.

Attack damage per hit:

1. Resolve card-specific base damage and effects preceding the hit (including
   Rupture from self-inflicted HP loss).
2. Add Strength; clamp at zero.
3. If Weak, multiply by 3/4 and round down.
4. If the target is Vulnerable, multiply by 3/2 and round down; then apply
   Cruelty's additional percentage and round down again.
5. Add applicable Glass Emblem and Hollow Skull bonuses.
6. The displayed number is before enemy Block. Combat absorbs Block separately
   on each hit, then subtracts remaining damage from HP.

Direct Block adds Dexterity and Fortress Pin, clamps at zero, applies an unused
Unmovable doubling, and respects the 999 Block cap. Subsequent gains consume the
updated state, so Evil Eye and Second Wind can display different gains on their
first and later triggers. Passive powers retain their own actual resolution
rules; they do not universally inherit Dexterity.

Repeated attacks use the changed state after the first play. This includes
Vulnerable from Bash, Strength from Setup Strike, Bloodstone healing that can
disable Hollow Skull, and zero-hit replays after spending X Energy or exhausting
the hand. Random values are shown per hit, not as a guaranteed sum.

The audit also corrects three resolver issues:

- Thrash adds the exhausted Attack's base damage; it no longer applies that
  card's Strength/Weak/Vulnerable modifiers and then applies them again.
- First-Attack relics use the Attack counter, so playing a Skill or Power first
  no longer consumes their bonus.
- Hemokinesis does not hit after its self-damage has killed the player.

## Validation

- 39,608 arithmetic, actual-resolution, and RNG invariance cases under Address
  Sanitizer and Undefined Behavior Sanitizer, including all 184 base/upgraded
  variants. An independent rational-arithmetic oracle checks 7,040 modifier
  combinations; separate checks cover negative Dexterity, Unmovable and caps.
- 1,472 dynamic layout cases retain full descriptions; observed preview heights
  remain 100–149 pixels. All 184 static descriptions also fit completely.
- All playable Powers record their names, and played history survives saves.
  Save format remains version 18 / 3,144 bytes; a 1.3.5 save loads successfully.
- Existing 2,000-map, 45-floor campaign, reward/skip, shop/deck/potion, menu,
  character-status, animation, and audio regressions pass.
- PPSSPP 1.20.4 tested the final executable using controller inputs and native
  screenshot capture. Start opens PLAYER; scrolling reaches played powers;
  ENEMIES lists both foes; CARD displays current values; close works.
- Actual PSP resolution: 11 damage through 4 Block loses 7 HP; Defend with
  Dexterity 2 and Unmovable gains 14 Block; repeated Bash hits for 8 then 12;
  Thrash resolves within its 13–42 per-hit range; AoE deals 8 before Block to
  the Vulnerable/Cruelty target and 5 to the other target.
- EBOOT structure and user-mode imports pass. All three original soundtrack
  tracks are preserved byte for byte. Screenshots are from the final EBOOT.

## Build and test

With the existing PSPSDK on PATH:

```sh
python3 qa/v136/build_source.py
rm -f portable_build/PARAM.SFO
make -C portable_build -j2
python3 qa/v115/import_114_art.py GAME/src portable_build/EBOOT.PBP
python3 tools/audit_psp_imports.py portable_build/EBOOT.PBP
python3 GAME/verify_eboot.py portable_build/EBOOT.PBP
python3 qa/v134/pack_music.py /path/to/previous/full/EBOOT.PBP portable_build/EBOOT.PBP EBOOT.PBP
```

Delete the cached PARAM.SFO before building a new patch title.

```sh
mkdir -p ../evidence136
cc -O1 -g -fsanitize=address,undefined -I GAME/src qa/v136/qa.c GAME/src/game.c GAME/src/artwork.c GAME/src/enemy_art.c GAME/src/motion_art.c GAME/src/sound.c -o ../evidence136/qa
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 ../evidence136/qa
```

The test includes render.c; do not link it a second time. `qa_ppsspp.py` uses
the existing runtime directory (PPSSPP AppDir, PSPSDK, Xvfb/PulseAudio/xdotool)
and the fixtures generated by the native tests. Set `RIFT_WORKSPACE` to the
parent of this repository if it differs from the default. Runtime setup is
shared with the previous patches.

Source files and tests for this patch are in `qa/v136/source` and `qa/v136`.
Generated artwork remains in the previous patch directories. Played-power
history uses previously unused combat-state slots, preserving the save ABI;
older in-progress saves retain all active effects, while name history starts
with Powers played after this update.
