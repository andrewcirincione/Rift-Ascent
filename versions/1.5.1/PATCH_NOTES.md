# Rift Ascent 1.5.1

- Rebuilt every Nightshade sprite with a white hood/cape and a crouched silhouette. Shortened her attack pose sequence and stabilized the model with fixed body/ground anchors.
- Retimed Warden's attack: quick lift/downstroke, longer sword-down follow-through, short recovery, same overall duration.
- Reduced the combat Victory artwork size.
- Added ten shared relics with generated artwork: Bloodglass Phial, Quiet Bell, Twinfang Brooch, Sepulchral Spindle, Hollow Chalice, Mourner's Lens, Briar Rosary, Grave Ledger, Thornbound Seal and Prism Bone. Effects support self-damage, poison, exhaust, Block, Powers, mixed card types and skipping rewards. Existing relic/power effects stack with them.
- Start opens Status / Inventory from combat, rewards, shop, map, mystery and campfire screens. Deck and relic tabs are available throughout. Triangle in the Potions tab discards the selected potion; Square opens Deck. Browsing preserves the underlying menu and mandatory card choices remain protected.
- Card and potion rewards now have independent take/leave choices. Take either first, both, or neither. If the belt is full, discard a potion in inventory and collect the pending offer. Bonus card rewards do not duplicate potions.
- Replaced mystery and inventory selection artwork with a quieter gothic tablet, kept text/icons clear of its ornaments, and moved the reward potion icon inward.
- Upgrade previews show the upgraded card's actual Energy badge and a current-to-upgraded cost label, including modified cards. Removed redundant one-modification-per-card wording.
- Retains the character-pool corrections published in v1.5b-r2.

Validation includes 756 upgrade/modification cost combinations; 15,952 Nightshade and 41,000 Warden arithmetic/resolution cases; 1,485 shared-relic combinations; independent reward/control/save tests; and 3,000 automated full runs with 82,907 exact save/resume continuations. The complete EBOOT was checked in PPSSPP with controller inputs and native screenshots. Automated policy runs are functionality checks, not measured human win rates or proof of equal character balance. Physical PSP hardware was not available for validation.

The complete five-track soundtrack and RiftBoss home-screen music are included. Replace `PSP/GAME/RIFTASCENT/EBOOT.PBP` and keep `SAVE.DAT` and `PROFILE.DAT`; previous saves migrate automatically.
