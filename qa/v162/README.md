# Rift Ascent 1.6.2

Canonical source: `qa/v162/source`. Save format remains v23. This patch retains the physical-PSP-confirmed 1.6-r2 loader correction, 32 MiB PSP target and fixed 4 MiB artwork cache.

## Presentation and world

The main menu establishes one towering rift: burning red foundations, misty blue middle reaches and an ivory/gold summit. Each act has a newly generated introduction, map, two normal arenas, two elite arenas, a unique boss arena, campfire and shop. Endless has a fourth introduction. Fight variants alternate with floor/lane so routes encounter different scenes. Ninety-six new Warden illustrations replace low-resolution card art. See `art/world-prompts.json` for exact generation prompts and `art/README.md` for processing details.

Warden's anchored idle is 33% faster. Status icons have soft dark separation. A generated red/gold glowing hand arrow overlaps the outer card. Selected cards lift and highlight consistently, including inventory, reward and modification grids; character and blessing selections also lift. Full previews are taller, adapt width for long descriptions and remain at most 166 pixels high. The inventory deck uses a three-column, two-row grid with shoulder-button tabs. Pause uses a quiet generated panel and the established menu buttons.

## Gameplay and reliability

Spoils now presents cards, an independent TAKE POTION action, and centered LEAVE beneath. Taking either reward does not forfeit the other. A full belt preserves the offer while Triangle opens potions for discarding. Triangle also opens potions in combat and at campfires. D-pad navigation follows the vertical arrangement and clamps into short last rows.

Newly generated act maps guarantee three rests on every route, with at most one optional fourth: the first by floor six, no more than six intervening rooms. Events use a shuffled 17-event bag and avoid consecutive modification rooms. Act-start choices draw from 58 eligible relics across dependable, general and build-focused pools; character restrictions remain enforced. Proxy's Stars gains have a dedicated glassy sound voice. The unnecessary adaptation labels are removed from card rules.

Artwork reads validate CRC before entering the cache. A stale/failed file handle is reopened and retried automatically, with a clean framebuffer fallback if the source stays unavailable. Native fault injection and PPSSPP forced-handle recovery verify this failure mode. The user's physical-PSP graphical incident has not been independently reproduced on hardware, so a hardware confirmation remains useful.

## Validation and rebuild

`evidence/final-tests.txt` records the final binary and completed checks. Native coverage includes 115,914 routes, 3,000 complete event bags, 27,000 blessing selections, 6,764 Regent assertions, every legal card/modification/upgraded preview, sanitizer UI checks, 1,275 exact texture comparisons and 2,032 identical resident-versus-streamed rendered cases. Emulator drivers use the complete EBOOT in 32 MiB PSP mode, real controller inputs and native PPSSPP screenshots. Injected fixtures only establish reproducible starting states.

From the repository root, with PSPDEV set and Python/Pillow/numpy available:

```sh
bash qa/v162/build_release.sh
```

This mirrors canonical source to GAME/src and portable_build, builds the corrected CRT-free executable, appends the verified artwork bank and preserves all five music streams plus the RiftBoss XMB audio. `prepare_art.py` crops/resizes generated paintings; it does not draw replacement art.
