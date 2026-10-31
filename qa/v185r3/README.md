# 1.8.5-r3: story outcomes, bargains and challenge balance

The in-game version remains 1.8.5. Baseline is commit
cdc4b32e848d5b989122b2b59d28996bffa18bae (the published 1.8.5-r2).
The canonical source remains `qa/v185/source`.

Forge is the reward for story IDs 5, 6, 15 and 16 in each character's
20-story bag, when a card is eligible. The existing 5% story trigger remains.
Other story rewards have Take and Leave only. An exhausted Forge falls back
to the story's normal gift; old pending gifts are preserved and their removed
third selection becomes Leave. Already-open Forge saves remain resumable.

Cinder Gambit now offers 150 Gold for 6 HP, or 180 Gold plus Injury.
Red Communion's offensive choice costs 3 HP and queues 3 Strength plus
1 Vulnerable. Candle for a Stranger costs 4 HP for its existing 2 Strength
and 10 starting Block. These boons still last for the next combat only.
Elites receive 8% more HP and bosses 10% more HP, rounded to the nearest
point. Normal encounters, damage values, opening intents and phase cycles
are unchanged.

## Campaign evidence

Both players use legal game actions with ordinary starter decks and all
unlocks available. They choose routes, cards, shops, rest, potions and events;
no combat stats or enemies are altered. A win means completing the first
three acts, not the optional seven-act ascent. These are heuristic bots,
not measurements of human win probability.

The established policy is preserved in `reference_campaigns.c`, adding only
handlers needed to navigate the new Forge outcome. This allows the new
requested 20–25% target to be compared with the previous benchmark.

| Campaign set | Runs per build | Old build wins | New build wins |
|---|---:|---:|---:|
| Initial seeds, 2001–2100 per character | 300 | 84 (28.0%) | 62 (20.7%) |
| Independent seeds, 10001–10300 per character | 900 | 276 (30.7%) | 220 (24.4%) |
| Combined | 1,200 | 360 (30.0%) | 282 (23.5%) |

No policy stalls occurred. The independent set was run after the candidate
was selected, without retuning to its seeds. Its 24.4% estimate has roughly
±2.8 percentage points of sampling uncertainty. By character, the holdout
has Warden 102/300, Vesper 88/300 and Proxy 30/300; the target is aggregate,
not a claim that every character or skill level has the same win rate.

The old inventory heuristic ignored card modifications, undervalued upgrades
and rewarded raw deck size. `campaigns.c` is a separate decision-aware player
that scores modifiers, upgrade improvements and average deck quality, and
discounts surplus Gold. On the initial 300 seeds it won 161/300 (53.7%) on
the old build and 139/300 (46.3%) on the new one. This exposes the substantial
policy dependence of the benchmark; we did not double enemy difficulty to
force this stronger policy down to 25%.

## Decisions, including consequences

The decision-aware player skipped old Cinder Gambit on all 74 visits. With
the new terms it chose blood 16 times, Injury once, and Leave 51 times in
68 visits. Red Communion's offensive bargain rose from 0 selections to 13;
Candle rose from 0/63 to 70/70 when affordable. These counts flag options
for investigation; they are not evidence on their own that every skipped
option needs a buff.

`paired_events.c` collected 60 naturally reached states for each of five
suspect events from 300 baseline campaigns, then replayed every affordable
choice on both versions using exactly the same serialized input states.
Each branch plays four more rooms (including act boundaries) or until death.
The account snapshot is restored between branches. The same initial RNG is
used; choices can cause legitimate later RNG divergence.

For the revised Cinder's 60 states:

| Choice | Survived four rooms | Mean ending HP | Mean ending Gold |
|---|---:|---:|---:|
| Pay 6 HP for 150 Gold | 60/60 | 65.3 | 344.0 |
| Take Injury and 180 Gold | 59/60 | 66.6 | 373.0 |
| Leave | 60/60 | 68.8 | 231.8 |

The blood bargain retains a health cost, but now funds purchases. Injury
pays more while retaining a deck penalty and risk. Leaving remains useful
when health is scarce or more Gold has little value. The old blood bargain
ended at 59.3 HP and 304.8 Gold, with 59/60 surviving. These short rollouts
support the change but do not establish full-run causal win-rate effects.

Living Altar was also skipped by the inventory score, yet its unchanged
max-HP choice survived 57/58 available new-build forks versus 56/60 for
leaving. Cursed Purse trades future HP for extra spending money. Those
findings did not justify automatically buffing either event. The broader
`decision_probe.c` also branches all affordable choices across all 42 events;
its CSVs retain the exploratory observations.

Limitations: both bots share the combat heuristic, lack sophisticated deck
synergy planning, and can evaluate simulated random offers. Neither is an
independent human playtester. The four-room tests are local decision probes,
not guarantees of later victory. Final build behavior is separately checked
in PPSSPP with explicitly assisted fixtures and actual controller input.

## Reproduction and release validation

Use `bash qa/v185r3/test.sh` for native ASan/UBSan regressions, and
`PSPDEV=/path/to/pspdev bash qa/v185/build.sh` for the full PSP package.
The PSPDEV pin remains v20261001. Existing startup stack auditing is mandatory.
Packaging writes and fsyncs a temporary file before replacing the EBOOT;
emulator drivers check the complete soundtrack footer and copied digest.
The first local candidate test caught a package truncated at 40 MiB; that
failed attempt is retained and was not used as successful release evidence.

Compile either campaign C file with `gcc -O2`. Arguments are runs per
character, seed offset, and `1` for full unlocks. For the old build, use
`-DBALANCE_SOURCE='"/path/to/baseline/qa/v185/source/game.c"'`.
Example candidate holdout: `reference_campaigns 300 10000 1`.

Compile `paired_events.c` against the baseline and run
`paired-old --collect /tmp/r3-event-states.bin`, then run each build with
`--replay /tmp/r3-event-states.bin`. CSV evidence is included. Compile
`challenges.c` against both versions to compare enemy HP, species, displayed
intent/damage and RNG over all seven acts.

For exact-EBOOT input testing, use `qa/v185/qa_ppsspp.py` with
`RIFT_INSTANCE=185r3 RIFT_PREFIX=r3- RIFT_DIRECT=1 RIFT_FAST_PRESENTATION=1`
and `RIFT_TOOLCHAIN` set to the prepared PPSSPP/PSPDEV/Xvfb environment.
Fixtures cover ordinary gifts, all Cinder choices, dedicated Forge actions,
elite/boss turns, three-target combat, inspection, save/quit/resume, and audio.
Run the six cold-boot scenarios through `qa/v185r2/boot_probe.py` with
`RIFT_BOOT_EVIDENCE` pointing here. This records fresh, current, stable-1.8.4,
beta-only, legacy SAVE.DAT and corrupt-save startup with observed stack use.

The full soundtrack and artwork are compared byte-for-byte with 1.8.4-r2.
The final release digest, native results and exact-EBOOT runtime checks are
in `evidence/final-tests.txt`. Physical PSP-3000 / ARK-5 confirmation remains
outstanding; emulator success alone does not establish hardware success.
