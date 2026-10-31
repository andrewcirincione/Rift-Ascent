# Rift Ascent 1.8.5 — boss damage tuning

- Increase boss attack damage by 5%, rounded to the nearest whole point per
  hit before Weak, Vulnerable and player Block. Displayed intent and actual
  attacks use the same calculation, including multi-hit attacks and saves.
- Ordinary enemies and elites retain their existing damage. The preceding
  +8% elite HP and +10% boss HP adjustments remain included.
- Includes the occasional story Forge, improved Cinder Gambit bargains,
  raised Forge selection, deck/inventory access, quiet potion feedback,
  PSP startup stack correction, full artwork and all twelve music tracks.

Final automated benchmark: 252/1,200 victories (21.0%), including 197/900
on held-out seeds (21.9%), with zero policy stalls. These are comparative
heuristic results, not predictions of human win rates.

The exact complete EBOOT passed native damage/status/save regression checks,
actual boss attack and elite control checks in PPSSPP, startup stack checks,
combat rendering and audio validation. Physical PSP confirmation of this
exact binary remains outstanding.

In-game version: **1.8.5**. Release tag: **v1.8.5-r6**.
Replace the EBOOT in `PSP/GAME/RIFTASCENT`, preserving your existing saves.

EBOOT SHA-256:
`ce54a7e27d83784e2c1c57ee8d3eb5d625c4ddf6f81410391004bc7435069ffc`
