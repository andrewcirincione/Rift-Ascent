# Rift Ascent 1.0.3 — feedback-driven gameplay and soundtrack revision

**Build channel:** `release-1.0.3`. Versions 1.0.0, 1.0.1 and 1.0.2 remain archived independently.

## Implemented
- PSP XMB background artwork: new full-screen illustration showing the armored Warden, claymore and towering red rift. Applied when packaging the downloadable EBOOT via `tools/set_pbp_pic1.py`.
- More compact enemy intent panels; remove the distracting red/white slash streaks.
- Card cost and damage forecasts no longer draw in the same place; Whirlwind displays the X-cost symbol.
- Selected combat cards grow wider **within the bottom card row**, with longer on-card descriptions and no separate floating tooltip obscuring the battlefield.
- No double-rendered Weak, Vulnerable or Poison text beneath their dedicated side status chips.
- Enemy Block is integrated with the HP bar instead of a separate large popup.
- `LOW ENERGY` replaces the inaccurate `NO ENERGY` indication.
- Dead foes cannot be selected as targets in multi-enemy fights; played attacks retarget when necessary.
- Victory animation no longer resurrects defeated foes for a frame.
- Map can be scrolled to **any remaining row**, while still preventing entry into non-current encounters. Connected roads get thicker contrast strokes.
- Combat footer prioritizes Play, End Turn, Deck, and Inspect; deck information remains available with Square.
- A layered, extended steel claymore is painted over the hero combat sprite.
- Inspected the original 36-enemy sprite sheet and selectively mirrored 11 previously backward-facing soldier, beast and knight variants, without reversing forward-facing species.
- Redesigns and balances multiple cards around Ironclad-style Strength, Block, Exhaust, Fatal, self-damage and zero/X-energy strategies, including Strike, Defend, Bash, Inflame, Whirlwind, Armaments, Offering, Demon Form, Fiend Fire, Feed, Inferno, Uppercut, Thunderclap, Anger and others. These are gameplay adaptations, **not** imported StS2 artwork or full byte-identical card text.
- The old procedural **music** is completely muted, while retaining original sound effects.
- All TEN user-supplied tracks are embedded in the single EBOOT as low-bitrate 4-bit IMA ADPCM (11,025 Hz mono):
  - Title: **Checkers**
  - Menus / shops / events: **RiftMenu**
  - Battles (rotated by run/act/floor): **BARBARICE, BREAKER, Cof, Devour, DIRE, Drudge, OVERT, STAMMER**
- Music is decoded in small chunks from the EBOOT rather than loaded into PSP memory at once.

## Verification
- Source-level gameplay scenarios plus an assisted 3-act simulated campaign (164 action steps; all acts, shops, events, relic screens and victory), PSPDEV compilation, user-mode import audit, PPSSPP boot/screenshot checks, and a synthetic 10-track embedded-PBP test confirming the music index is read via ms0:.
- Review representative PPSSPP combat, map, inspect and title screenshots rather than treating a synthetic test alone as a manual end-to-end playthrough.
- The real ten-track **packaged EBOOT** also receives PIC1 PNG/offset, ADPCM index/CRC and size checks before distribution.
- Hardware compatibility and music playback on PSP-3000 with ARK-5 have **not** yet been physically verified.

## Outstanding / caveats
- This updates a large subset of cards but does **not** reproduce every Slay the Spire 2 Ironclad card, upgrade, enemy attack or enemy health stat. Full encounter-by-encounter balancing and exhaustive run-testing require additional cycles.
- Additional physical-PSP checks could reveal other animation or orientation issues that static sprite review cannot catch.
- Validate attack SFX/music transitions, all card descriptions, and full campaign route/boss rewards on physical hardware.
- Existing save structure and `GAME_VERSION=16` are preserved, but the changed card IDs/descriptions may alter the interpretation of old saved decks; back up `SAVE.DAT`.

## Deliverables
- `tools/embed_soundtrack_103.py`: executable-packing tool (reads the supplied ZIP's WAV files)
- `tools/set_pbp_pic1.py`: PSP XMB art replacement tool
- `overhaul/v103/`: independent patch scripts + deterministic native tests
- A single all-in-one `EBOOT.PBP` distributed in the current conversation as a ZIP and loose file (with SHA256 manifest and screenshots).
- The complete audio assets are not copied into public GitHub because they are user-supplied media. The compiled reproducible PSP base can be obtained from CI artifacts and repackaged using your original WAV ZIP.
