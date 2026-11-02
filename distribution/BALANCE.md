# Distribution balance validation

Final compiled ruleset: `b4b7250dc2a97ce9172fdaa5e98c88a0de5aedf08919f4de8989129c07f783e0`.

## Scope and limits

14,700 complete seeded simulations use the real game engine, ordinary starting decks, legal combat inputs, potions, rewards, route choices, shops, story events and forges. There are no debug kills or stat boosts. The first 11,700 runs compare the final tuning on fixed seeds; an additional 3,000 previously unused seeds were evaluated after tuning ended. Every run terminated in victory or defeat; no action-limit stalls occurred.

These are heuristic simulated players, not human playtests or a guarantee of player win rate. Combat lookahead considers cards already known in hand; the reward policy is approximate and favors some effects. Wilson intervals describe the seeded sample only; character results sharing seeds and multiple policies are not independent human observations. Late-act endless balance beyond the secret-boss route was not estimated.

## Outcomes

| Cohort | Runs | Wins | Win rate | 95% Wilson interval |
|---|---:|---:|---:|---:|
| Fresh profile, two fixed-seed batches | 6,000 | 1,330 | 22.2% | 21.1–23.2% |
| All progression unlocked | 3,000 | 746 | 24.9% | 23.4–26.4% |
| More defensive policy, fresh profile | 1,800 | 476 | 26.4% | 24.5–28.5% |
| Standard Act 3 aggregate | 10,800 | 2,552 | 23.6% | 22.8–24.4% |
| Independent fresh-seed holdout | 3,000 | 657 | 21.9% | 20.5–23.4% |
| All standard Act 3 samples | 13,800 | 3,209 | 23.3% | 22.6–24.0% |
| Full secret-boss route, five prior wins | 900 | 225 | 25.0% | 22.3–27.9% |

The secret-route rate includes losses before reaching the secret boss; it is not the conditional win rate of that boss encounter. Unlocks raise expected power. The defensive policy is slightly above the 20–25% target, while the broad standard sample and independent holdout remain within it.

| Character | Standard runs including holdout | Wins | Win rate | 95% Wilson interval |
|---|---:|---:|---:|---:|
| Warden | 4,600 | 1,149 | 25.0% | 23.7–26.2% |
| Vesper | 4,600 | 1,089 | 23.7% | 22.5–24.9% |
| Proxy | 4,600 | 971 | 21.1% | 20.0–22.3% |

## Act progression

| Act | Runs reaching act | Defeats within act | Conditional defeat rate |
|---|---:|---:|---:|
| 1 | 13,800 | 192 | 1.4% |
| 2 | 13,608 | 6,174 | 45.4% |
| 3 | 7,434 | 4,225 | 56.8% |

Losses increase through the three standard acts. Act 2 remains the largest difficulty step for this policy; these data do not establish a uniformly gentle human difficulty curve. Enemy formations and the existing boss/minion tuning were preserved rather than retuned around one automated strategy.

## Decisions changed after playtesting

Early candidates won too often (roughly 36–53% by character in the initial sample). Immediate card output, the strongest burst attacks and turn-economy loops were reduced. Starter Guard/damage, bounded Echo income, limited per-turn power triggers and Wither decay were then tested together. Later copies of template families trade immediate output for deferred defense rather than being strict numerical upgrades.

A subsequent 11,700-run batch exposed ten reward cards with zero selections under the standard policies. They were given distinct setup benefits or lower-cost roles, then the same seeds were repeated. Starter Silt Silencer was not inflated merely because the policy declines additional copies: it already sees substantial use in its starting deck.

| Revised reward | Offered | Picked | Selection rate | Plays |
|---|---:|---:|---:|---:|
| Whisper Reckoning | 1,923 | 118 | 6.1% | 2,867 |
| Sextant Tally | 2,013 | 22 | 1.1% | 416 |
| Wanderer Witness | 5,967 | 60 | 1.0% | 1,067 |
| Quiet Witness | 1,948 | 129 | 6.6% | 2,479 |
| Orbit Witness | 1,959 | 317 | 16.2% | 5,596 |
| Whisper Measure | 2,000 | 9 | 0.5% | 252 |
| Chime Witness | 1,686 | 489 | 29.0% | 8,621 |
| Sundial Measure | 1,454 | 513 | 35.3% | 7,994 |
| Cinder Census | 488 | 137 | 28.1% | 3,283 |
| Choir Refuge | 483 | 131 | 27.1% | 1,585 |

Selection rates are not intended to be equal. Some cards remain situational and rarely selected by this policy. Pick rates compare reward offers only; play counts also include starter, shop and event acquisitions. A card appearing in winning decks does not by itself prove that it caused those wins. Strong direct-damage/Wither and banked-defense cards remain frequent choices; no universal must-pick claim is made.

## Reproduction

Run `python3 distribution/tools/run_campaigns.py` for the 11,700 fixed-seed regressions, then `python3 distribution/tools/run_campaigns.py --holdout` for the 3,000 holdout runs. `--quick` is only a CI smoke sample. Raw per-run and per-card CSVs, cohort summaries, and source fingerprints are in `distribution/evidence/`. The recorded four-file rules fingerprint covers game.c plus the distribution catalog and card/relic engine includes; the release SHA-256 identifies the full compiled product.
