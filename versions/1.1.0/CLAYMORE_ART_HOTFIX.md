# Rift Ascent 1.1.0 — Claymore art correction

This is a **visual hotfix of 1.1.0**, not a gameplay-balance release. The first 1.1.0 claymore was overly bright, long, glossy, and visually disconnected from the original dark pixel-painted warrior.

## Correction

- Completely removed the oversized, skyward-pointing, blue-white sword geometry.
- Repainted the blade at **native pixel resolution**, not an oversampled vector shape, to match the original slightly gritty sprite.
- Sword now points **forward and slightly downward**, aligned with the original character's right glove and hilt rather than crossing his face/upper torso.
- Revised proportions: hero sprite width **202** pixels (previously 210), blade width approximately 8–12 pixels and length proportional to the character.
- Match the original style with **aged gunmetal, soot gray, dusty bronze and weathered steel**. No glossy white highlights, bright gold starbursts, or neon edges.
- Simplified crossguard and wrap; the original helmet, red tattered cape, armor, pose, and surrounding scene remain unchanged.
- Seven previously improved map icons remain unchanged.
- Game mechanics, card database, Inferno, saves, music tracks, and audio levels remain exactly as in 1.1.0.

## Verification

The patch is applied by `overhaul/v110/generate_art.py` before PSPDEV compilation. The same native-game tests, PBP import audit, PPSSPP gameplay render/screenshots, and 10-track embedded-audio smoke tests remain active.

**Packaging:** The corrected complete EBOOT includes the same ten soundtrack tracks at the same boosted audio gain. The old unmodified 1.1.0 release remains available as a backup.

**Hardware:** Compilation and emulator checks are distinct from physical PSP-3000/ARK-5 verification. Test voluntarily with your original EBOOT and `SAVE.DAT` backed up.
