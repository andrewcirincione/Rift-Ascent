# Rift Ascent 1.6

- Added **Proxy**, the blue-cloaked wizard king, with a large golden Sovereign Blade and new idle, attack, guard, casting, hit and death sprites.
- Implemented the Regent wiki catalog: 96 cards/tokens including all base/upgraded forms, Stars, Forge, minions, replays, retention, delayed effects and card-selection effects. Added 53 supporting Colorless cards and 149 original generated card illustrations. The five multiplayer-only Regent cards have labeled, obtainable solo adaptations.
- Proxy alone receives a Stars HUD and Star-cost badges. Stars persist between turns; gold star particles intensify up to 15 Stars. Damage and Block previews account for active effects and repeated plays.
- Renamed Nightshade to **Vesper** and replaced every sprite with a smaller, grittier, crouched model retaining her white hood and cloak. Existing saves keep the same character.
- Pause uses matching X, Circle and Select icons. Save & Quit returns to the title screen. Resume Run loads an unfinished save and is greyed out when none is available. Mid-card selections survive saving without duplicating payments or effects.
- Audited every enemy's facing and checked character grounding, weapon bounds, animation frames, card layout and UI overlap.

Validation includes all 298 new base/upgraded card resolutions, 6,764 Proxy assertions, 2,000 independent Proxy damage cases, 18,000 reward draws, 2,736 card layouts, existing Warden/Vesper arithmetic and control regressions, old-save migration and 300 automated runs without stalls. The exact complete EBOOT was tested in PPSSPP with controller inputs and screenshots, including save/quit/resume and animation sequences. Automated runs are functionality checks, not measured human win rates. Physical PSP hardware was not available.

Card mechanics use the wiki's v0.107.1 snapshot. Rift Ascent's encounters, shared relics and potion pool remain its own. The complete five-track soundtrack and RiftBoss home-screen music are included.

Replace `PSP/GAME/RIFTASCENT/EBOOT.PBP`. Keep `SAVE.DAT` and `PROFILE.DAT`; previous saves migrate automatically. Source and validation details are under `qa/v160`.
