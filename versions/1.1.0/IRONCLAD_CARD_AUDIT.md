# Rift Ascent — 1.1.0 Ironclad card-by-card audit

**Baseline:** release-1.1.0 compiled source from successful GitHub Actions run #40029123085. **Reference:** current 87-card Ironclad roster at https://sts2-wiki.org/slay-the-spire-2-cards?character=ironclad, plus card-specific details from https://sts.wiki/cards/ and individual entries (which may describe different patch versions).

**Scope:** 50 playable cards, IDs 0–49, plus four Rift-specific status cards (Burn, Daze, Injury, Dread). This audits actual `cards[]`, `upgrade_descriptions[]`, `game_play_card()`, `game_card_cost()`, and the embedded combat renderer — not just names. `rarity` in Rift has only three buckets (starter/common/rare), so it cannot faithfully distinguish STS2 Uncommon from Rare yet. Base names overlap **30 of 87** official roster entries; 57 official names are not represented. Several overlapping names were inaccurate prior to audit fixes.

**Verdict meanings:** *Core match* means the main mechanic corresponds, not exhaustive proof of rarity, UI targeting, every relic interaction or game version. *Corrected* means the accompanying patch addresses a verified discrepancy (subject to successful CI). *Partial* means concrete mismatches remain. *Rift-only* means there is no direct same-name correspondence; do not mistake it for a wiki-accurate clone.

