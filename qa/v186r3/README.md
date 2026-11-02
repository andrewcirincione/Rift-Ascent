# 1.8.6-r3 progression, journey gifts and Act 3 resolution

Base release: `c30b3cc9d54f44e86cbb0c591c702dbc6162654c` (1.8.6-r2). Canonical source remains `qa/v186/source`, mirrored in `GAME/src` and `portable_build`. In-game version 1.8.6; immutable release tag `v1.8.6-r3`.

## Changes

- Above the existing act curve, enemy HP and damage gain one additional percentage point per act after Act 1, capped at +20%. The existing 5% boss damage bonus and group shares remain. All displayed intents use the same calculation as actual damage.
- From Act 4 onward, returning early-act enemies meet the Act 3 base-stat floor: normal HP 78/attack 16, elite HP 163/attack 19, boss HP 300/attack 23. Species above the floor are unchanged. Dedicated elite partners and boss minions retain their individual profiles. This addresses weak old species diluting the later pools without removing their variety.
- Story card gifts retain their original Act 1 quality. In Act 2, half the room hashes promote rarity and one quarter upgrade the card. Act 3 story cards are at least Rare, half upgraded; Act 4 onward cards are upgraded. From Act 5, a growing 25–50% chance requests an unlocked Ancient, falling back to Rare when none is available. The offered raw card is saved, previewed and granted unchanged.
- Later story and relic mystery rewards favor unowned relics with dependable combat effects. Act 3 adds Severed Psalm to that pool. Full inventories use the existing gold fallback; no duplicate relic is granted.
- Journey Forge still appears in exactly four of twenty story outcomes and still offers three choices. From Act 2, Temper selects Keen, Ward, Bound or Primed, avoiding the Exhaust tradeoff. Echo's maximum-HP fee decreases 4/3/2/1/0 across Acts 1–5, with its +1 Energy cost clearly retained. Mystery-room modification prices also ease: Gold falls 10% per act, capped at 40%; HP/max-HP fees fall by one per act, capped at four steps and a minimum of one. Quote, eligibility and commitment share one price function. All other mystery costs are unchanged.
- Selected card mysteries gain later rarity/upgrades; foreign rare offers gain upgrades and remain distinct. Their labels describe the actual offered rarity and upgrade. No additional choices or story spawns are added.
- Act 3 now ends in an explicit character-specific aftermath, with a `THE CROWN FALLS` heading and resolution text. The legacy Vesper/Proxy ending art slots show the Unmaker confrontation; normal wins now use the existing return-to-the-city aftermath artwork, keeping the confrontation out of early endings. Later runs may recognize past victories without introducing the secret boss. The separate revelation requires ten recorded Act 3 wins **for that character**, including the just-completed tenth win. A complete imported card collection alone cannot unlock it. Earlier pre-boss dialogue avoids directing players to the maker before that threshold.
- Old saved early revelations and cached flags are repaired on load or ending input, including paused and standalone saves. Already-active later-act runs are preserved. No save/account layout change. The 1.8.6-r2 single-fade correction remains intact.

## Campaign evidence

The unchanged reference policy ran 3,600 ordinary Act 1–3 campaigns with all unlocks, using the same three 400-seed-per-character blocks as the previous release. There are no assisted kills, inflated combat stats, selected winning seeds or stalled runs. These heuristic comparisons are not predictions of human win rates.

| Result | 1.8.6-r2 | 1.8.6-r3 |
| --- | ---: | ---: |
| Act 1 losses / entrants | 401 / 3600 (11.1%) | 401 / 3600 (11.1%) |
| Act 2 losses / entrants | 901 / 3199 (28.2%) | 877 / 3199 (27.4%) |
| Act 3 losses / entrants | 1577 / 2298 (68.6%) | 1579 / 2322 (68.0%) |
| Completed Act 3 | 721 / 3600 (20.0%) | 743 / 3600 (20.6%) |

