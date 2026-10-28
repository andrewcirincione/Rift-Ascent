# Rift Ascent 1.8

Promotes the working 1.8 prerelease to the main release with the owner's hardware feedback addressed. Earlier published tags and downloads are preserved.

## Changes
- Replaced How To Play with Options. Debug shortcuts default OFF and persist in a separate A18_OPTIONS.DAT file. With debug enabled, L clears all living enemies through the normal reward flow; R advances one act without granting a boss clear. Inventory shoulder-button navigation is unchanged. No cheat-control hints were added to combat HUDs.
- Audited the legacy roster and replaced six designs with separate AI-generated full-body assets: Ash Crawler, Bone Sentinel, Glimmer Moth, Mire Oracle, Ashborne Monk, and Entropy Knight. Detached props, alternate poses, labels, and backgrounds are excluded from the sprites. Acts 1-3 keep their themed normal pools. Act 4 and beyond can draw any of the 30 normal enemies, excluding elites/bosses and duplicate pairs.
- Corrected the Infernal Colossus and Rift Behemoth facing direction.
- Normalized shared footer order to Potions, then Deck.
- Health fills are bounded by the painted frame aperture and use clamped proportional arithmetic, including increased maximum health. Very large numbers use compact labels rather than crossing the frame.
- Moon Spring now heals a flat 20 HP for free, up to maximum health. Its alternative costs 30 Gold to copy an eligible deck card with its upgrade/modification. Cancellation does not charge Gold.
- Removed unlock totals from the title screen; they remain in Unlocks.
- Removed developer/loading-test text from ordinary startup. Genuine package errors still display diagnostics. Preserved the actual-launch-path fix confirmed on the owner's PSP.
- Inset both save-slot text lines inside the highlight's decorative borders.
- Removed Act 4 clear totals from character selection and stopped incrementing those obsolete counters. Their reserved bytes remain for save compatibility. Secret-boss completion is shown only by the golden A at the top left. Act 3 wins and longest completed-act record sit at the top right.
- Removed the extra X marker identifying the selected pause option. The selection highlight and footer confirmation hint remain.

## Preserved
All 60 custom character cards and their individual generated illustrations; all new potions and relics; shared unlock progression; four independent run saves; endings and secret bosses; the purple Act 4+ arena; the complete existing five-track soundtrack and XMB preview audio. Existing A18 run/profile journals remain layout-compatible.

## Installation
For an existing 1.8 installation, replace ONLY EBOOT.PBP in its current folder. Keep every A18_* file and any older SAVE.DAT/PROFILE.DAT. The ZIP uses PSP/GAME/RIFTASCENT_A18 so it can coexist with older versions. The startup code uses the actual launch directory, including renamed folders.

## Verification scope
Fresh PSPDEV compilation; native ASAN/UBSAN progression and feedback regressions; 900 progression sequences; 10,000 Act 4/12 normal encounters; all 60 new card base/upgrade/mod cases; proportional-health assertions; full native-resolution review screenshots and all new card layout checks. The exact complete release EBOOT is tested using emulated controller input and read-only state assertions in PPSSPP, from an installation directory containing spaces. Saved-game fixtures are assisted test setups, not unassisted balance playthroughs. The owner confirmed prerelease 2 on real PSP hardware; this new release has not separately received that hardware confirmation.
