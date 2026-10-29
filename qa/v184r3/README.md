# Rift Ascent 1.8.4 revision 3

This correction is based on published v1.8.4-r2, commit 51f0ec49ad700bec22f6de6a85df3128d75966a8. The in-game version and save layout remain 1.8.4/184. The new tag is v1.8.4-r3; earlier releases are preserved.

Build with `PSPDEV=/path/to/pspdev bash qa/v184r3/build.sh`. The build extracts the exact artwork bank and twelve-track soundtrack from `versions/1.8.4-r2/EBOOT.PBP`, then appends them to the rebuilt engine. The source archive retains this required binary input and omits other historical compiled EBOOTs to stay below GitHub's asset limit. All historical binaries remain in Git and their own releases.

Afterimage now snapshots its active stacks for Warden cards as well as the new unlock cards. Proxy/colorless plays queue a serializable RT_AFTERIMAGE task after their effect/finish tasks, retaining the pre-play stack count through choices and nested autoplay. Existing Vesper task semantics and save struct sizes are unchanged. The new task kind is included in save validation.

Enemy damage uses a 1.06 multiplier with nearest-integer rounding at the shared raw-damage helper, before Weak and player defenses. Intent display and actual attacks share that calculation. Two exclusive elite moves reuse existing Block, Burn, multi-hit and Energy-drain mechanics; their new intent IDs are validated and rendered.

`tests/afterimage.c` reproduces the old failure with BASELINE, then checks 192 family/character/mode/upgrade cases, rejection/draw behavior, stacking, saves, Echo/autoplay and delayed choices. `tests/combat_tuning.c` checks 12,960 damage combinations, all 27 intents against actual damage, both new move rotations and saved combat effects.

Run `bash qa/v184r3/test.sh` for the native suites and rendering checks. RIFT_SUITES selects individual gameplay suites. `tests/card_art.c` compares every base card thumbnail against its upgraded/flagged variants, using the real streamed assets. Compiling it with BASELINE reproduces the blank-thumbnail bug in v1.8.4-r2. `tests/catalogs.c` checks exact catalog membership and order, locked/unlocked profiles, and each of the 60 individual card unlock bits after reordering.

`tests/routes.c` checks topology, connectivity and branch commitments across 10,000 maps. `tests/map_integrity.c` checks all pairs of branch prefixes across 70,000 maps; identical prefixes must not reconnect. Its BASELINE build compares each act's room totals, camp/shop/treasure floor counts, and the unchanged legacy route algorithms. The stored digests agree. Unlike the prior build's aggregate percentage, every tested fork must pass. A runtime guard also refuses any duplicate branch that could remain in an unusual layout.

New maps use route_style 2. Existing styles 0 and 1 retain their current-act paths; the next generated act uses the correction. Junctions preserve two/three live lanes and two/three-room commitments. A small number of combat/mystery rooms may trade places with nearby floors to make a choice distinct; act totals and camp/shop/treasure timing remain unchanged. Shops, treasures and camps are never exchanged across floors. The first two floors retain their original room contents.

Runtime testing uses the exact packaged EBOOT in PPSSPP 1.20.4, PSP 1000/32 MiB mode, isolated fixtures and controller input. `RIFT_REVISION184R3=1` tests actual upgraded-card addition/removal, mixed-character thumbnail pages, saved custom-draft resume, sorted catalogs and map branches. `RIFT_AFTERIMAGE=1` additionally exercises mixed-family Afterimage plays, stacking, save/resume, next-turn persistence and both new elite actions. The normal mode retains the broader gameplay/menu regression. Use distinct RIFT_INSTANCE, RIFT_DISPLAY and RIFT_PORT values for concurrent runs; no user saves are touched.

The emulator uses software OpenGL, dummy SDL audio and host fast-forward/frame skipping. This is targeted playtesting, not an unaided full campaign or a claim of physical PSP testing. Exact binary hashes and observed results are recorded in evidence/final-tests.txt.
