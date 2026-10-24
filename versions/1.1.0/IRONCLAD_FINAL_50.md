# Ironclad rules overhaul — audited 50-card playable roster

**Branch:** `fix-ironclad-mechanics-110`. Retains `GAME_VERSION=16`, IDs 0–49 for playable cards and IDs 50–53 for status cards; historical saves still load (older original cards at unchanged IDs now have Ironclad effects).

## Status

- **50/50 playable card slots** have names from the current *Slay the Spire 2* Ironclad roster. No remaining Rift-only or renamed cards in those 50 slots.
- **37 additional Ironclad cards are not yet represented**. This is not an 87-card-complete recreation; expanding the game to those cards would require new IDs, upgrade arrays, and several mechanics beyond the existing 50-slot save schema.
- Numeric effects in the table are implemented values, **not an independent certification** that every corner case, rarity, target rule, upgrade, card interaction, and current wiki patch precisely matches STS2. These are tested for representative mechanics and existing campaign progression, not a 50×full exhaustive engine-equivalence proof.
- Wiki reference version is the post-July 2026 STS2 card database; historical changelogs sometimes show earlier (different) values. See links below.

## Card-by-card implementation inventory

| ID | Ironclad card | Base/upgraded cost | Implemented rules |
|---:|---|---:|---|
| 0 | Strike | 1 | 6/9 damage |
| 1 | Defend | 1 | 5/8 Block |
| 2 | Bash | 2 | 8/11 damage; 2/3 Vulnerable |
| 3 | Stomp | 3→0 | 12/15 AoE; 1 discount per Attack played |
| 4 | Battle Trance | 0 | 3/4 draw then no further draws |
| 5 | Shrug It Off | 1 | 8/11 Block; draw 1 |
| 6 | Bloodletting | 0 | Lose 3 HP; gain 2/3 Energy |
| 7 | Bludgeon | 3 | 32/38 damage |
| 8 | Inflame | 1 | 2/3 Strength |
| 9 | Impervious | 2 | 30/36 Block; Exhaust |
| 10 | Whirlwind | X | 5/8 AoE per Energy |
| 11 | Pommel Strike | 1 | 9/10 damage; draw 1/2 |
| 12 | Iron Wave | 1 | Gain Block + deal damage |
| 13 | Ashen Strike | 1 | 6 +3/4 per exhausted card |
| 14 | Breakthrough | 1 | Lose 1 HP; 9/13 AoE |
| 15 | Cinder | 2 | 18/24 damage, random hand Exhaust |
| 16 | Evil Eye | 1 | 8/11 Block, doubled if exhausted earlier |
| 17 | True Grit | 1 | 7/9 Block; random or chosen Exhaust |
| 18 | Brand | 0 | Lose 1 HP, choose Exhaust, 1/2 Strength |
| 19 | Thunderclap | 1 | 4/7 AoE +1/2 Vulnerable |
| 20 | Feel No Pain | 1 | Persistent 3/4 Block for Exhaust |
| 21 | Body Slam | 1→0 | Damage equals current Block |
| 22 | Sword Boomerang | 1 | Three/four random hits of 3 |
| 23 | Armaments | 1 | 5 Block, upgrade chosen/all cards |
| 24 | Flame Barrier | 2 | 12/16 Block; return 4/6 each attack |
| 25 | Headbutt | 1 | 9/12 damage, pick Discard to Draw top |
| 26 | Pillage | 1 | 6/9 damage, draw until nonattack |
| 27 | Twin Strike | 1 | 5/8 twice |
| 28 | Inferno | 1 | Self HP-loss: 6/9 AoE |
| 29 | Hemokinesis | 1 | Lose 2 HP; 15/20 damage |
| 30 | Tremble | 1 | 3/4 Vulnerable; Exhaust |
| 31 | Taunt | 1 | 7/10 Block +1/2 Vulnerable |
| 32 | Dismantle | 1 | 8/10; twice if Vulnerable |
| 33 | Rage | 0 | 3/5 Block per Attack this turn |
| 34 | Dominate | 1 | Vulnerable 1/2; Strength per stack; Exhaust |
| 35 | Bully | 0 | 4 +2/3 per Vulnerable |
| 36 | Conflagration | 1 | 2 AoE four/five times |
| 37 | Pact's End | 0 | 18/24 AoE if at least three exhausted |
| 38 | Offering | 0 | Lose 6 HP, gain Energy and draw; Exhaust |
| 39 | Uppercut | 2 | 13/16 damage +Weak/Vulnerable |
| 40 | Molten Fist | 1 | 10/14 damage, double Vulnerable; Exhaust |
| 41 | Rupture | 1 | 1/2 Strength per own HP-loss event |
| 42 | Fiend Fire | 2 | Exhaust hand, 7/10 per card |
| 43 | Burning Pact | 1 | Choose Exhaust, draw 2/3 |
| 44 | Anger | 0 | 6/9 damage, add copy to Discard |
| 45 | Second Wind | 1 | Exhaust nonattacks, 5/7 Block each |
| 46 | Blood Wall | 2 | Lose 2 HP, 16/22 Block |
| 47 | Feed | 1 | 10/12 damage, Fatal +3/4 max HP; Exhaust |
| 48 | Demon Form | 3 | At turn start +3/4 Strength (July rules) |
| 49 | Perfected Strike | 2 | 6 +2/3 per card containing Strike |

