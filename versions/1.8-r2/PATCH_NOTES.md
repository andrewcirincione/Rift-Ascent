# Rift Ascent 1.8 — fresh 1.71-r2 rebuild

This overhaul starts from v1.71-r2 (16717db18a040f717acce2e63529621554cf0439). No code, artwork, dialogue or content from the previous 1.8 prerelease was reused. The new release tag is v1.8-r2; the in-game version remains 1.8. Existing published tags and repository history are preserved.

## Runs, unlocks and endings

- Four independent run saves, with shared account-wide unlocks and character records. Checksummed alternating snapshots recover from an interrupted write. Valid stable SAVE.DAT runs import into slot 1 once; the original file remains intact.
- Each character has 20 new custom cards: six common, six uncommon, six rare and two ancient. Completing Act 3 unlocks two different random rarities without duplicates; the pairing guarantees the complete character set on the tenth win. Unlocked cards enter future runs' obtainable pools.
- Twenty new potions and ten new relics. Each Act 3 completion unlocks two remaining random potions and one remaining random relic until their collections are complete.
- Character-specific Act 3 ending, then separate Cards → Potions → Relic unlock screens. Main menu counts show each character's cards and the shared potion/relic totals; the Unlocks menu opens the collection browser.
- Character selection shows fire-marked wins, best cleared act and a golden A after the true boss victory.
- Before a character has all 20 cards, Act 3 still offers Endless. On the tenth win and every later run with that character, a revelation leads directly into the fourth act. A character-specific scene introduces the Unmaker, another follows the victory, and only then does Endless become available. Other characters' progress does not unlock this route.

## Combat, presentation and items

- Normal fights have a 25% chance of two enemies. Heavy opening attacks are usually deferred beyond round one. Act 4 and later normal enemies draw from the wider roster.
- Removed the victory banner and winning sound while preserving the existing finishing animation and delay before spoils.
- Added 112 original painted assets: 60 card illustrations, 20 potion bottles, ten relic icons, twelve character scenes, four fourth-act environments, the secret boss, three refreshed enemies and two record badges.
- The oldest forty relics now share 22 different themed icons from the expanded library. The six original potions each share a suitable new bottle. Actual item artwork is consistent in shops, rewards and inventories; new items no longer stand apart from a largely generic old collection.
- Refreshed Mist Gazer, Mire Oracle and Void Disciple; corrected Rift Behemoth and Infernal Colossus facing.
- Health fills remain proportional and inside their frames as maximum HP grows. Potions consistently precede Deck in gameplay navigation.
- Moon Spring heals a flat 20 HP for free, or accepts 35 Gold for +6 maximum HP and 6 healing.
- Options replaces How to Play. Enable Debug there to use L to defeat current enemies or R to advance an act during play. Debug remains off by default and is saved globally.

## Soundtrack

All ten tracks come from the supplied updated Soundtrack(1).zip. RiftBoss replaces the old track in the PSP splash, title menu and boss encounters; gameplay uses stereo 44.1 kHz for that track. The other streams use stereo 22.05 kHz.

| Context | Tracks |
| --- | --- |
| Title and boss fights | Updated RiftBoss |
| Act 1 menus | RiftMenu |
| Act 2 menus | RiftMenu2 |
| Act 3 menus | RiftMenu3 |
| Act 4 onward menus | RiftMenu4 |
| Normal fights | RiftFight, updated RiftFight3, RiftFight5 |
| Elite fights | RiftFight2, RiftFight4 |

## Installation and verification

Extract the PSP ZIP to the Memory Stick root, or place the complete EBOOT.PBP in PSP/GAME/RIFTASCENT/. Keep all existing save/profile files and ASCENT18 files. The download includes streamed artwork and all music; it is not an engine-only build.

Validation includes native address/undefined-behavior sanitizers, save recovery and migration, every new card/potion/relic, unlock schedules, and the exact packaged EBOOT in PPSSPP 1.20.4 using 32 MiB PSP mode. Late-game emulator checks use explicit nine-win state fixtures followed by real game inputs; they do not represent thirty unassisted playthroughs. Physical PSP verification is still pending. Full evidence and checksums accompany the source.
