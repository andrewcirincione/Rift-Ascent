# Rift Ascent 1.8 Pre-release — The Last Gate

This is the isolated prerelease of the recovered **A1.8** development line. The PSP home-screen title is `RIFT ASCENT 1.8 PRE-RELEASE`; the internal development identifier is A1.8. It is not a replacement for the stable 1.71 release. Only the `1.8-Pre-release` branch and this new prerelease are written by its publication workflow; `main`, A1.8, other version branches and earlier releases are left unchanged.

## Install

Download `Rift-Ascent-1.8-Pre-release-PSP.zip` and copy its `PSP/GAME/RIFTASCENT_A18` folder to your memory stick. The complete game is the single `EBOOT.PBP` inside that folder. Keep that exact folder name. The prerelease uses its own save/profile filenames and install folder, so it does not overwrite another version's saves. Back up your memory stick before testing any prerelease. New prerelease slots and progression are local to this install; an old save is not guaranteed to have compatible new unlock receipts.

The standalone EBOOT contains all artwork, the existing soundtrack and the original RiftBoss XMB audio. No external artwork or music files are required.

## Added

- Four independent run slots, overwrite confirmation, dual-generation recovery files, and a shared account unlock profile.
- 60 custom character cards: 20 each for Warden, Vesper and Proxy. Each set has six Common, six Uncommon, six Rare and two Ancient cards. Every new card has its own AI-generated painting.
- 20 new potions and 10 new relics with actual effects and generated artwork.
- Act 3 completion awards two previously locked cards of different rarities for that character, two previously locked potions and one previously locked relic, until the respective pools are complete. Separate card, potion and relic pages present the awards after a character-specific ending scene.
- New content becomes eligible in future runs. Each in-progress run retains its start-of-run unlock snapshot.
- Character records for Act 3 wins, furthest cleared act and separate Act 4 clears. The tenth Act 3 win completes that character's 20-card set and permanently unlocks that character's secret Act 4 encounter.
- Character-specific revelation, secret-boss introduction and true-ending scenes. Clearing the secret Act 4 boss grants that character the golden A. Act 4 clears increment separately and do not grant a second Act 3 win or repeat that run's unlock awards. Save/reload receipts guard against duplicate completion awards.
- Main-menu unlock counts and a collection browser for all new cards, potions and relics.
- Restored multi-enemy fight frequency, distinct enemy pairs, and delayed heavy opening attacks.
- Existing post-victory hold/fade timing retained without the victory banner or fanfare.
- A shared actual AI-painted purple outer-space arena for Act 4 and beyond, including normal, elite and secret-boss fights.

## Validation and limitations

The publication workflow rebuilds from recovered, checksum-verified source with pinned PSPDEV, reruns native regression tests under AddressSanitizer/UndefinedBehaviorSanitizer, checks every appended art texture, preserves and validates the original soundtrack/XMB audio, and exercises the exact packaged EBOOT in PPSSPP. Runtime screenshots and logs are attached as evidence.

Progression/Act 4 runtime checks use explicitly assisted saved-game fixtures. They verify real PSP executable screen transitions, account records and asset loading; they are **not** unassisted balance playthroughs. Native tests additionally cover the 60 cards, upgrades/modifications, potion effects, rarity distribution, 900 character progression sequences, four save slots and corruption recovery. Emulator sound is routed to a null output for automation: no claim of human audible or new physical-PSP validation is made.

This remains a **prerelease**: full-run balance, exhaustive menu/layout review, every possible cross-card interaction and physical PSP acceptance testing are not claimed complete. Earlier builds have been playtested on native hardware by the owner, but that is not verification of this new binary.

## Reproducibility

The branch retains readable source, original generated paintings and manifests, the packing scripts, complete tested EBOOT, checksum and validation evidence under `prerelease/1.8/`. `PACKAGE_AUDIT.json` identifies the exact binary. The source archive includes the base package needed to preserve the existing music and artwork. Published binary digests are checked before the draft is made public. Existing release tags are never overwritten, and this prerelease is not marked as the repository's latest stable release.
