# 1.8.5-r6: five percent more boss attack damage

Implements the recommended bosses-only option accepted after the +5%
comparison. The existing per-hit encounter damage is multiplied by 105/100
with nearest-integer rounding before Weak, Vulnerable and Block. The shared
helper feeds attack resolution, intent text and preview calculations;
Paper Krane uses it too. Enemy HP, attack sequences and non-boss damage are
unchanged. Tiny hits may remain identical after rounding. No save layout or
in-game version change is needed.

`boss_damage.c` verifies full resolved turns against fixed expected values
for ordinary enemies, elites and bosses: basic attacks, Block, two/three-hit
attacks, Weak/Vulnerable and Paper Krane. It also checks defensive intents
and save/resume without double-applying the increase, and generates four
fixtures for the exact packaged EBOOT. The emulator uses those assisted
states and real end-turn controller input to assert actual HP losses.

Build with pinned PSPDEV v20261001 and `bash qa/v185/build.sh`. Run
`bash qa/v185r6/test.sh` for all previous sanitizer suites plus the new damage
checks. Run `qa/v185/qa_ppsspp.py` with `RIFT_BOSS_ONLY=1 RIFT_INSTANCE=185r6
RIFT_PREFIX=r6- RIFT_DIRECT=1 RIFT_FAST_PRESENTATION=1` and `RIFT_TOOLCHAIN`.
The separate cold-boot probe checks the exact EBOOT startup and stack.

## Campaign comparison

Same reference policy, 1,200 character/seed pairs, ordinary starter decks,
all unlocks available, victory at completion of act 3. Run the existing
`qa/v185r3/reference_campaigns.c` with `100 2000 1` and `300 10000 1`.
The implementation is identical to the preceding bosses-only +5% experiment.

- Current r5 comparison baseline: 283/1,200 (23.6%).
- Final r6: 252/1,200 (21.0%); zero stalls.
- Initial seeds: 55/300 (18.3%).
- Held-out seeds: 197/900 (21.9%).

The preceding isolated +5% experiment yielded 251/1,200 (20.9%). Small
run-to-run differences in the reference harness are observed; these are
approximate automated comparisons, not deterministic action replays or
human win-rate estimates. No policy changes or assisted kills/stats are
used in the campaigns. The separate emulator fixtures are assisted.

`evidence/final-tests.txt` ties results to the tested EBOOT digest. The
published complete EBOOT must match that digest; physical PSP hardware
confirmation of this exact binary remains outstanding.
