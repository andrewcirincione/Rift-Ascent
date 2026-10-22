# Rift Ascent — Gameplay Overhaul candidate

This is an original deckbuilding roguelike built for PSP homebrew. This overhaul is in a **separate** GitHub Actions workflow from the user's known-working PSP-3000 / 6.60 / ARK-5 flicker-free build, to allow a safe rollback.

## Changes
- Native CRT-free PSP EBOOT, preserving confirmed NEXTVSYNC / queue-before-wait presentation and user-mode firmware imports
- Improved animation: character idle motion, lunges/recoil, spark particles, floating damage/healing/block values, faded screen transitions and a visible enemy turn windup
- Pacing: card-action cooldown and a short enemy telegraph before turn resolution
- Combat controls: D-pad right to select END; Cross to confirm; Circle remains an optional shortcut
- START in combat opens a four-tab **Card / Foe / Status / Potions** encyclopedia; Triangle opens the selectable potion belt
- Enemy intentions include concise immediate telegraphs plus detailed descriptions (Weak, Vulnerable, Poison, block, multi-hit, curse and energy drain)
- Card inspector and on-card damage badges compute live attack strength, weakness and target vulnerability, including multi-hit bonuses
- Active player and enemy status conditions are displayed in combat
- Larger space for playable card text; tooltips retain descriptions without clipping the 480x272 bottom margin
- Title menu now uses Up/Down and Cross, not separate buttons per action
- Antialiased raster font generated **during build** from DejaVu Sans Condensed (no font file required at runtime)
- Deterministic 3-lane / 12-floor route graph: only defined connections are traversable, forks create real choices, node pools and probabilities give different risk / reward opportunities
- Longer procedural two-mode themes with act-specific register changes, low ambience and dedicated action SFX
- Saved run structure retains `GAME_VERSION=16` for existing saves

## Checks run
GitHub Actions uses the official PSPDEV compiler and user-mode import audit, native ASAN/UBSAN engine tests, PPSSPP 1.20.4 launch/title/combat screenshots, scripted menu-to-combat actions, map rendering and an enemy inspection-panel screenshot.

**Hardware note**: the previous NEXTVSYNC game was confirmed playable on physical PSP-3000 (6.60 ARK-5), but *this larger gameplay overhaul is a new EBOOT*. The screenshots and automated tests cannot guarantee it performs identically on the real handheld. Preserve the previous EBOOT as a backup and test only once when comfortable.

## Controls
| Button | Action |
|---|---|
| D-pad | Move through title choices, connected paths, cards, and help-panel tabs |
| Cross | Select action, play card, use potion, or select END turn |
| Circle | End turn directly (combat), back / close elsewhere |
| Start (combat) | Open / close Card, Foe, Status and Potion details |
| Triangle (combat) | Open potion belt to select a specific available potion |
| Up/Down (combat) | Select targeted enemy |
| Square | Show full deck |

The map uses valid **connected** paths rather than allowing every nearby route. The glossary makes unusual status/intent words understandable without memorization.
