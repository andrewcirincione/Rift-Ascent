# 1.8.1 working record

Base is the public v1.8-r2 tree 606e0f4819a37efff6d3a0b32cd7c268cdac9838,
main c28191e9161cda3e4a15ee8e5e7c28c753a4ae96. Local release-1.8.1 starts from
6c60b1b (identical source tree). Preserve main history when publishing.

Requested: mandatory very hard Act 7 god-abomination, before/after scenes;
Expect a Fight energy lock; progressively stronger post-Act-3 enemies;
20 new always-available relics; exhausted relic rewards never blank;
60 character events (20 each, 5% after normal/elite combats), dialogue and
card/relic gifts; rare optional foreign-character rare-card event;
playtest exact compiled EBOOT, push main and publish v1.8.1.
User additionally reports 1.8 XMB splash music missing.

Implemented so far, not validated:
- New canonical source directory source/; changes only here until final sync.
- Append-only Expedition181 preserves complete 1.8 save prefix. 32 overflow
  relic slots raise capacity to 96; accessor replaces direct indexed reads.
  1.8 slot migration and earlier SAVE.DAT prefixes retained; account unchanged.
- Energy gain helpers cover Bloodletting, Offering and Energy potions.
- Quadratic modest extra HP/damage scaling only beyond Act 3.
- Boss 45 THE UNBEGOTTEN, 610 base HP, 31 base attack, three phases and
  telegraphed moves. Every Act 7 final boss. Other Endless bosses remain random.
- 20 relic definitions and engine hooks; 60 authored side stories/180 pages.
- Queue 5% stories once per normal/elite victory, show after standard spoils,
  no repeats until all 20 seen; extra gift screen. Rare mystery appears in 4%
  of mystery rooms, with three functional rare cards from other heroes.
- Boss/elite exhausted relic pool falls back to repeatable stat blessings.
- Splash cause: ICON1 239616 + old 1.8 SND0 397584 = 637200 bytes over shared
  ~500 KiB XMB budget. Building 14-second high-quality LP2 updated RiftBoss
  preview with combined 480 KiB guard. In-game ten-track soundtrack unchanged.

Still required:
- Finish new art (boss sprite generated/copied; 12 character scene settings,
  before/after boss + arena + mystery + eight relic icons planned), integrate
  all rendering/UI, expanded base-relic collection page and new screens.
- Write strict pack/build/media tests against v1.8-r2 bank and unchanged audio.
- Thorough native regressions: every relic, 60 stories, 5% rolls/no rerolls,
  migration and >64 relic persistence, foreign cards actually resolve,
  energy lock reset/all sources, Act 7 phases/routes, fallback reward offers.
- Exact packaged PPSSPP tests and screenshots; fixture-assisted late-game.
- Sync GAME/src + portable_build, metadata/notes/checksum/Combat.png,
  commit source and EBOOT, publish v1.8.1 and verify public digest.

Toolchain paths: ../toolchain/pspdev, ../toolchain/ppsspp/AppDir/AppRun,
../toolchain/atracdenc/build/src/atracdenc. Previous QA driver:
qa/rebuild180/qa_ppsspp.py, slow software host; actual L/R buttons are
ltrigger/rtrigger. Use native fixture state layout and final ELF symbols.
Do not regenerate all 1.8 art; preserve it and the existing soundtrack bank.

GitHub connector supports create_blob/tree/commit/branch/update_ref, fetch.
CLI push has no credentials. Prior transfer used git bundles split into 512 KiB
parts, uploaded as verified blob SHAs, incremental subtree batches <=32.
Temporary import Actions workflow validates then commit-tree creates main with
old main and new local head as parents; nonforce push, dispatch publish workflow.
Reference transport code lives in /tmp/rift-transfer from 1.8; adapt to 1.8.1,
new branch/READY path and expected main. Never reuse old READY blindly.

Unrelated preexisting untracked files: portable_build/.rift150_art.h.UjfgEf,
qa/v135/.icons-original.png.1gwHgA, qa/v170/source/.rift162_art.h.kclwQd,
versions/1.5/.EBOOT.PBP.V5EKWz, versions/1.7/.EBOOT.PBP.vde5ED.
Leave those untouched/untracked. No subagents requested or permitted.
