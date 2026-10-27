# Rift Ascent 1.5 — preview-height correction

Expanded hand previews grow with their full descriptions while staying clear of the health bars, enemy intents and Energy warning. The longest cards use a slightly shallower illustration and, only if needed, a little more width. Text sizes, modification seals and all description content are preserved. Previews remain aligned with their position in the hand; played-card animations use the same fitted layout and stay below the HUD.

Validated 6,280 base/upgraded/modified card layouts: heights range from 100 to 162 pixels; the widest stress case is 136 pixels. Another 2,800 rendered hand and animation frames preserve the HUD, including insufficient-Energy cases. Native sanitizer regressions and actual PPSSPP navigation/card-play checks pass. Combat.png is a native PPSSPP screenshot of a long modified card description.

All 1.5 gameplay additions, five music tracks, splash assets and save compatibility are preserved. Keep SAVE.DAT and PROFILE.DAT when replacing EBOOT.PBP. The in-game version remains 1.5; this correction uses release tag v1.5-r2 without replacing the original release. Physical PSP hardware was not available for testing.
