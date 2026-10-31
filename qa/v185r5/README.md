# 1.8.5-r5: quiet potion feedback

The user reported the black combat rectangle reading “Guard potion: +20
Block” after using a potion. It came from `r1310_potion_notice`, a dedicated
success-notification buffer rendered over the battlefield for 360 ticks.
This revision removes that buffer, its feedback writer, draw pass and reset
path for every potion type. It also removes routine healing/discard success
captions from the inventory overlay. Failed-use explanations (combat-only
potion, already full health) remain visible and keep the potion.

Potion mechanics, status icons, shield/health/energy values and effect
animations remain intact. `game.c` is byte-identical to r4 (Git blob
`3f4fe79e4b6f024c44aa682f1c7a96bbd8428d4c`), so the r4 1,200-run benchmark
still applies: 23.3% wins, including 24.2% on the 900-run independent
holdout. These are heuristic comparison results, not human win rates.
All story, Cinder, elite/boss, raised Forge focus and inventory changes from
r3/r4 are included, as is the original r2 PSP startup stack correction.

Build with pinned PSPDEV v20261001 and `bash qa/v185/build.sh`. The build
still audits startup/save stack budgets. Native tests are
`bash qa/v185r4/test.sh`; the release workflow reruns them.

Run `qa/v185/qa_ppsspp.py` with `RIFT_POTION_ONLY=1 RIFT_INSTANCE=185r5
RIFT_PREFIX=r5- RIFT_DIRECT=1 RIFT_FAST_PRESENTATION=1`, plus the prepared
`RIFT_TOOLCHAIN`. The exact complete EBOOT is driven through real controller
input with assisted fixtures. Guard, Healing, Strength, Bulwark and Fire
potions cover all three characters; their actual effects and consumption
are asserted. The legacy popup symbol must be absent. Screenshots are
inspected to confirm that the battlefield remains clear. Two failed-use
cases verify retained hints and unconsumed potions. Raised Forge focus,
deck opening/return, combat rendering, streamed art and audible PCM are
also checked. A separate fresh cold-boot probe measures the exact binary's
startup stack and verifies frame 60 and music.

`evidence/final-tests.txt` records the exact digest and results. R3/r4 retain
broader campaign, migration and inventory coverage. Physical PSP hardware
confirmation of this exact binary remains outstanding. The public build
is a new immutable tag, v1.8.5-r5; the in-game version stays 1.8.5.
