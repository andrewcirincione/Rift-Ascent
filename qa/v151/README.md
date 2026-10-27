# Rift Ascent 1.5.1

Canonical source is `source/`. The complete release keeps all five soundtrack tracks, the animated home-screen icon, RiftBoss home-screen music and the existing wallpaper.

## Changes

- Entirely new white-caped, white-hooded, crouched Nightshade sprites for idle, attack, block, power and death. Shared hip and ground anchors keep the model stable. Attack uses fewer poses; Warden retains the original duration with a quick downstroke, longer sword-down follow-through and shorter recovery.
- Smaller combat Victory art; restrained generated mystery/inventory selection tablet; inset potion icons; ten generated relic emblems.
- Start opens Status / Inventory on combat, reward, shop, map, mystery, campfire and related browse screens. Left/right changes tabs; Square opens Deck; Triangle discards the selected potion in the Potions tab. Underlying menu cursors and pending decisions remain intact. Mandatory card-effect choices cannot be bypassed.
- Card and potion rewards are independent. Either may be taken first, skipped, or both declined. A full belt leaves the offer pending while inventory frees a slot. Hunt bonus card rewards never repeat the potion grant.
- Upgrade previews show the actual upgraded card, its Energy badge and the current-to-upgraded cost, including Echo and Fleeting. Removed redundant one-modification-per-card wording.
- Save version 22 appends reward and relic counters. Prior v21 saves migrate without duplicating previously granted potions; older migrations remain supported.

## New shared relics

| Relic | Effect |
|---|---|
| Bloodglass Phial | First self-inflicted HP loss on your turn grants 1 Energy and 4 Block. |
| Quiet Bell | No Attacks played by turn end grants 1 extra Energy next turn. |
| Twinfang Brooch | Every third Attack in a turn applies 1 Poison to all enemies. |
| Sepulchral Spindle | First three exhausted cards in combat grant 1 Energy and draw 2. |
| Hollow Chalice | After combat, heal 2 per Power played, capped at 6. |
| Mourner’s Lens | First card effect each turn adding enemy Weak, Vulnerable or Poison draws 1. |
| Briar Rosary | At least 20 Block at turn end applies 2 Poison to all enemies. Includes Plating. |
| Grave Ledger | Skip a card reward for 8 Gold; every third skip upgrades a random unupgraded card. |
| Thornbound Seal | Start combat with 1 Strength per curse/status in the permanent deck. |
| Prism Bone | First Attack + Skill + Power combination in combat grants 2 Energy and draws 2. |

New relic triggers stack with existing relics and powers. Draw/Energy effects that depend on a mandatory Warden choice settle after that choice, without invalidating the picker or firing twice.

## Build and validation

```bash
export PSPDEV=/path/to/pspdev
bash qa/v151/build_release.sh versions/1.5b-r2/EBOOT.PBP qa/v151/evidence/EBOOT.PBP
python qa/v151/verify_media.py
python qa/v151/test_native.py
```

`prepare_art.py` emits `rift151_art.h` from the new original atlases; old Nightshade sprites are not referenced by the renderer. Previous card illustrations remain unchanged. One-time implementation scripts are development records, not rebuild entry points.

Native evidence: 1,522 complete upgrade/inventory layouts (maximum height 149 pixels), ASan/UBSan checks, 756 valid upgrade/modification cost combinations (62 cost changes), independent new-relic trigger tests and choice deferrals, all reward take/skip orders, full-belt recovery, read-only menu controls, 174 Nightshade card resolutions, 15,952 independent Nightshade arithmetic cases, 41,000 Warden arithmetic/resolution/RNG cases, 18,000 class/rarity reward draws, real legacy save migration, 1,485 shared-relic singles/pairs, 20,400 event choices, 1,000 shops, 160,000 mixed-deck actions and controlled 100-act progression for each character.

Three thousand automated complete runs executed 2,372,229 actions and 82,907 exact save/resume continuations without stalls, invalid states, cross-character reward leaks or continuation divergence. Warden policy win rates were 31.0–42.6%; Nightshade 7.4–11.6%. These fixed heuristic policies are functionality tests, not human win-rate measurements or proof of equal character balance; no new difficulty adjustment was requested for this patch.

PPSSPP validation uses the complete packaged EBOOT, actual controller inputs, native 480×272 screenshots, and saved state fixtures for targeted scenarios. `qa_ppsspp.py` tests inventory, independent rewards, full belts, forced-choice guards and actual upgrades; `qa_ppsspp_animation.py` captures complete rendered combat timelines. The final logs record the tested SHA-256.
