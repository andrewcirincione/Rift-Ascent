# 1.5b follow-up functionality audit

The expanded audit found and fixed two character-pool leaks: Forgotten Armory could offer the other character's cards, and the Warden's random-hand replacement could generate Nightshade or multiplayer cards. Both now filter the character pool. Existing saves remain compatible; previously acquired cross-character cards are preserved.

## Full runs on the corrected engine

3,000 full campaign simulations: 500 matched seeds per character for each of three policies. Real game rules, starting stats, rewards and HP are used. Every action validates the state. Checkpoints save, load, and compare the next action against an uninterrupted copy. All runs ended in victory or defeat: **2,507,972 actions and 89,562 save/resume comparisons, zero stalls, invalid states, or divergent continuations.** All newly acquired deck cards stayed in the correct character pool (shared curses excepted).

| Policy | Warden wins / 500 | Nightshade wins / 500 |
| --- | ---: | ---: |
| Balanced | 259 (51.8%) | 63 (12.6%) |
| Defensive | 292 (58.4%) | 87 (17.4%) |
| Aggressive | 318 (63.6%) | 123 (24.6%) |

Balance remains an open finding: Nightshade wins substantially less often under these policies. The policy retains detailed Warden card ratings and uses simpler Nightshade ratings; one-action lookahead also undervalues longer discard/Poison combinations. These are functional regression and calibration results, not human win rates or proof of equal balance. No balance values were changed in this correction.

An additional 3,000 runs on the pre-correction engine are retained under `before-fixes/`. They completed without stalls but did not include the new class-pool invariant; the targeted event test exposed the leak.

## Targeted integration checks

`parity.c` ran with AddressSanitizer and UndefinedBehaviorSanitizer:

- 20,400 mystery-event choices across all 17 events, both characters and all three options; resource-free picker cancellation, valid commits and class-correct cards.
- 1,000 shops: purchases, read-only deck browsing, three potion purchases without overwriting slots, full-belt rejection without payment, and map potion use.
- 990 shared-relic singles/pairs: equivalent Strike/Defend plays, all six potions and turn transitions produce matching HP, Block, Energy, Strength, Dexterity and enemy effects for both characters.
- 1,000 Warden random-hand replacements remain class-correct.
- 2,000 mixed-deck encounters / 160,000 actions exercising all 87 solo Nightshade cards, upgrades, eligible modifications, relic combinations, preview purity and valid effect resolution.
- Both characters' normal victory/menu/endless choices and 100 acts of controlled boss-defeat progression each. These are progression tests, not legitimate 100-act wins.
- All four Nightshade choice types: mandatory-choice shortcut guards, pause/return, and exact save/resume.
- Repeated the existing 15,952 independent damage/Block cases, 174 base/upgraded card resolutions, 18,000 reward draws, and legacy/current save migrations.

The corrected complete EBOOT is also tested in PPSSPP using real controller input and emulator screenshots: the established combat/control/save suite, both Armory choices and Warden hand replacement. `../evidence/ppsspp-final.txt` and `ppsspp-resume.txt` identify the exact binary SHA. Music payloads remain unchanged.

## Reproduce

From the repository root:

```bash
cc -O2 -I qa/v150b/source qa/v150b/audit_followup/extended_runs.c qa/v150b/source/game.c -lm -o /tmp/rift-runs
# runs, first seed, character (0 Warden / 1 Nightshade), policy (0/1/2)
/tmp/rift-runs 500 10001 1 0
cc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -I qa/v150b/source qa/v150b/audit_followup/parity.c -lm -o /tmp/rift-parity
ASAN_OPTIONS=detect_leaks=0 /tmp/rift-parity
```

All three policies use starts 10001, 11001 and 12001 respectively. LeakSanitizer is disabled because of the environment's process tracing; address/undefined-behavior checks remain enabled.