Loss rates increase by act for each of the three characters as well as the aggregate. Warden wins 349/1200, Vesper 306/1200, Proxy 88/1200; the fixed heuristic is substantially weaker with Proxy. Stronger rewards offset the small stat increase, leaving overall completion within the requested 20–25% band. Results and seeds are in `standard-*.csv` and `balance-summary.json`.

A separate matched 600-run extended check follows legal runs through Act 7, using an already-unlocked profile. Baseline/candidate Act 3 wins were 112/116; Act 7 wins were 17/20. Later entrants are a small, increasingly selected set of strong decks: their conditional loss rates are **not strictly monotonic** (candidate Acts 4–7: 7.8%, 20.6%, 8.2%, 74.4%). The stat progression increases, but a slight adjustment cannot guarantee monotonically increasing observed loss rates through optional/endless acts. We retained the requested gentle adjustment instead of imposing a large post-victory difficulty spike. Intermediate and final extended results are retained, rather than hiding that limitation.

## Verification and reproduction

`revision.c` exercises 33 actual Act 3 victories across all characters; first/ninth/tenth/eleventh gates; independent card unlocks; idempotent receipts; old, paused and standalone ending repair; secret-boss resolution; 10,500 story-card gifts; relic exclusion and full inventory; every act's Forge preview/cancel/save/payment; all mystery modifier prices; 126 displayed-versus-actual damage/save cases; and returning-enemy stat floors. All run under AddressSanitizer and UndefinedBehaviorSanitizer.

`test.sh` also reruns the existing card, three-enemy, inventory, migration, boss bonus and event suites. `prior186.c` adapts only the previous lore expectations to the explicit tenth-win gate. `timeline.c` extends the full 1.8.6-r2 native frame regression with independent checks of all twelve character/scene image mappings. Ordinary endings must use resolution art; the secret confrontation must remain in its own scene. The earlier negative-control evidence is retained in the r2 folder.

PPSSPP 1.20.4 uses the exact complete EBOOT with PSP-1000 memory settings. Fixtures are explicitly assisted positions with separate account snapshots; real controller actions resolve wins, purchases and reward claims. Functional route/cost tests skip repeated presentation after resolution. A separate `RIFT_FADE_ONLY=1` run captures every completed PSP framebuffer at Normal/Fast speed for Act 3 leader-last/minion-last and Act 4/7 controls, without skipping the transition. Contact sheets and normalized frame-sequence videos support visual review, not wall-clock performance claims. Physical PSP testing is not performed here.

```sh
PSPDEV=/path/to/pspdev bash qa/v186/build.sh
bash qa/v186r3/test.sh
gcc -O2 -std=gnu11 qa/v186r3/fixtures.c -o /tmp/r186r3-fixtures
/tmp/r186r3-fixtures
cp qa/v186/evidence/EBOOT.PBP qa/v186r3/evidence/EBOOT.PBP
RIFT_TOOLCHAIN=/path/to/toolchain RIFT_INSTANCE=186r3 RIFT_DISPLAY=601 RIFT_PORT=19601 RIFT_DIRECT=1 python3 -u qa/v186r3/qa_ppsspp.py
RIFT_TOOLCHAIN=/path/to/toolchain RIFT_INSTANCE=186r3fade RIFT_PREFIX=fade- RIFT_DISPLAY=602 RIFT_PORT=19602 RIFT_DIRECT=1 RIFT_FADE_ONLY=1 python3 -u qa/v186r3/qa_ppsspp.py
```

The emulator toolchain layout/dependencies match `qa/v186r2/README.md`. Standard campaign reproduction uses `qa/v185r3/reference_campaigns.c` with `BALANCE_SOURCE` pointing to the chosen `game.c`, arguments `400 <offset> 1` and offsets 2000, 5000, 9000. Extended reproduction uses `qa/v186r3/campaigns.c` with arguments `200 12000`. To extract the baseline, archive `qa/v186/source` from the base commit before compiling the harness against it.

Complete tested EBOOT: 99,136,823 bytes; SHA-256 `4681c18951baee174be95e0fa7467714237287d06333429b036c6a8036549ab1`. All twelve full soundtrack streams and the artwork bank are unchanged. Stack checks retain the 192 KiB allocation.
