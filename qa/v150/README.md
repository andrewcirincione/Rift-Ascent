# Rift Ascent 1.5

Canonical implementation: `source/`. `build_source.py` synchronizes `GAME/src` and `portable_build`; `prepare_art.py` extracts and packs the original generated assets. Existing 1.4.0 art and music are preserved.

## Build and test

```
PSPDEV=/path/to/pspdev bash qa/v150/build_release.sh versions/1.4.0/EBOOT.PBP qa/v150/evidence/EBOOT.PBP
python3 qa/v150/verify_media.py
bash qa/v150/run_native.sh
python3 qa/v150/qa_ppsspp.py
RIFT_CONTROL_AUDIT=start python3 qa/v150/qa_ppsspp.py
RIFT_CONTROL_AUDIT=resume python3 qa/v150/qa_ppsspp.py
```

The release uses PSP GCC 15.2.0 and PPSSPP 1.20.4. Native checks use GCC AddressSanitizer and UndefinedBehaviorSanitizer. The exact complete EBOOT is played in PPSSPP and captured with PPSSPP's own native 480x272 screenshot function. Physical PSP hardware was not available.

The PPSSPP harness expects the emulator, PSP SDK, Xvfb, xdotool and PulseAudio under the sibling `runtime` directory, as resolved at the top of `qa_ppsspp.py`; adjust those paths to your installation. Run native presentation checks first to regenerate the deterministic fixtures. Frozen v19 migration fixtures are included under `fixtures/` and require no previous build. `build_source.py` must run before renderer tests so the runtime mirrors match the canonical source.

- `mechanics_tests.c`: all eligible modified base/upgraded cards against real resolution, independent modifier arithmetic, Echo/One-Two Punch, physical pile conservation, Retain across turns/saves, modified Headbutt, all new/old relic pairs and all 40 relics together.
- `events_tests.c`: all 34 offers, cancellation and transaction timing, insufficient resources, legal card targets, deck/belt limits, named relic alternatives, copy semantics and Moon Spring partial payment.
- `expansion_tests.c`: all new intents with independent Block/Weak/Vulnerable arithmetic, all four new cards, Covenant caps and once-only turn-end resolution, natural spawning of eight new enemies, legacy save conversion.
- `regression.c`: existing damage/Block/Whirlwind/potion/animation/status/endless/map/save regressions, expanded to new Attack cards, 57-floor campaigns and 792 enemy-name/intent combinations.
- Existing dedicated relic, potion belt, rarity, offer variety, navigation, Bully and Ashen Strike regressions are retained under this directory.
- `map_choice_tests.c`: 40,000 maps and 160,000 forks; every fork differs within two floors, planar paths and stable saves. One or two shops per lane; a final campfire before the boss.
- `scaling_tests.c`: every species across 1,000 acts, every floor and both group sizes. Health and damage continue growing; health grows faster and damage slower than 1.4.0.
- `presentation_tests.c`: all event choices, modification pickers and results, modification seals, relic details, new enemies/cards/intents and extended map fixtures. Modified card text fits the card interior.
- `mod_visual_tests.c`: all 2,965 static/live modified layouts, full modification clauses, separate seal/energy/title/body regions, and six mixed-hand fixtures. The longest preview is 157 pixels and the longest description is 141 characters.
- `audit_controls.py` / `control_tests.c`: the actual release PSP button decoder, 73,728 physical button combinations, 6,000 shortcut sequences and 9,380 save/resume boundaries. Confirming changes one selected card and charges once; navigation, unsupported shortcuts and cancellation cannot change possessions. Frozen saves and current pickers round-trip safely.
- `qa_ppsspp.py`: real controller actions against the packaged PSP executable, plus deterministic injected starting states for focused reproduction. These are targeted runtime checks; injected fixtures are not represented as naturally completed runs.
- `playtest.c`: deterministic full-run heuristic players using actual rules, without HP/reward/deck cheats. Results are calibration evidence, not a human win-rate prediction. Three independent policies/seed ranges are recorded.

## Design and persistence

Six modifications occupy one slot in the card's raw value, independent of the upgrade bit. Pile moves, shuffles and upgrades retain them; duplication copies them. Echo replays the card effect without inventing another physical card, including Exhaust interactions. It stacks additively with One-Two Punch. Ward applies only to the card's own Block effects; reactive powers and relics do not inherit it. Fleeting and Kindled Exhaust once. Card-selection effects are excluded from Echo/Kindled where a second unresolved choice would be ambiguous; X-cost cards are excluded from the fixed-cost Echo/Fleeting offers.

Costs are charged only when the final card choice is confirmed. Event offers are determined once on room entry and serialized; save/load and cancellation cannot reroll them. An unaffordable or ineligible choice does nothing. HP prices cannot kill the player. Every mystery room offers Leave, including an immediate Circle shortcut.

Four new relics use separate counters in previously unused combat state slots. They reset at combat/turn boundaries and coexist with the 36 existing unique relics. The legacy duplicate Compass remains retired.

Save version 20 adds four map rows (3260-byte run state versus 3196 bytes in v19). `save_legacy.inc` freezes the old layout and migrates versions 16–19 by field offsets. Existing cards, items, visited rooms, profiles and combats are preserved. The extra map rows are generated deterministically. A boss already in progress completes normally; a map save can proceed through the appended rooms.

All generated art originals and exact prompts are in `art/`; derived runtime sprites are in `assets/`. New enemies use their complete connected components rather than rigid atlas crops, preserving long weapons. Their idle and combat transforms keep feet grounded and remain below the HUD.

## Balance calibration

Final-source simulations completed 900 normal runs without stalls: 154/300 wins with the standard policy, 166/300 with the defensive policy and 200/300 with the aggressive policy. The policies use different seed ranges and actual game rules, including rewards, shops, events and card choices. They can evaluate candidate plays by copying game state, so these results should not be treated as human completion rates. The 1.4.0 standard-policy comparison was 122/300 on the same initial seed range; the longer map and expanded choices also increase opportunities to improve a deck.

Health multipliers before floor/group adjustments are 94%, 132%, 188% in Acts I–III, then +50 percentage points per act for normal/elite enemies. Bosses use 108%, 132%, 182%, then +44 points. Damage uses 82%, 99%, 124%, then +24 points. The existing Act I normal-combat bonuses remain. Non-boss health additionally grows with depth. Independent tests compare every species, floor and group size across 1,000 acts against 1.4.0 and check monotonic growth.

`ppsspp-candidate.txt` records an earlier candidate. Only the binary SHA in `ppsspp-final.txt`, `media.txt` and `release/current.json` identifies the published build. Screenshot capture waits for completed game frames, including double-buffer swaps, instead of assuming a wall-clock delay guarantees a fresh screen.

The additional control audit preserved two diagnostic logs. Its first harness incorrectly expected `sceKernelExitGame` to close the desktop emulator process, although the game saved correctly and returned PPSSPP to its menu; the corrected harness checks the successful PSP save trace. Its initial resume expectation also assumed the final starter-deck card could take Ward: that card is Bash, and the game correctly refused it without charging. The final audit explicitly checks that rejection, moves to Defend, confirms exactly once, and verifies read-only deck browsing afterward. `ppsspp-controls.txt` records the focused Save & Quit/relaunch retest; `ppsspp-controls-candidate.txt` also contains the successful live Keen/Fleeting, held-confirmation and shortcut checks. These harness corrections required no engine or EBOOT changes.
