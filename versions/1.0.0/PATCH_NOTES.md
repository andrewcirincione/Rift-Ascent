# Rift Ascent 1.0.0 — baseline

- **Source:** `c00775d0c1e26ebe69d8ab12e41798cf70a4b606` on `main`
- **CI:** https://github.com/andrewcirincione/Rift-Ascent/actions/runs/39964760951
- **Binary:** `EBOOT.PBP` archived here from the exact successful CI artifact
- **Build status:** PSPDEV compiled, PPSSPP smoke tested and passed.
- **Hardware status:** The earlier CRT-free double-buffered gameplay build was physically confirmed on a PSP-3000/6.60/ARK-5. This exact 1.0.0 gameplay-overhaul EBOOT is *not* proven on hardware.

## Features
- Complete PSP roguelike prototype, 3 acts, enemy combat, deck/reward choices, maps, events and shops.
- Original sprite artwork; readable antialiasing; 4-tab combat glossary and status effects.
- CRT-free user-mode startup, firmware 6.60 target, VBlank-synchronized double buffers.
- Branching procedural map, original procedural soundtrack and action sound effects.
- Original save format (GAME_VERSION=16).

## Known issues reported for 1.0.1
Card descriptions are sometimes clipped; battle intent can omit effects or show the wrong incoming damage; basic cards can appear as rewards; forge event upgrades without card selection; already-upgraded cards can be selected; elite relic rewards and relic ledger are missing; map/shop/UI/animation and balance need work.

**Safety:** Keep the known-working hardware EBOOT as a backup. PPSSPP cannot guarantee ARK-5 compatibility.
