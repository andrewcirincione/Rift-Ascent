# Distribution validation record

Target: `v1.8.6-dist1`, in-game version 1.8.6, PSPDEV `v20261001`.
The full executable includes the unchanged artwork and complete soundtrack.

SHA-256: `b4b7250dc2a97ce9172fdaa5e98c88a0de5aedf08919f4de8989129c07f783e0` (99103871 bytes).

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
- Progression checks cover all characters, wins 1–11, five-win secret access,
  exactly twenty unlock cards through ten wins, all six secret-clear orders,
  all five finale pages, repeated victories, stale saves, cold restoration and
  injected account/finale write failures. The shared finale is not duplicated
  or lost when writes fail and are retried.
- Complete-renderer timeline tests examine every frame of the 185-frame finishing
  interval for Act 3, Act 4 and Act 7, including leader/minion last kills. They
  reject combat returns, double fades, premature boss endings and wrong scene
  artwork. The shared finale has one transition with a black midpoint.
- 14,700 complete simulated campaigns, including 3,000 independent holdout seeds,
  finish without stalls. Detailed balance and decision data are in `BALANCE.md`.

## PSP executable checks

PPSSPP 1.20.4 uses PSP-1000 memory settings and the complete release EBOOT.
Debug fixtures set up legal test positions; real controller inputs execute card,
potion, inventory and ending actions. These are assisted regression tests, not
unassisted human campaign completions. Repeated scene-routing checks skip already
resolved presentation delays; a separate phase observes the actual finishing
transitions without that shortcut.

Completed executable checks include all twelve character/win gates, all six
character clear orders and repeat victories, two cold restarts, six full ending
sequences plus the shared finale transition, and each character's card/replay/
power/potion/end-turn state compared with a native oracle. The three-enemy HUD
and high-maximum-HP bar were visually inspected at PSP resolution. Forge choice
and deck-overlay screenshots are also included in the evidence.

The final per-phase logs and screenshots are stored under `distribution/evidence/`.
They record the tested executable's SHA-256. Physical PSP hardware has not been
available in this environment, so no physical-device validation is claimed.

## Build, media and stack checks

- `distribution/tools/check_catalog.py` verifies unique card/relic/potion names,
  known inherited-name regression hashes, deterministic generated rules, and
  removal of the retired catalogs. This is not an exhaustive comparison with
  every published game or a legal clearance check.
- Preserved media bank and embedded-image SHA-256 values match the exact prior
  release. Title music produces nonzero stereo PCM in PPSSPP. Artwork/music
  creative provenance was not audited, as directed by the owner.
- The PSP import audit finds no kernel-only import libraries.
- Compiler stack reports cover startup, slot resume, standalone load, save/quit,
  card actions and rendered previews. The 192 KiB game-thread allocation is
  unchanged; the tightest checked chain leaves 100,912 bytes after its 8 KiB
  small-callee allowance, exceeding the separate 32 KiB margin. This bounded
  call-chain audit is not a whole-program stack proof.
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
