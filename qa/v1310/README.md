# Rift Ascent 1.3.10

Continues the delivered 1.3.9 build. Existing save layout (version 18, 3,144-byte Game) and all three original soundtrack tracks remain intact.

## Changes

- Replaced the pale fiery wolf with a newly generated charcoal hellhound: blackened cathedral armor, restrained embers and silver silhouette lighting.
- Replaced the bright END button with a newly generated black stone/gunmetal plaque. Selection raises the plate, brightens its text and shows a matching directional pointer.
- Setup Strike has its own generated 240×135 source illustration, distinct from Strike. Card trim remains above the illustration.
- Corrected Scorch Witch, Storm Golem and Crimson Judge facing. Enemy rendering crops unused source margins, making most silhouettes approximately 25–35% taller inside the battlefield. The player, including attack/guard/Power/death frames, is about 6% smaller; all six attack anchors remain within 1.5 pixels of the idle stance.
- Replaced the hand's text chevrons with generated 36×28 gothic arrows. They appear only when additional cards exist in that direction.
- The Ancient Lesson event reveals the exact upgraded card before returning to the map. Continue does not add it twice, and the reveal saves/resumes.
- Replaced the combat turn/move log with one written intent row per living enemy. Damage and secondary effects are explicit: Weak, Vulnerable, Poison, Energy drain, Burn/Daze/Dread placement, healing, Strength and Block. Damage uses current modifiers and is before player Block. Existing intent icons remain below enemy HP.
- Strength potions are named Strength. The selected potion's use closes the inventory and displays an explicit result, such as “Strength +2 (now 2)”. The status icon, flame particles and card descriptions update immediately. The underlying Strength addition was working in the audited resolver; the old inventory presentation obscured its result.
- Combat potion drops now have a persistent named reward notice, retained while browsing cards/deck and across save/resume. Shop purchases identify the potion received.
- Block digits have an opaque dark stroke against the cyan shield, including three-digit values.
- Fixed Whirlwind's X handling: Energy expenditure and hit count are separate. Free/Unrelenting plays use the current X without consuming Energy. One-Two Punch replays the original hit count instead of replaying at zero Energy. Cascade also retains its X effect when made free.
- Fixed an enemy intent preview rounding mismatch: Colossus and Tank are now applied before player Vulnerable, matching actual incoming damage resolution.

## Damage audit

Outgoing attacks retain the established per-hit order: add Strength to base damage and clamp at zero; apply Weak (75%, rounded down); apply target Vulnerable (150%, rounded down); apply Cruelty if the target is Vulnerable; then add applicable flat relic bonuses. Enemy Block absorbs the resulting damage separately for every hit. Vulnerable/Weak stack counts affect duration, not multiplier strength, except cards that explicitly scale with stack count. Effects such as Setup Strike apply their Strength after their own hit.

Whirlwind example: base 5 + Strength 4 = 9; Weak rounds this down to 6; a Vulnerable target takes 9 per hit. At X=3 this is 27 against that target and 18 against a target without Vulnerable, before Block. The upgraded version begins with base 8. Glass Emblem applies to every hit of the first Attack card each turn; a repeated card is a subsequent play and does not receive the bonus again.

Added tests use an independent card-by-card arithmetic oracle, not only comparisons with the preview implementation:

- 51,072 cases across all 38 Attack cards, both upgrades, positive/negative Strength, Weak, each target's Vulnerable, Cruelty, Glass Emblem, Block and conditional branches.
- 30,240 Whirlwind cases, X=0–6, both upgrades, free plays, Unrelenting and One-Two Punch.
- 348 Strength potion cases across all three slots, Violet Orb, negative Strength and temporary Strength expiration.
- 1,568 enemy-intent comparisons against real resolution across all 14 intent types.
- Rendered-description cache invalidation after potion use, plus all 504 enemy-name/intent combinations fitting their text rows.
- Event reveal/save/resume and potion reward notification persistence.

Inherited sanitised regression checks also pass: 39,608 arithmetic/RNG cases, all 184 card variants, 1,472 dynamic text layouts, 1,104 animation-observer comparisons, 2,000 maps, 45-floor progression, 25,116 partial-healing cases, 27,000 rarity rolls, all 28 relics and nine blessings, and attack/Block audio feedback.

A fresh 1,000-run simulation completed 377 runs (37.7%), with zero stalls. It uses actual rules and a heuristic that evaluates resolved actions, including draws; this is a calibration result, not a measured human win rate. No balance parameters changed in this patch.

## PPSSPP validation

PPSSPP 1.20.4 runs the compiled EBOOT. Tests use saved scenarios, real controller input, memory assertions and the emulator's built-in screenshots. `evidence/ppsspp-results.txt` records the completed checks; `evidence/ppsspp-final.txt` records the final combat screenshots and rendered potion-preview verification. Native screenshots are kept separate from PPSSPP screenshots. No physical PSP was available for this patch.

The potion test uses Triangle → Up → Cross to choose the middle Strength potion, verifies Strength 0 → 2, consumes exactly that slot, checks the inventory closes and captures the visible result. Whirlwind tests verify six contacts (ordinary) and twelve contacts (One-Two Punch), exact HP/Block changes on both enemies, and corresponding audio dispatches.

## Build

Current canonical sources: `qa/v1310/source`. Runtime copies: `GAME/src` and `portable_build`.

Set PSPDEV to the PSP SDK root, then run from the repository root:

```sh
bash qa/v1310/build_release.sh /path/to/complete-1.3.9-EBOOT.PBP /path/to/release/EBOOT.PBP
```

The script stops if compilation or an audit fails. The private soundtrack is absent from this source package; the supplied baseline EBOOT preserves it byte for byte in the release.

```sh
mkdir -p ../evidence1310
cc -O1 -g -fsanitize=address,undefined -I GAME/src qa/v1310/qa.c GAME/src/game.c GAME/src/artwork.c GAME/src/enemy_art.c GAME/src/motion_art.c GAME/src/sound.c -lm -o ../evidence1310/qa
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 ../evidence1310/qa
cc -O1 -g -fsanitize=address,undefined -I GAME/src qa/v1310/relic_tests.c -o ../evidence1310/relic-tests
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 ../evidence1310/relic-tests
cc -O2 -I GAME/src qa/v1310/playtest.c GAME/src/game.c -o ../evidence1310/playtest
../evidence1310/playtest 1000 40001 0
```

`patch_rules.py` and `patch_visual.py` document the one-time transition from 1.3.9; do not rerun them against already patched sources. `build_source.py` is the repeatable source/asset synchronization step. Generated originals, converted assets and provenance are included under `qa/v1310`.
