# Original 1.6 artwork

Generated specifically for Rift Ascent using the image-generation tool. No STS2 artwork was downloaded or reused. The wiki supplied mechanics and card names; these paintings, sprites and UI assets are original generated assets.

- `nightshade.png`: Vesper, twenty fresh frames in a 5 × 4 sheet. Weathered white hood/cape, crouched dark leather assassin, gritty gothic finish. Idle, dagger lunge, guard, short casting gesture, hit and collapse.
- `regent.png`, `regent-clean.png`: Proxy, twenty fresh frames. Blue-robed wizard king, tarnished crown, enormous gold-lit sovereign sword. Idle, overhead swing, guard, short golden casting gesture, hit and collapse. The clean sheet is an image-generation transparency cleanup of the original.
- `regent-cards-0.png` through `regent-cards-3.png`: 96 distinct gothic fantasy illustrations, six columns/four rows in catalog order. Celestial magic, ruined royalty, forged weapons, summoned retainers and fallen-star landscapes.
- `colorless-cards-0.png` through `colorless-cards-2.png`: 53 supporting Colorless illustrations, six columns/three rows in catalog order; the final spare cell is unused.
- `icons.png`: golden Star resource medallion, luminous star particle, monochrome Select button, Divine Right crown icon.

`../prepare_art.py` extracts cells, aligns feet and body anchors, downsamples and converts to PSP palettes. Vesper uses a smaller silhouette; neither character slides its body anchor during attacks/powers. Original generated sprites are preserved here; no prior Vesper frames are reused. Indexed images retain their alpha palettes, keeping the 32 MB PSP memory budget viable.
