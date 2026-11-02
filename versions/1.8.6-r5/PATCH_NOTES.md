# Rift Ascent 1.8.6-r5 — main edition

- The Rift Abomination encounter, its two minions, arena and before/after scenes now occur in Act 6. Act 7 and subsequent acts use the random boss rotation. The full supplied RiftAbomination track plays during that special fight.
- Act-start blessings now offer deck and rule changes. Examples include full unused-Energy carryover, hand retention, free unremovable Strikes, cross-character rewards, starter-relic upgrades and attack replays. Act 1 uses opening boons; greater Ancients arrive in later acts.
- The researched stable STS2 catalog contains 296 relics, now represented in the main edition. Rift-specific adaptations are labelled in their descriptions; the two beta-only entries are outside this baseline. Loot, shop, event and Ancient pools remain separate. Relics can interact and custom-run duplicates scale their numeric effects.
- Card rewards softly favor cards not previously offered in the current run, within the same rarity. Skipped cards count as seen. History survives saving and changing acts.
- Added relic-dependent camp options and expanded potion capacity, with scrolling inventory and deck access during boon selections.
- Fixed a replayed Void Form turn-ending deadlock, a three-enemy debuff snapshot overrun, and relic interactions involving gold awards, potion restrictions, exhaust, discard and replay.
- Later-act enemy HP offsets the stronger boons; the existing 5% boss damage increase remains. In 900 paired seeded campaigns the heuristic won Act 3 in 25.9% of candidate runs versus 20.4% on r4. This is an automated comparison, not a human win-rate guarantee.
- Five-win secret-boss access, unlocks through ten wins and the once-only three-character finale are preserved. Old stable saves import into the extended run state.

Validation: native address/undefined-behavior sanitizer regressions, full-catalog pickup/save/combat exercises, 180,000 same-rarity reward draws, 1,800 boon offers, paired campaigns and PPSSPP checks of the exact complete EBOOT. Hardware PSP testing was not available.

Install: replace EBOOT.PBP in PSP/GAME/RIFTASCENT. Preserve existing account and save files. In-game version remains 1.8.6. The separate distribution branch is unchanged.
