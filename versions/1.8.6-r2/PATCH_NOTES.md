# Rift Ascent 1.8.6-r2

- Fixed the Act 3 boss ending fading out immediately after the killing blow, returning to combat, and then abruptly cutting to the ending. The finishing animation now owns one fade out, followed by a smooth reveal of the cutscene.
- Applied the same correction to the shared Act 4 and Act 7 ending transitions. Both Normal and Fast animation speeds are covered.
- Gameplay, encounter balance, artwork, save compatibility and all twelve complete soundtrack streams remain unchanged from 1.8.6. The in-game version remains 1.8.6.

Validation: pixel-level native rendering regression with sanitizers; every rendered frame captured from the exact packaged EBOOT in PPSSPP 1.20.4, including Act 3 boss-last and minion-last endings, Normal/Fast speeds, and Act 4/7 controls. The original renderer and an incomplete fix both fail the new regression. Captures confirm one final fade and no return to combat. Physical PSP validation remains with players.

Install the PSP ZIP or replace `PSP/GAME/RIFTASCENT/EBOOT.PBP`. Keep existing save/account files.
