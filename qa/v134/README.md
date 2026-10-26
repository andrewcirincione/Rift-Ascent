# Rift Ascent 1.3.4

A complete generated-art UI pass and combat presentation repair, retaining 1.3.3 gameplay and save format 18.

- New painted main-menu wordmark, opaque hand surface, sculpted menu panels, health frames, sapphire energy orb, four card frames, map icons, twelve event illustrations, target crest and six attack frames.
- Native antialiased typography, large energy numerals, full card titles, central art crops, measured text wrapping and separate Attack / Skill / Power / curse palettes. Card paintings are preserved; images never overwrite trim.
- A compact 17-pixel top HUD. Player and enemy health/block at y=32, intents at y=57, characters beginning at y=80, opaque hand area at y=174. Selected card previews remain below all health and intent elements.
- Played cards travel from their actual hand slot to the screen center, hold briefly and alpha-fade back into the scene. Resolution waits for the ascent; input cooldown protects the animation.
- Independent metallic block audio mixes with simultaneous attacks and cannot be replaced by cursor clicks. Block gain and blocked impacts both trigger it.
- Generated sword/impact/ember animation replaces geometric slash marks. Ember Seer faces toward the player.
- Redesigned shop, deck, inventory, potions, relics, reward, rest, event, help, pause and outcome screens. Prior store deck/potion access, relic skipping, status scrolling and 15-floor planar maps are retained.

## Build

Requirements: Python 3 with Pillow, C compiler for native QA, PSPSDK on PATH. The fonts and resolved assets are included for an offline rebuild.

```
python3 qa/v134/build_source.py
make -C portable_build -j2
python3 qa/v115/import_114_art.py GAME/src portable_build/EBOOT.PBP
python3 tools/audit_psp_imports.py portable_build/EBOOT.PBP
python3 GAME/verify_eboot.py portable_build/EBOOT.PBP
python3 qa/v134/pack_music.py /path/to/previous/full/EBOOT.PBP portable_build/EBOOT.PBP EBOOT.PBP
```

Keep private soundtrack data out of public source control. The private packaged EBOOT already contains all three unchanged tracks.

## Validation

Native QA uses AddressSanitizer and UndefinedBehaviorSanitizer, with leak detection disabled because the host does not support LeakSanitizer. It covers 2,000 generated maps, 45-floor progression, save/load, shop inspection/removal, relic take/skip, status enumeration, damage audio and simultaneous block/attack mixing. It renders every base card, verifies all base/upgraded descriptions fit the large preview and every title fits the hand thumbnail, renders all menus and twelve events, and captures the card animation timeline.

```
mkdir -p ../evidence134
cc -O1 -g -fsanitize=address,undefined -I GAME/src qa/v134/qa.c GAME/src/game.c GAME/src/artwork.c GAME/src/enemy_art.c GAME/src/motion_art.c GAME/src/sound.c -o ../evidence134/qa
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 ../evidence134/qa
```

The native QA includes render.c intentionally to inspect renderer layout and reset transient visual state. Do not also link render.c into that QA executable.
