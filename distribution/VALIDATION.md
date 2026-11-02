# Distribution validation record

Target: `v1.8.6-dist2`, in-game version 1.8.6, PSPDEV `v20261001`.
The full executable includes the unchanged artwork and complete soundtrack.

SHA-256: `f42eff279f0f1754b176a34d5d7ba70a3bb584fc9e6126ceb39397799bc951b1` (99103503 bytes).

## Native checks

All native suites run with AddressSanitizer and UndefinedBehaviorSanitizer.

- 58,008 card cases: every playable slot, base/upgrade, eligible forge modifiers,
  one/two/three enemies and 0/3/6/9 Echo. Preview does not change live state or
  account progression; preview damage/Guard matches resolution. Replay, Exile,
  pile bounds, resource caps and all four burden payments are checked.
- Character resource generation, every relic trigger path, and live Echo/power/
  once-only/deferred state save round-trips are exercised.
- 20,496 available event-choice transactions cover three characters and Acts 1–7.
  All 63 act/character/forge transactions preserve the selected card and cost;
  deck access preserves the underlying forge cursor. All 26 potion-use paths
  consume the potion and suppress the successful-use notification.
- 4,842 full card previews fit complete names/rules, including upgrades and all
  permitted forge modifiers. Combat renders for every character with one, two
  and three enemies and 987 maximum HP.
- 2,723 story text samples fit the four-line PSP dialogue box, including first
  journeys, return histories, ordinary resolutions, five-win revelations and
  the shared finale.
- Progression checks cover all characters, wins 1–11, five-win secret access,
  exactly twenty unlock cards through ten wins, all six secret-clear orders,
  all five finale pages, repeated victories, stale saves, cold restoration and
  injected account/finale write failures. The shared finale is not duplicated
  or lost when writes fail and are retried.
- Complete-renderer timeline tests examine every frame of the 185-frame finishing
  interval for Act 3, Act 4 and Act 7, including leader/minion last kills. They
  reject combat returns, double fades, premature boss endings and wrong scene
  artwork. The shared finale has one transition with a black midpoint.
- All 14,700 complete simulated campaigns were rerun, including the original
  3,000-seed holdout cohort. Results and per-card decision CSVs are byte-identical
  to dist1, with no stalls. `dist2-balance-equivalence.json` records the hashes.
  These are paired regressions; the reused holdout seeds are not a new holdout.
  Detailed balance and decision data are in `BALANCE.md`.

## PSP executable checks

PPSSPP 1.20.4 uses PSP-1000 memory settings and the complete release EBOOT.
Debug fixtures set up legal test positions; real controller inputs execute card,
potion, inventory and ending actions. These are assisted regression tests, not
unassisted human campaign completions. Repeated scene-routing checks skip already
resolved presentation delays; a separate phase observes the actual finishing
transitions without that shortcut.

For this dist2 executable, completed phases are `review`, `cards`, `gates`,
and `review-finale`. They cover the twelve fourth/fifth/tenth/eleventh-win gates;
each character's card/replay/power/potion/end-turn state against a native oracle;
three forge choices and deck overlays; all four revised Proxy entry settings,
a three-line boss exchange, three complete revised story events, the first-run
Act 3 resolution, corrected character/forge gift text, and the third-character
individual/shared ending followed by a repeat victory that skips the finale.

Representative revised scenes and HUD/menu screenshots were visually inspected
at 480x272. The native suites rerun all six finale orders, cold restoration,
write-failure handling and complete transition frames. Dist1's broader emulator
cold-restart and six-order/full-fade evidence remains historical; it is not
represented as a fresh run of every emulator phase on dist2.

The final per-phase logs and representative screenshots are stored under `distribution/evidence/`.
They record the tested executable's SHA-256. Physical PSP hardware has not been
available in this environment, so no physical-device validation is claimed.

## Build, media and stack checks

- `distribution/tools/check_catalog.py` verifies unique card/relic/potion names,
  known inherited-name regression hashes, deterministic generated rules, and
  removal of the retired catalogs. This is not an exhaustive comparison with
  every published game or a legal clearance check.
- `review_text.py` adds pinned two-game reference-name regression hashes and
  checks removal of the identified identity/rule remnants. The broader text
  comparison and its limitations are in `LEGAL_REVIEW.md`.
- Preserved media bank and embedded-image SHA-256 values match the exact prior
  release. Title music produces nonzero stereo PCM in PPSSPP. Artwork/music
  creative provenance was not audited, as directed by the owner.
- The PSP import audit finds no kernel-only import libraries.
- Compiler stack reports cover startup, slot resume, standalone load, save/quit,
  card actions and rendered previews. The 192 KiB game-thread allocation is
  unchanged; the tightest checked chain leaves 100,912 bytes after its 8 KiB
  small-callee allowance, exceeding the separate 32 KiB margin. This bounded
  call-chain audit is not a whole-program stack proof.
- A clean local rebuild matches the fully tested executable byte-for-byte. The
  build always regenerates PSP menu metadata to avoid a stale incremental title.
- Release CI rebuilds with the same pinned toolchain and requires an exact match
  to the locally playtested EBOOT digest before packaging. Source archives contain
  only the distribution snapshot and preserved media, not legacy catalogs or
  historical executables. All uploaded release asset sizes/digests are verified.

## Reproduction

```sh
PSPDEV=/path/to/pspdev bash distribution/build.sh
python3 distribution/tools/check_catalog.py
python3 distribution/tools/check_stack.py
bash distribution/test.sh
python3 distribution/tools/run_campaigns.py
python3 distribution/tools/run_campaigns.py --holdout
```

`distribution/tests/fixtures.c` and `layout.c` generate emulator positions and
layout offsets. `ppsspp.py` uses a local PPSSPP/Xvfb toolchain supplied through
`RIFT_TOOLCHAIN`; phase logs explicitly identify setup assistance and any skipped
presentation intervals. See the harness for environment and debugger requirements.
