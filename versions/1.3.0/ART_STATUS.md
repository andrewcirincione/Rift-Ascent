# Rift Ascent 1.3.0 — AI texture integration status

Updated: 2026-10-26

## Build provenance

The public branch compiles **build 264** successfully under PSPDEV and passes the
GitHub Actions/PPSSPP smoke suite. Build 264 deliberately carries art *sentinels*
in eleven scene slots and six HUD/card UI slots. **Do not distribute build 264
unchanged as a finished art release.**

A private, post-link art-integration pass now replaces:

- Eleven 480 × 272 RGB565 scene slots: three act maps, three act bosses,
  two normal fight backgrounds, two elite fight backgrounds, and the campfire.
- Six HUD sprites: hand surface, health trim, energy trim, attack/skill/power
  borders; the trim centers use transparent color key so game text stays visible.
- All 92 card bitmaps at 60 × 40; Strike and Defend have stable shared art
  across copies.

The images are raster conversions/crops of genuine AI-generated paintings
rather than the old geometric Pillow generator. Some card images reuse
different compositions or crops from source paintings, so **92 bespoke
independently illustrated concepts are not yet complete**.

The post-link pass preserves the compiled PSP DATA.PSP bytes outside these
verified art ranges. It also privately packages RiftBoss/RiftMenu/RiftFight
using the RIFTM119 three-track trailer and the RiftBoss XMB SND0 ATRAC3
preview. No private soundtrack bytes should be pushed to this public repo.

## Quality gates and remaining work

- PSPDEV compile and PPSSPP smoke tests: **passed for the unmodified,
  sentinel-bearing build 264**.
- Deterministic post-link validation: **passed for 11 scenes + 6 UI sprites +
  92 card art bitmaps; no sentinel patterns remain**.
- PBP index, checksum, original execution segment, SND0.AT3 and ADPCM footer:
  **validated for the private package**.
- Modified-art runtime screenshots in PPSSPP: **not yet verified** (the
  patched executable is outside GitHub CI; rerun emulator on this exact file).
- Real PSP hardware: **not yet checked for the modified file**.
- All 92 card concepts individually image-generated, layout review at PSP
  native resolution, extended playthrough / no overlaps: **still open**.

The private download includes contact sheets and a per-texture SHA256 manifest
to support review and eventual final art sign-off.
