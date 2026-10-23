# Rift Ascent 1.0.2 — release candidate

**Status:** PSPDEV compile, emulator smoke tests, and physical PSP-3000 validation are tracked separately. Do not equate PPSSPP screenshots with hardware proof. The archived `EBOOT.PBP` is produced only by a successful build.

## Changes, applied in requested order

1. **Save & Quit:** SELECT pauses; Cross writes a validated snapshot to `ms0:/PSP/GAME/RIFTASCENT/SAVE.DAT` and exits only when the write succeeds. Circle/SELECT resumes without losing the previous menu context. Saving a turn with END highlighted is valid.
2. **Cards and HP:** Combat hand now rises only above the bottom UI, leaving player and enemy health bars visible.
3. **Sword/damage forecast:** Weapon symbol has a dedicated left-side area separate from the numeric forecast.
4. **Sunder:** Damage and Vulnerable rider explicitly shown in its full hover preview, including upgraded values.
5. **Upgrade/deck grid:** Up/Down move an entire row; Left/Right move a column without wrapping.
6. **Store:** Distinct CARD, RELIC, POTION and SERVICE labels, costs and full selected-item descriptions.
7. **Controls:** Custom face-button glyphs in contextual footer directions.
8. **Armory event:** The two awarded cards are named and displayed as card images before the next room, with the 10 HP cost.
9. **Card explanations:** Complete selected-card details in battle, reward and deck view instead of relying solely on tiny card faces.
10. **Status badges:** Enlarged full-word STR, VUL, WEAK and PSN chips to the right of selected enemies, and redundant enemy status row removed.
11. **Nonattack intent:** Omit damage labels entirely for nonattacking actions.
12. **Overflow cue:** Smaller, less distracting hand continuation arrows.
13. **Enemy block:** Separate, always-visible block value panel above health bar.
14. **Unplayable cards:** Grey overlay for cards that cannot be played at current energy.
15. **Zero energy:** Energy circle switches to grey.
16. **Hand animation:** Previous hand cards move into discard, new hand enters from below during turn transition.
17. **Relic confirmation:** Boss, chest and relic-granting events show the acquired relic's exact name and full effect before advancing.
18. **Route map:** More distinct crossed-sword/skull icons; Up previews next floor, Down returns to current floor; preview cannot be entered.
19. **Combat deck:** Square opens four navigable card zones: DRAW, HAND, DISCARD and EXHAUST, with actual contents and count.
20. **Card names:** Combat-card titles use two complete lines, with adjusted art and energy placement.
21. **User-provided soundtrack:** `CHURN.PCM` plays during battles, streamed in 16 KiB chunks from PSP memory stick and mixed with existing SFX. Missing PCM safely falls back to procedural battle music; outside combat original procedural tracks remain.

## Installing the soundtrack

The audio is delivered **in the version download ZIP**, separately from the EBOOT. The original uploaded file was `CHURN.wav` (3:06, 44.1 kHz stereo float). For PSP efficiency it was converted into `CHURN.PCM` (22,050 Hz stereo, signed little-endian 16-bit PCM; ~16 MB) and must be located at:

`ms0:/PSP/GAME/RIFTASCENT/CHURN.PCM`

Install both `EBOOT.PBP` and `CHURN.PCM` in `PSP/GAME/RIFTASCENT/`. The audio file **is not embedded in EBOOT.PBP** and is not committed to the repository due to its size; the standalone download ZIP includes it.

## Integrity, compatibility and limitations

- PSP firmware target: 6.60, CRT-free user-mode imports, working NEXTVSYNC double-buffer presentation retained.
- Save layout: `GAME_VERSION=16`; no `Game` struct fields changed.
- Native automated tests and PSPDEV/PPSSPP smoke steps are in `.github/workflows/overhaul-psp.yml`.
- No hardware compatibility or frame-rate guarantee for the 1.0.2 EBOOT until physically tested.
- Preserve the previous working EBOOT before changing hardware files; copying or removing the music asset is reversible.
- Audio fidelity on a physical PSP and the start/loop behavior still need direct listening validation.