## Remaining 37 official cards absent from the existing playable IDs

Aggression, Barricade, Break, Cascade, Colossus, Corruption, Crimson Mantle, Cruelty, Dark Embrace, Demonic Shield, Drum of Battle, Expect a Fight, Fight Me!, Forgotten Ritual, Grapple, Havoc, Hellraiser, Howl from Beyond, Infernal Blade, Juggernaut, Juggling, Mangle, One-Two Punch, Primal Force, Pyre, Rampage, Setup Strike, Spite, Stampede, Stoke, Stone Armor, Tank, Tear Asunder, Thrash, Unmovable, Unrelenting Attack, Vicious.

## Major defects fixed in this follow-up

1. **Mandatory choice screens:** Brand, base Armaments, Burning Pact and upgraded True Grit now select from the hand; Headbutt opens a true Discard-pile selector. The pending selection can be saved/reloaded without changing the Game struct.
2. **Conflagration** is updated to the current 2×4/5 AoE multi-hit design, with Strength applied to each hit, not the old attack-count bonus. Its former text described a 2026 pre-rework build.
3. **Original/redundant cards eliminated:** 16 prior substitutes were replaced, including True Grit, Flame Barrier, Rage, Rupture, Ashen Strike, Breakthrough, Cinder, Evil Eye, Feel No Pain, Sword Boomerang, Tremble, Dismantle, Bully, Second Wind, Perfected Strike, Stomp, Headbutt, Pillage, Pact's End and Molten Fist across the audit phases.
4. **Status and card synergies:** Exhaust events trigger Feel No Pain; Rupture responds to player HP loss; Flame Barrier retaliates on each enemy hit; Rage grants Block per Attack; Stomp discounts according to Attacks played; Pillage draws until a non-Attack, Headbutt places a chosen discarded card on Draw top.
5. **Latest wiki corrections:** Bloodletting+ HP loss remains 3, Demon Form gains 3/4 Strength each turn and stacks, and Pact's End deals 18/24 AoE if at least three cards are exhausted (it can be played for no damage without them).

## Validation and limitations

- Native tests: `test_choice_mechanics.c`, `test_expanded_roster.c`, `test_final_five.c`, `test_latest_wiki.c`, plus earlier 1.0.1–1.1.0 tests and an assisted 3-act campaign.
- PSPDEV toolchain and PPSSPP smoke checks use `.github/workflows/overhaul-psp.yml`; real PSP-3000/ARK-5 boot and sustained frame/audio playback remain unverified.
- The soundtrack remains the same user-supplied ten embedded songs as 1.1.0; the new compiled engine is repackaged into a single EBOOT.
- Some advanced relic/event interactions and the **37 missing cards** remain outside the implemented scope. No claim that enemy attacks and HP equal STS2 data.
- **Save caution:** Preserving IDs and struct layout allows old saves to deserialize but changes the interpretation of cards at those slots. Back up `SAVE.DAT` and known-working EBOOT before installing.

### Sources

- https://sts.wiki/characters/ironclad
- https://sts.wiki/cards/conflagration
- https://sts.wiki/cards/headbutt
- https://sts.wiki/cards/stomp
- https://sts.wiki/cards/pillage
- https://sts.wiki/cards/pacts-end
- https://sts.wiki/cards/molten-fist
- https://sts.wiki/cards/brand
- https://sts.wiki/cards/armaments
