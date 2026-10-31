# 1.8.5-r4: raised Forge focus and inventory access

This revision includes the story/event/challenge work documented in
`qa/v185r3/README.md`. The r3 release had entered publication when the user
added the Forge readability and inventory requests; r4 supersedes that
complete release without changing its tag or assets. In-game version stays
1.8.5 and the canonical source stays `qa/v185/source`.

The selected Forge panel moves up 3 pixels and extends 6 pixels farther on
each side. It has a shadow, a warm contrasting interior, two bright focus
outlines, the existing arrow artwork and an explicit SELECTED label.
Unselected panels remain recessed. All three choices use identical focus
cues, including an unavailable choice. Existing game artwork is reused.
The footer advertises Square for Deck and Start for Inventory; the same
shortcuts work in the card picker.

The shared overlay is now available across 28 in-run screens, including
Forge, story gifts, mystery card choices, pending card effects, unlock
results and run recaps. It is also available from pause/help when their
underlying screen supports inspection. Cutscenes and front-end setup stay
closed. Square retains the established battle-pile view during combat;
the full permanent deck remains in the inventory's Deck tab.

Opening, browsing and closing the overlay uses separate cursors and leaves
the underlying Game byte-identical. Between-room potion use is supported,
including healing at the Forge. Pending combat effects, pause screens and
completed-run results allow inspection but cannot consume/discard potions.
The footer explicitly labels their potion page VIEW ONLY. Pressing X in a
pause-menu overlay must not trigger Save & Quit; the outer PSP input handler
now checks that the overlay is closed before executing that action.

## Verification

`bash qa/v185r4/test.sh` runs the existing sanitizer suites plus the new
inventory control audit. It exercises all 28 screens, all excluded story
scenes, exact return-state preservation, ignored confirms in the Deck tab,
view-only item guards, Forge healing and pause return.

The final package is built with pinned PSPDEV v20261001 and the existing
startup stack audit. Run `qa/v185/qa_ppsspp.py` with
`RIFT_FORGE_ONLY=1 RIFT_INSTANCE=185r4 RIFT_PREFIX=r4- RIFT_DIRECT=1
RIFT_FAST_PRESENTATION=1` and the prepared `RIFT_TOOLCHAIN` environment.
It uses actual controller inputs on the exact complete EBOOT and assisted
state fixtures. It captures all three selected panels, opens Deck and
Inventory from each, opens Deck from all three card pickers, verifies the
complete return state, checks focus after cancel/save/resume, proves X in
the paused Deck does not save/quit, confirms upgrade and exact Echo cost,
and plays a card against the third enemy. Screenshots are inspected at
480x272. Audio must produce PCM with the full bank active.

Final exact-EBOOT fresh and beta-import boots are independently observed
through `qa/v185r2/boot_probe.py`, including thread stack use and frame 60.
The prior r3 revision's six-scenario migration evidence is retained there;
only the two explicitly listed scenarios are rerun on r4. The earlier r4
UI check before the pause guard is retained as historical evidence and is
not the final package result.

The reference campaign policy was rerun after the inventory change:

| Set | Wins | Runs | Rate |
|---|---:|---:|---:|
| Initial comparison seeds | 62 | 300 | 20.7% |
| Independent holdout seeds | 218 | 900 | 24.2% |
| Combined | 280 | 1,200 | 23.3% |

There were no policy stalls. These automated results meet the aggregate
20–25% benchmark target. They are not a prediction of human win rates or
per-character parity; the stronger decision policy and event-fork analysis
are documented with r3. Elite/boss HP and event terms are unchanged from r3.

The final digest, native regressions, complete media audit, stack audit,
exact-EBOOT input checks and measured cold boots are consolidated in
`evidence/final-tests.txt`. Physical PSP-3000 / ARK-5 confirmation remains
outstanding.
