# Rift Ascent 1.8.5 — clearer Forge focus and inventory access

- The focused Forge option is visibly raised and wider, with a shadow,
  contrasting background, bright outline, arrow and SELECTED label.
- Square opens your current deck from the Forge and its card picker.
  Start opens inventory. Closing either preserves the exact choice.
- Deck/inventory inspection is available throughout run decision and result
  screens, including pending card effects and recaps. Story cutscenes remain
  uninterrupted. Unresolved effects and finished runs allow viewing only.
- Fix a pause-overlay edge case: confirming a card in the paused Deck must
  not activate Save & Quit underneath it.

Includes the preceding balance changes:

- Forge is one possible story outcome, not an always-available alternative.
- Cinder Gambit offers 150 Gold for 6 HP, or 180 Gold plus Injury.
- Red Communion's offensive bargain costs 3 HP and adds 1 Vulnerable.
  Candle for a Stranger costs 4 HP for its existing next-combat boon.
- Elites have 8% more HP and bosses 10% more, retaining their attack cycles.

The reference bot won 280/1,200 runs (23.3%); the independent 900-run holdout
finished at 24.2%. These are automated comparison results, not predicted
human win rates. Native sanitizer checks, all Forge focus states, inventory
return behavior, save/resume, combat, music and startup are validated
separately on the exact complete EBOOT. Physical PSP confirmation remains
outstanding. See `qa/v185r4/README.md` for evidence and limitations.

Retains the 1.8.5-r2 PSP startup stack fix, existing save compatibility,
full artwork and all twelve music tracks. In-game version: **1.8.5**.
Release tag: **v1.8.5-r4**. This supersedes r3 without replacing its assets.

Install the complete `RIFTASCENT` folder under `PSP/GAME`, preserving your
existing save files when replacing the EBOOT.

EBOOT SHA-256:
`3e3b18118e8d157667b0a89e9781f50d8da42f77c7b92e05707cd6ede0381027`
