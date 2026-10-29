# Rift Ascent 1.8.4 — combat tuning and reward correction

Release tag `v1.8.4-r4`; the in-game version remains 1.8.4.

- Enemy attacks deal 30% more damage than the published r3 build, rounded per hit. The existing act/floor difficulty curve remains, so the increase also applies from Act 1. Enemy health is unchanged.
- Two-enemy encounters reduce each enemy's damage to 75% of its single-enemy value. This reduction remains after one enemy dies. Intent numbers and actual damage use the same calculation, including Weak, Vulnerable and multi-hit attacks.
- Enemies attack slightly more often: the chance of replacing an otherwise defensive move with a strike rises from 4% to 9%. The Act 7 special boss retains its scripted rotation.
- Stone Giant attacks on at least three turns in its four-turn rotation: Braced Strike, Crush, Purge, then Strike. It retains its cleansing move and does not open with Crush.
- Bone Captain gains Bone Bulwark: a light hit and 4 Block for each living enemy. Chain Keeper gains Chain Lash: remove 3 Block, a light hit, and 1 less Energy next turn. Cathedral Horror gains Grave Choir: 5 Block for each living enemy and 2 Poison. Rift Behemoth gains Rift Rend: a light hit, 2 Vulnerable, and a Daze. Each replaces an existing move and has overhead icons and an inspection description.
- Debris is excluded from obtainable card rewards, shops and event reward pools. Existing unclaimed Debris offers are replaced when loading, preserving their upgrade and the other offers. Already-owned Debris is retained. Cards that intentionally generate Debris still work; genuine unlocked Ancient cards remain obtainable.
- Investigated the supplied Proxy save: its 6 Energy returned to the normal 3 on subsequent turns without card plays. Hegemony+ grants 3 Energy next turn; Refine Blade+ grants 1. Save/load, repeated previews, Prism Bone and delayed Energy tests found no persistent unexplained Energy gain, so intended card effects remain unchanged.

The final automated campaign probe cleared Act 3 in 46 of 300 fully unlocked runs (15.3%), with no policy stalls. Losses: 82 in Act 1, 89 in Act 2, 83 in Act 3. Fresh-account runs cleared 16 of 300 (5.3%). These are results from a fixed heuristic player, not an estimate of a human player's win rate; it is particularly weak with Proxy. The proposed enemy-health reduction was not applied.

All existing artwork, twelve complete music tracks, map generation and save layout are preserved. Install by replacing EBOOT.PBP in PSP/GAME/RIFTASCENT while retaining the save/account files.
