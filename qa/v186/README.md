# Rift Ascent 1.8.6

Canonical source: `qa/v186/source`. Base: `516750f438090d21658d18064de6a5a82e391ff6` (published 1.8.5-r6). Source is mirrored in `GAME/src` and `portable_build`.

## Changes

- Three-foe HP uses the same 9-point bold font as ordinary encounters. The player meter fills the actual 89-pixel aperture; its width never changes with max HP. Overlong HP values abbreviate within fixed bounds, including the top status line.
- Journey Forge uses the existing gilded focus panel, a two-pixel lift and a restrained shadow. No extra label or arrow. All three choices and the card picker retain deck/inventory access.
- The Hollow King has one Crown Tether. The Unbegotten has two staggered Unborn Witnesses. Leaders retain their HP; their damage multipliers are 96% and 98%, before the retained 1.8.5-r6 5% boss bonus. Minions have separate HP/damage shares. Surviving foes do not gain damage when a partner dies. Existing saved single-boss encounters remain valid.
- Approximately one in six elite rooms is a pair: Pyre Castellan/Censer Bailiff in Act 1, Abyssal Sacristan/Rime Pallbearer in Act 2, Eclipse Justicar/Vowbreaker Seraph in Act 3 and later celestial acts. These three new elite species and the boss minions never enter ordinary encounter pools. Five original transparent sprites accompany the new enemies.
- Temper rotates through six modifiers, instead of three. Primed puts an Attack, Skill or Power into the opening hand, subject to the hand limit; extra copies remain in the draw pile. A First Light Forge mystery offers Primed, Bound or Leave. Echo now also supports Defend, Shrug It Off, Impervious, Vesper's Defend, Backflip, Deflect, and Proxy's Defend. Their real effects and live previews repeat; payment and physical pile movement happen once. The story Forge still has exactly three choices and appears at the same dedicated outcomes/frequency.
- Repeat-run dialogue gradually reveals the three characters as unwilling anchors of a prison that resets their lives. Character-specific entries, selected random stories, boss conversations and the Act 4 Unmaker reveal different clues. First-run text and all cutscene artwork/page counts are unchanged. A separate checksummed, alternating memory receipt counts completed non-custom journeys once. Starting unused slots does not advance it. Existing saves import only their available win/retirement history.
- Game version 186 preserves the Game/account layouts. Because Primed uses the seventh raw modifier value, event actions move from 7–10 to 8–11. Standalone and alternating 1.8.5 saves migrate interrupted and paused event-card actions. Earlier 1.8.4/beta/legacy migration and the PSP stack fix remain covered.

## Balance evidence

These are fixed heuristic players making legal decisions with ordinary starter decks, rewards, shops, events and potions. All unlocks are available; no debug kills, stat boosts or selected winning seeds are used. They compare builds, not predict a human player's win rate.

Matched boss entries came from 6,000 natural baseline campaigns: 3,000 per policy. Each candidate replays exactly the same deck, HP and RNG state at the boss door. Policies are unchanged from `qa/v185r3`.

| Policy / encounter | 1.8.5-r6 | 1.8.6 | Change |
| --- | ---: | ---: | ---: |
| Reference, Act 3 (1,108 entries) | 57.76% | 57.76% | 0.00 points |
| Stronger, Act 3 (1,782 entries) | 75.81% | 75.31% | -0.50 points |
| Reference, Act 7 (567 entries) | 18.87% | 18.69% | -0.18 points |
| Stronger, Act 7 (1,228 entries) | 28.09% | 30.29% | +2.20 points |

Complete Act 1–3 campaigns use three disjoint seed ranges, 400 seeds per character per range (3,600 runs per build):

| Seeds | 1.8.5-r6 wins | 1.8.6 wins |
| --- | ---: | ---: |
| 2001–2400 | 247 / 1200 (20.6%) | 252 / 1200 (21.0%) |
| 5001–5400 | 269 / 1200 (22.4%) | 221 / 1200 (18.4%) |
| 9001–9400 | 266 / 1200 (22.2%) | 248 / 1200 (20.7%) |
| Total | 782 / 3600 (21.7%) | 721 / 3600 (20.0%) |

No policy stalls occurred. The second seed block was lower; the aggregate is at the lower edge of the requested 20–25% range. No settings were changed to fit those later blocks. Encounter tuning trials, final CSVs, and compressed pre-boss snapshots are retained under `evidence/`.

## Build and checks

Use PSPDEV `v20261001`, Python 3.12 and Pillow `12.3.0`:

```sh
PSPDEV=/path/to/pspdev bash qa/v186/build.sh
bash qa/v186/test.sh
```

The build extracts the established artwork and twelve complete music streams from the pinned `versions/1.8.4-r2/EBOOT.PBP`, appends six new images, and verifies checksums. It also imports the established XMB/title/hero artwork from the historical inputs used by prior releases. `check_media.py` checks every old artwork byte, all music bytes, new assets, title and package size. `check_stack.py` audits the compiler's startup/migration/save frames against the unchanged 192 KiB thread allocation.

Native ASan/UBSan suites cover 2,340 card/upgrade/character cases against three foes, attack intent versus actual damage, old and new Forge flows, all Echo defensive Skills on all three characters (including foreign cards), Primed overflow, event migrations, run history, boss/minion completion and 28 inventory screens. Native fixture generation validates every assisted state used for screenshots.

PPSSPP 1.20.4 runs the exact packaged EBOOT with the PSP-1000 memory model. `qa_ppsspp.py` uses debugger-assisted state fixtures for targeted UI and controller checks; those are separate from the legal balance campaigns. It checks actual card effects, target selection, enemy turns, leader/minion deaths, Forge deck access, save/quit/resume, potion effects, audio and streamed-art errors. `boot_probe.py` observes actual MIPS stack use without changing game state during fresh/current/old/beta/legacy/corrupt cold boots. Screenshots are native 480×272 and were visually inspected. Emulator checks do not replace testing on physical PSP hardware.

To reproduce boss comparisons, compile `tests/boss_replay.c` with `BALANCE_SOURCE` pointing at the desired `game.c`, and optionally `STRONG_POLICY`. Decompress the matching `boss-entry-*.bin.gz`, then run `replay <file>`. `capture <file> 1000 30000` generates reference states; the stronger policy used offset 40000. Full campaign CSVs are reproduced by `qa/v185r3/reference_campaigns.c` with `BALANCE_SOURCE`, arguments `400 <offset> 1`, offsets 2000, 5000, 9000.
