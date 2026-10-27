# Rift Ascent 1.6 validation and build

Canonical source: `qa/v160/source`. Public character names are **Proxy** and **Vesper**; stable internal enum names and old card IDs remain unchanged to migrate prior saves.

## Content

Proxy implements the Regent catalog from the STS2 wiki's v0.107.1 data snapshot: **96 entries** (85 reward cards, four basics, two Ancient cards, four summoned tokens and Debris). The 85 include Constellation, Largesse, Plot, Tutor and Hammer Time with explicitly labeled solo adaptations, since Rift Ascent has no multiplayer. They are obtainable at their normal rarity. Ancient gifts offer Meteor Shower or The Sealed Throne at later act starts. The complete **53-card solo Colorless pool** supports card-generation effects. Potions, shared relics, encounters and turn timing use Rift Ascent's existing systems.

Reference records are preserved in `reference/regent.json` and `reference/colorless.json`, sourced from https://sts.wiki/assets/data.fe7de9cb.js. Mechanics were cross-checked against individual wiki pages, including Void Form, Black Hole, Crescent Spear, Conqueror, Monologue, Gold Axe, Seeker Strike, Stratagem, Glitterstream, Fisticuffs and Omnislice. Future wiki balance updates do not silently alter this build.

Forty newly generated character frames, 149 new card illustrations and four UI icons are preserved in `art/` and prepared by `prepare_art.py`. Vesper has a smaller, crouched, weathered white-cloak silhouette. Proxy wears blue with a large gold-glowing sword. Golden particle count increases to 30 at 15 Stars and stays capped beyond 15. Other characters keep their existing resource layout.

## Validation

- `smoke.c`: all 149 new cards, base and upgraded, resolve followed by three subsequent turns under AddressSanitizer/UndefinedBehaviorSanitizer (298 cases).
- `regent_audit.c`: 6,764 assertions, including 2,000 independently calculated attack cases, wiki Energy/Star costs, pure previews, single physical Sovereign Blade, Forge/Vigor/Conqueror/Seeking Edge/Parry/Sword Sage stacking, X payments, replay payments, free manual plays, Void Form stacks, mixed-character power hooks, delayed Block/Dexterity, creation capacity, draw scaling, mandatory choices and save/resume.
- `pools.c`: 18,000 Proxy reward draws; all 85 reward cards reached; bosses offer rare cards; normal combat excludes rares; Ancient gifts reachable.
- Prior-character regression: 15,952 Vesper and 41,000 Warden arithmetic/resolution cases; existing relic/reward/menu/upgrade tests, plus 174 base/upgraded Vesper resolutions.
- Save compatibility: real v22 pending-Sly snapshot with instance tag 900 and upgrade/modification/free/Sly flags migrates into v23 and resolves; older v16–v21 fixtures and exact v23 roundtrips also pass.
- Layout: 2,736 card upgrade/modification layouts; maximum upgrade/inventory card height 149 px. Another 1,214 live descriptions with large modifiers fit within 157 px. Native animation timelines and audio-contact checks pass. All 44 enemy-facing screenshots were reviewed: directional silhouettes point toward the player; front-facing models retain their intended pose. No additional reversed enemy was found.
- Automated policy runs: 100 per character, no stalled runs or invalid live states. Wins: Warden 38, Vesper 12, Proxy 6. This is a shared heuristic policy, not human win-rate data or evidence of equal balance; it is considerably less capable with setup/generation decks.
- `qa_ppsspp.py`: exact complete EBOOT, real controller input, native PPSSPP screenshots, disabled Resume, all three character choices, Star affordability/payment, upgrade costs, pause icons, Save & Quit to title, exact in-session/cold Resume and protected mandatory choices.
- `qa_ppsspp_animation.py`: actual-input Vesper/Proxy attacks, Block, Powers, finisher and death captures. Screenshot key injection is sequenced after the controller press to avoid synthetic-key interference.
- Complete PBP media audit: all five soundtrack payloads preserved, RiftBoss home-screen audio preserved, animated icon and wallpaper preserved, soundtrack CRC verified. PSP imports audited. Physical PSP hardware was not available.

## Rebuild

Use a PSP SDK in `PSPDEV` and Python with Pillow, numpy and scipy. From the repository root:

```sh
bash qa/v160/build_release.sh versions/1.5.1/EBOOT.PBP qa/v160/evidence/EBOOT.PBP
```

`build_source.py` mirrors canonical source into `GAME/src` and `portable_build`. `prepare_art.py` reproduces the indexed asset header. The other one-time implementation scripts document development; do not rerun them on the completed source. `run_native.py` runs regression checks; the other C harnesses use the current source. PPSSPP scripts require the runtime layout described in their imports and the fixtures emitted by `ui_fixtures.c`.
