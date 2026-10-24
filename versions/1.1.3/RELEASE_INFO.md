# Rift Ascent v1.1.3 — build candidate

**Branch:** `v1.1.3`
**Status:** PSP cross-build and PPSSPP smoke-tested. **Real PSP-3000/ARK-5 hardware remains unverified.**

## Changes

- Replaced the player sprite with original illustrated warrior art; the PSP XMB splash composites the **same sprite**, avoiding character mismatch.
- XMB PIC1.PNG is generated at exactly **480 × 272** pixels; no forced aspect distortion.
- New dedicated illustrated backgrounds for mystery/event and merchant/shop rooms; neither reuses the combat screenshot.
- Generated **92 distinct card-art illustrations**, indexed by actual card ID.
- No synthesized music is allowed in the release: source-only EBOOT is music-silent, while the packaged standalone release contains the exact ten user-provided WAV recordings compressed as indexed ADPCM. PSP SFX remain available.
- Added clear event choice descriptions, including what Dread does; existing card/relic/upgrade inspection remains available.
- Made random Exhaust effects explicitly report the card selected.
- Removed confusing duplicate/obsolete status labels above enemies and constrained intention text to its black background.
- Moved enemy Block beneath the HP bar with a shield glyph.
- Corrected the footer (Square: Deck; Start: Info / Inventory).
- Hidden map nodes are those with no valid incoming route from the starting lanes; moved boss icon clear of floor 12.
- Removed duplicate 'Spoils of Battle' rendering.

## Build and testing

Use **GitHub → Actions → Rift Ascent PSP Gameplay Overhaul → latest successful run on `v1.1.3`**, then download the hardware-unverified artifact. Extract `portable_build/EBOOT.PBP` to `PSP/GAME/RIFTASCENT/EBOOT.PBP`.

The Actions workflow compiles with PSPDEV, runs native engine tests with sanitizers, launches under PPSSPP, validates the startup trace, screenshots a combat interaction, and checks route/map and enemy glossary screens. **Passing emulator checks does not guarantee hardware compatibility or a suspend/resume fix on the real handheld.**

Keep your previously working EBOOT as a backup.
## User's soundtrack — mandatory release procedure

The GitHub Actions artifact intentionally has **no soundtrack**, because the recordings were privately supplied outside the repository. It must **not** be used as the distributable game. The complete EBOOT must append ten original tracks in this exact order using `tools/embed_soundtrack_110.py`: Checkers (1), RiftMenu, BARBARICE (1), BREAKER (1), Cof (1), Devour (1), DIRE (1), Drudge (1), OVERT (1), STAMMER (1). Original recorded audio only; never generate background music. A 96-byte `RIFTM103` index footer and CRC32 must validate before delivering the final EBOOT.

Native sound generator is now `overhaul/v113/sfx_only.c`. `overhaul/v113/test_qa.c` ensures background remains silent under all ten scene settings until an explicit SFX triggers. `overhaul/v113/test_audio_policy.py` statically excludes synthesizer sources. **The earlier source-only ~3 MB EBOOT is no longer considered a complete music release.**
