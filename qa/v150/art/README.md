# Original 1.5 artwork

Generated with the built-in image-generation tool and the imagegen skill for this project. Exact prompts are retained in `prompts.json`.

- `event-relic-atlas.png`: five new mystery-room objects and four individual relic artifacts.
- `modification-seals.png`: six distinct embossed seals for the hand, deck and card preview.
- `enemies.png`: eight new left-facing, full-body gothic enemy sprites.
- `cards.png`: four original illustrations for Grave Cleaver, Dusk Guard, Mercy's Edge and Black Covenant.

`prepare_art.py` preserves alpha, extracts complete enemy components, adds safe transparent margins to icons and packs the runtime headers. It does not redraw the art. Inspect `evidence/enemy-art-bounds.json` for the complete source bounds and sprite sizes.
