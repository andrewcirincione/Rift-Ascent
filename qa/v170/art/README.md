# Generated art provenance

Mode: built-in image generation. Exact prompts are retained in `prompts.json` and `boss-sprite-prompts.json`.

- `warden-entries.png`, `vesper-entries.png`, `proxy-entries.png`: four stacked paintings per character, ordered Act 1, Act 2, Act 3, Endless.
- `warden-bosses.png`, `vesper-bosses.png`, `proxy-bosses.png`: three stacked character/boss confrontations, ordered Furnace Maw, Crimson Judge, Hollow King.
- `furnace-maw.png`, `crimson-judge.png`, `hollow-king.png`: transparent, left-facing combat models generated from the confrontation sheet as an identity reference. Full weapons and feet remain in frame.

`../prepare_art.py` crops and resizes the generated pixels into 21 scene tiles and three alpha sprites, retained in `../assets`, and packs `../source/rift170_art.h`. Scene tiles use RGB565; combat sprites retain RGBA. Runtime rendering uses the bounded texture cache. These are detailed gothic paintings, with no pixelation filter or decorative-line replacement art.
