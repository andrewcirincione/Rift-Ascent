# Generated bronze Stars artwork

Created with the built-in image-generation tool from the existing `qa/v160/assets/star.png` as a redesign reference. Original output: `bronze-stars.png`; runtime crops: `star-lit.png` and `star-empty.png`. `prepare_art.py` only crops, resizes and packs the original RGBA pixels; it does not draw substitutes.

Prompt: Redesign the reference star resource icon as a matching pair of high-contrast dark aged bronze five-pointed stars on transparency. The active variant has a restrained amber halo outside the silhouette; the depleted variant has none. Both have a broad, quiet, nearly-black bronze center for subsequently rendered bright numbers, copper bevel highlights at the outer tips, no circular rim, cage, ornament, text or numbers. Make the silhouette readable at 36-pixel HUD and 20-pixel card sizes, with generous margins around the entire glow.

The game renders white or bright-peach numeric glyphs with a full dark outline. HUD values 0–99 use the large font; 100–999 use the compact bold font. Card badges are 18 pixels in the hand and 24 pixels in enlarged previews. The texture fills remain dark at zero; only the halo changes.
