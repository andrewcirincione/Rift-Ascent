# Rift Ascent — Full 87-card Ironclad catalog

**Rules reference:** [STS2 Ironclad main-release dataset v0.107.1](https://spiregenius.com/characters/ironclad/) (87 character cards). This release builds upon the [corrected original fifty](IRONCLAD_FINAL_50.md), adding 37 abilities at IDs 54–90. Four pre-existing status cards keep IDs 50–53, and Giant Rock uses generated-token ID 91.

**Implementation status:** All 87 named Ironclad entries have definitions and card-play implementations, and the original 50-card roster is retained. A generated token is not counted among the 87. Two explicitly multiplayer-only cards are cataloged but cannot be played in Rift Ascent's solo mode and are excluded from ordinary rewards.

## The 50 existing playable card IDs

| ID | Name | ID | Name |
|---:|---|---:|---|
| 0 | Strike | 25 | Headbutt |
| 1 | Defend | 26 | Pillage |
| 2 | Bash | 27 | Twin Strike |
| 3 | Stomp | 28 | Inferno |
| 4 | Battle Trance | 29 | Hemokinesis |
| 5 | Shrug It Off | 30 | Tremble |
| 6 | Bloodletting | 31 | Taunt |
| 7 | Bludgeon | 32 | Dismantle |
| 8 | Inflame | 33 | Rage |
| 9 | Impervious | 34 | Dominate |
| 10 | Whirlwind | 35 | Bully |
| 11 | Pommel Strike | 36 | Conflagration |
| 12 | Iron Wave | 37 | Pact's End |
| 13 | Ashen Strike | 38 | Offering |
| 14 | Breakthrough | 39 | Uppercut |
| 15 | Cinder | 40 | Molten Fist |
| 16 | Evil Eye | 41 | Rupture |
| 17 | True Grit | 42 | Fiend Fire |
| 18 | Brand | 43 | Burning Pact |
| 19 | Thunderclap | 44 | Anger |
| 20 | Feel No Pain | 45 | Second Wind |
| 21 | Body Slam | 46 | Blood Wall |
| 22 | Sword Boomerang | 47 | Feed |
| 23 | Armaments | 48 | Demon Form |
| 24 | Flame Barrier | 49 | Perfected Strike |

See [IRONCLAD_FINAL_50.md](IRONCLAD_FINAL_50.md) for their detailed costs and effects.

## The 37 newly implemented Ironclad cards

| ID | Card | Type | Cost | Base mechanic | Upgrade |
|---:|---|---|---:|---|---|
| 54 | Aggression | Power | 1 | Start turn: take random upgraded Attack from discard | Innate |
| 55 | Barricade | Power | 3 | Retain Block between turns | Cost 2 |
| 56 | Break | Attack | 1 | 20 damage, apply 5 Vulnerable | 30 damage, 7 Vulnerable |
| 57 | Cascade | Skill | X | Play top X draw cards | Play X+1 |
| 58 | Colossus | Skill | 1 | 5 Block; halve incoming damage from Vulnerable foes this turn | 8 Block |
| 59 | Corruption | Power | 3 | Skills cost zero and Exhaust | Cost 2 |
| 60 | Crimson Mantle | Power | 1 | Turn start lose 1 HP, gain 8 Block | Gain 10 Block |
| 61 | Cruelty | Power | 1 | Vulnerable foes take 25% extra damage | 50% extra |
| 62 | Dark Embrace | Power | 2 | Draw one whenever a card Exhausts | Cost 1 |
| 63 | Demonic Shield | Skill | 0 | Multiplayer: give ally Block at self-HP cost | Multiplayer-only |
| 64 | Drum of Battle | Skill | 1 | Draw two; Exhaust triggers Energy gain | 2 → 3 Energy |
| 65 | Expect a Fight | Skill | 2 | Gain Energy per Attack in hand, lock further gains | Cost 1 |
| 66 | Fight Me! | Attack | 2 | Two 5-damage hits, self +3 Strength, foe +1 Strength | 6 twice; self +4 |
| 67 | Forgotten Ritual | Skill | 1 | If Exhausted earlier this turn gain 3 Energy; Exhaust | 4 Energy |
| 68 | Havoc | Skill | 1 | Play draw-top card; Exhaust it | Cost 0 |
| 69 | Hellraiser | Power | 2 | Auto-play Strike cards drawn | Cost 1 |
| 70 | Howl from Beyond | Attack | 3 | 16 to all, replay while Exhausted at turn end | 21 damage |
| 71 | Infernal Blade | Skill | 1 | Generate a temporary free random Attack; Exhaust | Cost 0 |
| 72 | Juggernaut | Power | 2 | Gaining Block deals 6 to a random foe | 8 damage |
| 73 | Juggling | Power | 1 | Copy third Attack played each turn | Innate |
| 74 | Mangle | Attack | 3 | 15 damage; foe loses 10 Strength this turn | 20 damage, 15 Strength |
| 75 | Not Yet | Skill | 2 | Heal 10 HP; Exhaust | Heal 13 |
| 76 | One-Two Punch | Skill | 1 | Replay next Attack this turn | Next two Attacks |
| 77 | Primal Force | Skill | 0 | Transform hand Attacks into Giant Rock tokens | Upgrade tokens |
| 78 | Pyre | Power | 2 | Gain 1 Energy each turn | 2 Energy |
| 79 | Rampage | Attack | 1 | 9 damage, grows 5 each play this combat | Grow 9 |
| 80 | Setup Strike | Attack | 1 | 7 damage, temporary +2 Strength | 9 damage, +3 |
| 81 | Spite | Attack | 0 | 5 damage, twice if HP lost this turn | Up to 3 hits |
| 82 | Stampede | Power | 2 | Auto-play a random hand Attack at turn end | Cost 1 |
| 83 | Stoke | Skill | 1 | Exhaust hand, generate random cards | Generated cards upgraded |
| 84 | Stone Armor | Power | 1 | Gain 4 Plating and Block each end turn | 6 Plating |
| 85 | Tank | Power | 1 | Multiplayer: protect allies, take amplified damage | Multiplayer-only |
| 86 | Tear Asunder | Attack | 2 | 5 damage, extra hit per self-HP-loss event | 7 damage |
| 87 | Thrash | Attack | 1 | 4 twice plus chosen/random Attack damage via Exhaust | 6 twice |
| 88 | Unmovable | Power | 2 | Double first Block gained from a card each turn | Cost 1 |
| 89 | Unrelenting | Attack | 2 | 14 damage; next Attack costs zero | 20 damage |
| 90 | Vicious | Power | 1 | Draw one when Vulnerable applied | Draw two |

## Game-system changes

- **Reward selection:** normal pools now sample the complete playable ID space, excluding the four status effects, Giant Rock token, two multiplayer-only cards, and the two Ancient-only cards. Ancient cards have an event path.
- **New combat state:** Power stacks, temporary Strength, card auto-play, free temporary cards, persistent Block, Plating, repeated Attacks, HP-loss counters, Exhaust triggers, random targets, generated Rocks, and end-of-turn effects are integrated without reusing fragile old fields.
- **Saves:** `GAME_VERSION=17` appends a 48-integer combat state array. The loader can read an existing `GAME_VERSION=16` save, verify its original checksum, zero-initialize new fields, and migrate to v17 on the next save. Existing original card IDs do not change.
- **UI:** Cost and upgrade data extend to 92 IDs, including status effects and Giant Rock. The existing deck/upgrade screens and card selection views use the expanded definitions.
- **Audio and graphics:** Preserve the previously tested art and all ten user-supplied compressed soundtrack tracks in a single EBOOT.

## Validation and exact-parity limitations

The reference is **main v0.107.1**, not the newer beta branch. The expanded game contains adaptations of all 87 cards' main mechanics, but remains a single-player proof-of-concept, not a byte-perfect recreation of STS2: unavailable multiplayer features, target-choice details for some uncommon interactions, enemy-specific behavior and relic synergies may differ. Generated card rarity, some repeat/timing interactions, and advanced UI behavior should be validated in sustained handheld play.

Test coverage includes the entire 87-card catalog, normal/upgraded playtests, save migration, 120 deterministic reward draws, individual Power interactions, a simulated three-act campaign, PSPDEV compilation and PPSSPP smoke checks. The complete EBOOT is **not physically tested on the PSP-3000/ARK-5** until the user tests it.

**Compatibility:** Back up `EBOOT.PBP` and `SAVE.DAT` before updating. Old v16 saves are supported via migration, but saved original deck card IDs now refer to the revised Ironclad mechanics introduced in earlier updates.
