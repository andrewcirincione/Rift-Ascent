# Rift Ascent 1.3.9

This patch continues the delivered 1.3.8 build. It retains the original soundtrack and save layout.

## Changes

- Cinder Hound has a newly generated pale bone/ember silhouette that reads clearly against dark combat backgrounds.
- Combat advances four timeline units per rendered frame, versus three in 1.3.8. Card windup takes four frame intervals. Block uses one brief held guard pose and a short shield flash. All actual Block gains emit their animation event, including non-Defend cards and triggered effects.
- Player attack frames are anchored to the idle stance; all six measured foot anchors stay within 1.5 pixels of that stance. The horizontal lunge and pose-dependent vertical movement are removed. Transparent neighboring sprite fragments were removed during atlas extraction. Guard, Power and surrounding effects use the same character center. Incoming recoil stays small.
- PSP audio output is now full volume instead of half volume. Impact voices have stronger transients and bass; music sits lower and briefly ducks at contact. Low/medium/high hit samples peak at 15,410 / 21,445 / 29,554 without clipping in the isolated tests. Every observed contact dispatches once.
- A single surviving enemy has no target marker beneath it. Multiple living enemies retain selection markers.
- Killing blows hold the completed combat scene longer before rewards, independently of the faster attack. The player collapse still finishes before defeat.
- Upgrade, discard/exhaust and deck grids show a raised crimson card with a crest and dimmed alternatives. Shops use a newly generated crimson selection plaque. END uses a new bright ruby/amber plate when selected. The shield icon is newly generated with a small cyan glow.
- Moon Spring lets you adjust healing with Up/Down. The offer displays exact HP and Gold before accepting: 2 Gold per HP, maximum 20 HP, bounded by missing HP and affordable healing. Odd leftover Gold remains yours. No charge for unavailable healing.
- All 28 relic descriptions now specify trigger timing, scope and amounts. Weak or redundant effects have been redesigned or strengthened. Examples: Fury Censer rewards every third Attack played in a turn with Strength; Worn Chain rewards every third Skill with Dexterity; Resting Idol combines resting with an upgrade; Scavenger heals when a combat card reward is skipped. Last Ember restores 25% maximum HP once per act. Glass Emblem applies +5 to every hit of the first Attack card each turn, including multi-hit cards; playing a Skill does not consume it.
- Expect a Fight explicitly states that each Attack remaining in hand grants Energy, then further Energy gain is disabled for the current turn. Base and upgraded descriptions agree with the resolver.
- Enemy status applications, including a Vulnerable-only card, have a generated curse burst and the relevant status icon.
- Card rewards offer Square to browse the deck and Triangle to inspect rewards individually. Returning from either view does not take the reward and retains selection.
- Normal fights offer Common/Uncommon cards only (63/37). Elite card rolls use Common/Uncommon/Rare weights 50/40/10. Boss card rewards are Rare. Choices cannot repeat a card. Non-Rare rewards have 0/25/50% upgrade chance by act; Locket upgrades all offered combat cards. Shops and events retain a separate pool.
- Normal Act I enemies received a small increase over 1.3.8: HP scaling +10 percentage points and damage scaling +8 points. Later scaling was recalibrated around full inter-act healing and the new blessings.
- Every act begins with three different blessings; choose one. Acts II and III begin fully healed. Nine persistent blessings provide distinct offensive, defensive, draw, Energy, Gold and healing benefits. Choices save and resume correctly.
- The relic list and combat relic tab show complete selected descriptions; the combat list scrolls through all owned relics.

## References and deliberate differences

The researched STS2 reward system allows rare cards in normal fights. Rift Ascent intentionally follows the requested stricter rule: no normal-fight Rare rewards. Its elite/boss rarity weights, upgraded reward progression and act-entrance healing/choice are informed by these references:

- https://sts.wiki/guides/card-rewards
- https://sts.wiki/ancients
- https://sts.wiki/relics/shuriken
- https://sts.wiki/relics/kunai

The relic and blessing names/artwork remain Rift Ascent's. The blessing pool and custom relic effects are adaptations, not a claim of exact STS2 behavior.

## Validation

AddressSanitizer and UndefinedBehaviorSanitizer pass. Inherited regression coverage includes 39,608 arithmetic/resolution/RNG cases, 1,472 dynamic description layouts, all 184 card variants, 1,104 animation-observer comparisons, 2,000 planar maps and full 45-floor traversal. Added coverage includes 25,116 Moon Spring cases, 27,000 reward rarity rolls, all 28 relic effects, all nine blessings, save/resume, all-act healing and reward browsing. Block feedback is checked for 28 base/upgraded cards. Attack audio checks cover 37 damaging cards and skipped render frames.

Full-run simulations resolve the actual game rules with a heuristic player that drafts, shops, upgrades, uses potions and chooses routes. It evaluates resolved candidate actions, including draws, so its success rate is a calibration measure, not a universal human win rate. Calibration after the new healing/blessing systems: 354/1,000 wins (35.4%). An independent set of seeds: 377/1,000 wins (37.7%), zero stalls, with 30 / 168 / 425 losses in Acts I / II / III. Normal encounters remain predominantly solo.

PPSSPP 1.20.4 checks use the compiled release EBOOT, saved fixtures, real button input, state assertions, audio capture and PPSSPP's built-in screenshots. See `evidence/ppsspp-results.txt` for completed cases. This patch has not been tested on a physical PSP. Save version remains 18, with a 3,144-byte Game structure; existing 1.3.8 saves load. New blessing screens and changed relic behavior require this EBOOT.

## Build and test

From the repository root, with PSPDEV on PATH:

```sh
python3 qa/v139/build_source.py
make -C portable_build -j2
python3 qa/v115/import_114_art.py GAME/src portable_build/EBOOT.PBP
python3 tools/audit_psp_imports.py portable_build/EBOOT.PBP
python3 GAME/verify_eboot.py portable_build/EBOOT.PBP
python3 qa/v134/pack_music.py /path/to/complete-1.3.8-EBOOT.PBP portable_build/EBOOT.PBP /path/to/release/EBOOT.PBP
```

The source package deliberately excludes the private soundtrack itself. The final EBOOT preserves the three original tracks byte for byte.

```sh
mkdir -p ../evidence139
cc -O1 -g -fsanitize=address,undefined -I GAME/src qa/v139/qa.c GAME/src/game.c GAME/src/artwork.c GAME/src/enemy_art.c GAME/src/motion_art.c GAME/src/sound.c -lm -o ../evidence139/qa
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 ../evidence139/qa
cc -O1 -g -fsanitize=address,undefined -I GAME/src qa/v139/relic_tests.c -o ../evidence139/relic-tests
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 ../evidence139/relic-tests
cc -O2 -I GAME/src qa/v139/playtest.c GAME/src/game.c -o ../evidence139/playtest
../evidence139/playtest 1000 30001 0
```
