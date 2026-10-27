# Generated art provenance

Mode: built-in image generation. Exact prompts are retained in `world-prompts.json` (all jobs) and `prompts.json` (initial jobs).

- `rift-menu-world.png`: megalithic view of the full three-tier rift with Warden in the foreground.
- `rift-act-1-world.png`, `rift-act-2-world.png`, `rift-act-3-world.png`: each is a 3x3 atlas. Row-major cells: introduction, map, normal A, normal B, elite A, elite B, boss, campfire, shop.
- `act-endless.png`: endless introduction.
- `warden-cards-0.png` through `warden-cards-5.png`: six 4x4 atlases containing 96 distinct illustrations.
- `hand-arrow.png`: transparent red enamel/gold glowing forged arrow.
- `pause-panel.png`: transparent subdued gothic panel.

`../prepare_art.py` crops cells, resizes the actual generated pixels and packs RGB565/alpha data into `../source/rift162_art.h`. Runtime crops are retained in `../assets`. `../pack_assets.py` streams these through the bounded artwork cache. No decorative-line substitutes are used. The first-pass act illustrations were superseded by the coherent world atlases before release.