| ID | Card | Cost | Verdict | Card-by-card result |
|---:|---|---:|---|---|
| 0 | Strike | 1 | Core match | 6 damage; 9 upgraded. |
| 1 | Defend | 1 | Core match | 5 Block; 8 upgraded. |
| 2 | Bash | 2 | Core match | 8 damage +2 Vulnerable; upgrade 11/+3. |
| 3 | Cleave | 1 | Rift-only | This named card is not in the surveyed current 87-card STS2 Ironclad roster; replace with a documented current card if strict parity is desired. |
| 4 | Battle Trance | 0 | Corrected | Wiki: draw 3/4 then prevent further card draws that turn. v1.1.0 previously omitted the draw lock; patch enforces it. |
| 5 | Shrug It Off | 1 | Core match | 8 Block + draw 1; +11 Block (upgrade implementation should be checked against version-specific wiki). |
| 6 | Bloodletting | 0 | Corrected type | Wiki: Skill, lose 3 HP / +2 Energy. Previously incorrectly tagged Power; converted to Skill. Upgrade remains subject to separate wiki version check. |
| 7 | Bludgeon | 3 | Core match | 32 damage; +38. |
| 8 | Inflame | 1 | Partial metadata | 2 Strength/3 upgraded, matching mechanical core; internal rarity is Rare while STS2 Uncommon. |
| 9 | Impervious | 2 | Core match | 30/36 Block, Exhaust. |
| 10 | Whirlwind | X | Partial metadata | 5/8 AoE X times. In this build X spends remaining energy; classified Rare internally versus STS2 Uncommon. |
| 11 | Pommel Strike | 1 | Corrected | Wiki: 9 damage, draw 1; upgrade is 10 damage, draw 2. Previously upgraded to 12 damage and still drew only 1. |
| 12 | Iron Wave | 1 | Core match | Gain 5 Block, deal 5; both upgraded to 8 in Rift Ascent. |
| 13 | Siphon | 1 | Rift-only | Heal and attack is a local design, not an STS2 Ironclad card. |
| 14 | Rust Cut | 1 | Rift-only | Weak-applying Strike variant; not in the surveyed STS2 roster. |
| 15 | Overpower | 3 | Rift-only | 22-damage attack with self-Weak; not in STS2 roster. |
| 16 | Preparation | 0 | Rift-only | Energy plus draw; local card (STS2 card pool has different named enablers). |
| 17 | Stand Fast | 1 | Rift-only / redundant | Block plus draw overlaps Shrug It Off without a new mechanic. |
| 18 | Brand | 0 | Partial | Now correctly classified as Skill. Self-damage, Strength, Exhaust exist, but automatically exhausts the last hand card instead of letting the player choose. |
| 19 | Thunderclap | 1 | Core match | 4 AoE +1 Vulnerable each; Rift upgrade 7 AoE +2 Vulnerable. Must cross-check against desired STS2 patch version. |
| 20 | Blood Pact | 1 | Rift-only | Simple heal-and-exhaust; not a current STS2 card. |
| 21 | Body Slam | 1→0 | Corrected | Upgrade lowers cost to zero with damage still equal to your current Block; previously retained cost 1 and added flat +5 damage. |
| 22 | Blade Storm | 2 | Rift-only | 3 damage 4/5 hits. Could instead use Sword Boomerang or other official multi-hit card. |
| 23 | Armaments | 1 | Partial | Now retains 5 Block when upgraded, with + upgrading all cards in hand. Base still upgrades the first eligible card automatically instead of permitting user choice. |
| 24 | Last Stand | 2 | Rift-only / redundant | Big exhaustible Block card competes directly with official Impervious. |
| 25 | Vital Strike | 1 | Rift-only | Attack plus guaranteed small self-heal; not a surveyed official Ironclad card. |
| 26 | War Cry | 0 | Rift-only | Draw and discard (automatic discard from end of hand); not in surveyed current Ironclad pool. |
| 27 | Twin Strike | 1 | Core match | 5 damage twice; 8 twice upgraded. |
| 28 | Inferno | 1 | Core match | Wiki-based 1 HP lost per copy each turn; on own-turn HP-loss trigger deals 6/9 AoE; multiple copies stack. |
| 29 | Hemokinesis | 1 | Core match (patch-dependent) | 2 HP loss, 15/20 damage in the card-specific wiki update; older generic index may show 14, so pin live vs beta. |
| 30 | Twin Guard | 1 | Rift-only | Block granted twice, distinct only in hit count; not in surveyed official roster. |
| 31 | Taunt | 1 | Core match | Gain 7 Block; apply 1 Vulnerable; upgraded 10 Block and 2 Vulnerable. |
| 32 | Shatter | 2 | Rift-only | Set target Block to zero then attack; not a surveyed current STS2 Ironclad card. |
| 33 | Quick Guard | 1 | Rift-only / redundant | 5 Block and draw 1, weaker copy of Shrug It Off. |
| 34 | Dominate | 1 | Corrected | Now Skill; applies 1/2 Vulnerable before gaining Strength equal to target Vulnerable, then Exhausts. Previously had no Vulnerable application, incorrect upgrade multiplier, wrong type and no Exhaust. |
| 35 | Piercing Strike | 2 | Rift-only | Attack ignoring Block; not in surveyed current STS2 Ironclad list. |
| 36 | Conflagration | 1 | Mismatch / version-sensitive | The game's one-hit AoE scaling uses all cards played, not just other Attacks. STS2 latest published generic roster says +2 per other Attack, while a card-specific wiki flags an older multi-hit ruleset. Needs explicit patch/version selection. |
| 37 | Coagulate | 2 | Rift-only | Heal plus Block; not in surveyed official Ironclad roster. |
| 38 | Offering | 0 | Corrected type | Lose 6 HP, +2 Energy, draw 3; Exhaust. Previously shown as Power instead of Skill; corrected. |
| 39 | Uppercut | 2 | Core match | 13 damage and 1 Weak/1 Vulnerable; 16 damage and 2/2 upgraded. |
| 40 | Grit | 2 | Rift-only | Block plus Strength; differs from official True Grit, which Exhausts a hand card. |
| 41 | Brawling Wave | 1 | Rift-only / redundant | Duplicates Iron Wave's 5 Block +5 damage and upgrade; remove/replace in broader overhaul. |
| 42 | Fiend Fire | 2 | Core match | Exhaust remainder of hand, 7 damage per exhausted card (10+). Self card also Exhausts. |
| 43 | Burning Pact | 1 | Partial | Correct Skill type and 2/3 card draw; still auto-Exhausts the last hand card instead of allowing a selection. |
| 44 | Anger | 0 | Core match | Deal 6/9 damage and add copy to discard. |
| 45 | Adrenal Force | 0 | Rift-only | Gain 2/3 Energy and Exhaust; not current official Ironclad card. |
| 46 | Blood Wall | 2 | Core match | Lose 2 HP, gain 16/22 Block; now classified Skill. |
| 47 | Feed | 1 | Core match | Deal 10/12; Fatal increases max HP 3/4, self Exhausts. |
| 48 | Demon Form | 3 | Core match | At turn start +2/3 Strength as persistent Power; internal implementation reuses a field named heal_after_battle. |
| 49 | Fracture | 1 | Rift-only / redundant | Damage/Vulnerable/draw composite; more of an upgraded variant than its own gameplay archetype. |

