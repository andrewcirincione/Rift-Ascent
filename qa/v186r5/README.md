# 1.8.6-r5 validation

Canonical source: `qa/v186/source`; source mirrors: `GAME/src`, `portable_build`. Release branch `release/1.8.6-r5` builds with PSPDEV v20261001, reproduces the tested digest, then fast-forwards main and publishes an immutable release.

`relic-coverage.json` records the 296-entry researched stable-game baseline and explicitly adapted rules. `RESEARCH.md` identifies sources and beta exclusions. `audio/` contains the full encoded supplied track, source hash and reproducible encoder; the release preserves all twelve existing streams and appends track 12.

`test.sh` runs ASan/UBSan regressions, including prior saves, five-win gates and one-time finale, all three card engines, three-enemy snapshots, relic stacking, replayed Void Form, 180,000 reward draws and 1,800 boon offers. Renderer tests examine every ending-fade frame for Acts 3, 4 and 6 and the finale artwork.

`campaigns.c` and `policy.c` use legal starter decks, all account unlocks and heuristic choices. Candidate and baseline use the same seeds. The policy is a balance probe and has unequal skill across the three characters. `evidence/balance-summary.json` reports the paired 900-run sample. Two candidate campaigns exposed the replayed Void Form deadlock; both were replayed from the start after the fix. No remaining stalled result is counted as a loss.

`fixtures.c` generates explicitly assisted states for emulator regression, and `layout.c` emits their actual structure offsets. `qa_ppsspp.py` operates the complete EBOOT through PPSSPP 1.20.4 in a PSP-1000 profile: new menus, Energy carryover, replayed Void Form, Act 6 intro/fight/music, Act 7 rotation, five-win gating and finale persistence. A separate pass records the complete Act 6 normal/fast finishing fades with either leader or minion last. Screenshots are read only after a complete rendered frame. Repeated functional finishers may skip presentation; the separate frame sequence pass does not.

Commands:
```sh
PSPDEV=/path/to/pspdev bash qa/v186/build.sh
bash qa/v186r5/test.sh
gcc -O2 -std=gnu11 qa/v186r5/fixtures.c -o /tmp/r5-fixtures && /tmp/r5-fixtures
gcc -O2 -std=gnu11 qa/v186r5/layout.c -o /tmp/r5-layout && /tmp/r5-layout
```

Copy the complete `qa/v186/evidence/EBOOT.PBP` into `qa/v186r5/evidence/` before emulator checks. Set `RIFT_TOOLCHAIN`, `RIFT_INSTANCE`, `RIFT_PREFIX`, `RIFT_DISPLAY`, `RIFT_PORT`, and `RIFT_DIRECT=1` as in the recorded run. Add `RIFT_FADE_ONLY=1` for the full-frame pass. Hardware PSP testing is not claimed.
