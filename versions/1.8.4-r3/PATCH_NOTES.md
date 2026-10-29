# Rift Ascent 1.8.4 — map, card-browser and combat fixes

Release tag `v1.8.4-r3`; the in-game version remains 1.8.4.

- Alternate map branches now offer different room sequences before reconnecting. Opening paths also differ by their second room. New maps retain two or three live routes, recurring forks, and commitments of at least two rooms. Across 70,000 maps, all 629,238 tested forks were distinct; a separate topology check retains at least four forks on every route.
- Room totals for each act are unchanged. Shops, treasures and guaranteed camps retain their floor distribution. Where an entire branch window was identical, nearby combat/mystery rooms exchange positions without adding rooms or introducing new early elites. Existing saved acts retain their current layout; the improved generation applies to new runs and subsequent acts.
- Fixed missing artwork on upgraded cards in the Custom Run Your Deck list. Thumbnail lookups now strip card flags before selecting artwork. Base and upgraded copies keep their own quantities and behavior.
- Character card catalogs in Unlocks and Custom Run are ordered Attack, Skill, then Power, with a consistent order within each type. Other card types follow. The separate base-game card catalogs use the same order. Your Deck retains the order in which cards were added.
- Unlock visibility follows the actual card ID after sorting, preserving account progress and preventing locked cards from appearing available.
- Fixed Afterimage failing to grant Block when playing Warden, Proxy or colorless cards. Its effect now covers every card family, including mixed custom decks, replays and autoplay. Stacked copies, later turns and combat saves retain the correct effect.
- Enemy attack damage is increased by 6%, rounded to the nearest point per hit. Small hits below 9 damage remain unchanged; enemy health, Block and healing are unchanged.
- Pyre Castellan gains Cinder Guard: Block plus a Burn in your discard pile. Storm Golem gains Static Crush: two small hits and one less Energy on your next turn. These replace one move in each elite's existing rotation and have complete overhead intentions and descriptions.
- All existing artwork, twelve complete music tracks and splash media are preserved byte-for-byte. Card costs and effects are otherwise unchanged.

Install: replace EBOOT.PBP in PSP/GAME/RIFTASCENT and keep all existing save/account files. Validation uses native regression tests and the exact complete EBOOT in PPSSPP 1.20.4; physical PSP testing remains unverified.

The source ZIP contains the current source and required build inputs. Older compiled releases remain in Git history and their individual release downloads.
