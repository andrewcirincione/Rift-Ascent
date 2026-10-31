# Rift Ascent 1.8.5 — quiet potion feedback

- Remove the black combat confirmation box after using any potion, including
  “Guard potion: +20 Block.” Results remain visible through the normal
  meters, status icons and effect animations.
- Suppress routine potion success captions in inventory. Failed-use
  explanations remain, and unsuccessful attempts keep the potion.

Includes all preceding changes:

- Clearly raised, bright Forge selection with a shadow, arrow and label.
- Square opens your deck at the Forge; Start opens inventory. Inspection is
  available across run choices/results and preserves the current choice.
  Cutscenes remain uninterrupted.
- Forge is one possible story outcome, not an always-available alternative.
- Cinder Gambit: 150 Gold for 6 HP, or 180 Gold plus Injury. Improved terms
  for Red Communion and Candle for a Stranger.
- Elites have 8% more HP; bosses have 10% more, retaining their attack cycles.
- Retain the PSP startup stack fix, saves, full artwork and all 12 music tracks.

The unchanged game code's benchmark is 280/1,200 wins (23.3%), including
24.2% on the independent holdout. These are automated comparison results.
The exact complete EBOOT passed potion-effect/notification, failed-use,
Forge/deck-return, combat, audio and startup checks in PPSSPP. Physical PSP
confirmation of this exact binary remains outstanding.

In-game version: **1.8.5**. Release tag: **v1.8.5-r5**.
Install the complete `RIFTASCENT` folder under `PSP/GAME`, preserving your
existing save files when replacing the EBOOT.

EBOOT SHA-256:
`21a40f9513685012db65c75db7a7c8583bed33939481ba4cedde9ae5a8e0cd9f`