## STS2 Ironclad cards not implemented by name (57)

Aggression, Ashen Strike, Barricade, Break, Breakthrough, Bully, Cascade, Cinder, Colossus, Corruption, Crimson Mantle, Cruelty, Dark Embrace, Demonic Shield, Dismantle, Drum of Battle, Evil Eye, Expect a Fight, Feel No Pain, Fight Me!, Flame Barrier, Forgotten Ritual, Grapple, Havoc, Headbutt, Hellraiser, Howl from Beyond, Infernal Blade, Juggernaut, Juggling, Mangle, Molten Fist, One-Two Punch, Pact's End, Perfected Strike, Pillage, Primal Force, Pyre, Rage, Rampage, Rupture, Second Wind, Setup Strike, Spite, Stampede, Stoke, Stomp, Stone Armor, Sword Boomerang, Tank, Tear Asunder, Thrash, Tremble, True Grit, Unmovable, Unrelenting Attack, Vicious.

## High-priority discrepancies that remain

1. **User-directed card selection:** Brand, Armaments (base), and Burning Pact must open hand selection, not silently consume the last/first available card. This requires a new two-stage targeting UI and persistence through Save/Quit; the audit patch does **not** claim it is fixed.
2. **Conflagration:** current source counts all cards via `turn_played`, whereas the roster description counts *other Attacks only*. Also the card-specific wiki has multiple historic versions. Choose a pinned source patch and then build an attack-count tracker and tests.
3. **Original duplicates:** Stand Fast / Quick Guard / Shrug It Off, Iron Wave / Brawling Wave, and Last Stand / Impervious undermine deck variety. Replace or clearly mark them original; do not automatically assume historical STS1 cards belong in STS2.
4. **Upgrade/rarity sweep:** compare every upgrade branch and reward rarity individually against one pinned STS2 game version, especially Bloodletting, Thunderclap, Conflagration, and the Power vs Skill distinctions. Existing `type`/rarity fields and `UPBIT` only support a simplified model.
5. **Entire absent card pool:** 57 wiki cards aren't implemented at all; one-for-one STS2 recreation requires adding cards and event rules, and a save migration / version bump because existing save records depend on fixed numeric card IDs.
6. **Presentation:** even after an engine rule is fixed, the inspector, reward view, upgrade preview, and card cost visuals must be checked against that same rule. The new upgrade-cost function is used in card visuals, but peripheral UI paths may still require validation.

## Changes in the audit patch

- Correct Battle Trance's no-extra-draw rule and display its Skill type.
- Correct upgraded Pommel Strike from +12 / draw1 to **10 damage / draw2**.
- Correct Body Slam+ to **zero energy and damage equal to Block**, including its in-card cost display.
- Correct Armaments+ to retain **5 Block**, while upgrading the whole hand. Base card remains a known targeting discrepancy.
- Correct Dominate to apply Vulnerable first, gain Strength from the resulting stacks and Exhaust; fix its Skill metadata and upgraded description.
- Correct Skill vs Power types for Bloodletting, Brand, Offering, and Burning Pact.
- Add deterministic tests for these cost, status, drawing and upgraded behavior rules; preserve GAME_VERSION 16 and prior save structure.

**No claim of full equivalence:** This report makes the remaining gaps explicit so they can be implemented and tested one by one.
