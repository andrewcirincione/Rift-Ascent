# Rift Ascent 1.4.0

Complete PSP EBOOT with all five music tracks. Keep SAVE.DAT and PROFILE.DAT when replacing your game.

- Stable title logo on black with animated flames in the home-screen icon. The splash wallpaper and RiftBoss home-screen audio are preserved.
- New painted Victory artwork and a brief bell chime. The final-hit slow motion and short reward transition remain.
- Potion rewards and shop purchases show a potion icon, name and description. Violet Orb lists its exact bonuses, and potion descriptions display enhanced values when owned.
- Open the potion belt with Triangle from the map or shop. Healing potions can be used there; combat potions remain available for fights. Full-health healing is not wasted outside combat.
- The shop shows the belt's three-slot capacity and a clear full-belt state. Purchases preserve existing potions, including duplicates. A full belt is never charged.
- Down navigation now reaches a short final card row from any column in upgrade, removal and card-choice screens.
- Expanded Bully/Bully+ and Ashen Strike descriptions explain their Vulnerable-duration and current Exhaust-pile bonuses. Combat previews show the final damage and the current count.
- New six-pose heavy greatsword animation with a larger wind-up, full cut and follow-through. Attack timing is unchanged, and boot anchors and the complete blade stay in frame.
- Unaffordable cards dim, show a depleted grey energy jewel and display the exact missing Energy amount. Free and X-cost cards remain available.
- Rewards and shops give unowned cards three times the weight of owned cards within the same rarity tier. Duplicates remain possible; upgraded copies count as owned. Shop offers contain three distinct cards.
- Newly generated maps keep their planar routes but make every fork offer different room types within two floors, before merging. Utility spacing, shop availability and the final campfire are preserved.
- Enemy idle breathing is restored with fixed ground anchors. Rift Channeler now faces the player. The player stands three pixels lower, and Power casting uses three anchored poses over four rendered frames.
- Quieter inventory selection artwork, unobstructed potion text, and stronger campfire selection feedback.
- Hand navigation arrows overlap the edge cards slightly. Preview energy badges are slightly larger, and card art crops favor the top to preserve faces.
- New complete relic-chest map artwork with uncropped edges.
- Seer's Oath was identical to Old Compass. Old Compass is retained; existing saves consolidate the duplicate without shifting other relic IDs.
- Relic stacking audited across all 630 distinct pairs and all 36 relics together. Rust Amulet adds to existing Weak once per target on the first Attack; attack-only relics no longer trigger from Fire potions.
- Corrected 15 card rarity assignments. Act bosses offer three distinct Rare cards, with explicit rarity labels. Pending boss rewards in existing saves are repaired when needed.
- A completed normal run offers Continue Endless or Return to Menu. Continuing heals you and starts Act IV with your deck, relics, potions and gold intact, followed by the next blessing choice.

Validation: native AddressSanitizer/UndefinedBehaviorSanitizer regression suite; 81,312 attack/Whirlwind combinations; 480 Bully and 160 Ashen Strike cases; 720,000 weighted selections; 40,000 maps / 120,000 distinct forks; 57,847 grid positions; 21,000 Rare boss offers; 6,000 potion purchases and 6,000 consume/refill cases. The exact release EBOOT was tested in PPSSPP 1.20.4 with controller navigation, real shop purchases, map healing, both normal-run ending choices, animations, final-hit music pause, Victory/chime and reward flow. Combat.png is a PPSSPP screenshot. Physical PSP hardware was not available for testing; ICON1 was encoded to PSP constraints and decoded/validated separately.

Full source, generated art, reproducible tests and validation evidence are included in the repository and source archive.
