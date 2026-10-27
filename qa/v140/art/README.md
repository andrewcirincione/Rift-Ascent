# Generated art for 1.4.0

The original PNGs were generated using OpenAI image generation for this release, then inspected and integrated directly in the game. These are new painted assets, not programmatic decorative line additions.

Design briefs:
- `victory.png`: transparent dark-gothic Victory wordmark, readable ivory/gold engraved letters, restrained winged crown, wide composition, no background panel.
- `selection.png`: understated wide black leather/velvet selection plate, bronze end details and a small pale selection marker at the left, quiet center with space for readable text.
- `flames.png`: six consistent red/amber flame sprites in a 3-by-2 sheet, fixed cells, transparent background, designed for a stable title logo on black.
- `chest.png`: complete dark gothic relic chest, front/right perspective, isolated transparent background, feet and right side fully visible with generous clear margins.

`prepare_art.py` crops alpha bounds, resizes and packs the runtime arrays. It preserves a transparent margin around the chest. `build_icon.py` composites the existing title logo above the flame cycle and encodes the real PSP ICON1.PMF; the wallpaper and RiftBoss SND0 are unchanged. Original art and derived PNGs are retained for reproducibility.

Additional generated assets: `energy_empty.png` is a new smoky-grey energy jewel. `attack-sheet.png` contains six new heavy greatsword poses. Exact built-in ImageGen prompts and references are recorded in `extra-prompts.json`. `prepare_attack.py` extracts complete connected sprites rather than cutting at cell borders, preserving the whole blade; measured boot anchors share one canvas position and uniform scale.
