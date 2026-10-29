# Rift Ascent 1.8.4 — revised build

This remains version 1.8.4 in game. Release tag `v1.8.4-r2` preserves the original published build.

- Restored sustained map choices. Each act keeps two or three live routes, with recurring forks and joins, varied opening lanes, and branches that stay separate for at least two rooms before reconnecting. Room counts on each floor retain the previous distribution; rooms can exchange lanes to make otherwise identical branches offer a different choice. Existing saved rooms and visited edges remain intact.
- Added the supplied RiftMain as the main menu theme and RiftAbomination for all three characters' secret Act 4 boss encounters. All other existing music routes and the RiftBoss splash audio are preserved. Both new songs are included in full.
- Raised the main menu logo slightly and added card names to the small unlock tiles. The large card preview retains its layout.
- Custom Run card catalogs now respect account unlocks for each character. Locked cards cannot be added or launched through an old draft; those drafts remain editable so the cards can be removed.
- Custom Run card rewards draw from the character families represented in the current permanent deck, including upgraded cards. Removing the last card from a family removes that family from subsequent offers. Eligible Rare and Ancient boss rewards remain available; starter and generated Proxy cards are excluded from new reward offers.
- Back to Loadout now returns to Your Deck before the item-selection screen. Removed the extra selected-row X from the pause menu; the confirmation footer remains.
- Reviewed all 60 new unlock cards and their upgrades. Adjusted 16 cards to address repeatable free resource loops, healing loops and unusually generous combined effects. The Last Decree receives a lower Energy cost, and Threadcutter now requires an actual discard before granting Energy. Concepts, artwork, rarities and unlock progression are retained. Enemy difficulty is unchanged. Detailed values and rationale are in `qa/v184r2/BALANCE_REVIEW.md`.

Validation includes native sanitizer checks, 10,000 generated maps, 37,800 custom reward offers, 240 new-card combat cases, and targeted controller-driven playtests of the exact complete EBOOT in PPSSPP 1.20.4. Physical PSP testing remains unverified.

Install: replace EBOOT.PBP in PSP/GAME/RIFTASCENT. Keep SAVE.DAT, PROFILE.DAT and every ASCENT18_* file to preserve account progress, active slots, settings and retired-run receipts.
