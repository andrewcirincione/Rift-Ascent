# Main edition research baseline

Reviewed 2 November 2026. This work belongs only to the STS-inspired main edition. The distribution branch is unchanged.

Sources:
- Mega Crit, Neowsletter 16: https://www.megacrit.com/news/2025-11-13-neowsletter-issue-16/ — Ancients, exclusive act-start boons, transformative effects.
- https://sts.wiki/relics and https://sts.wiki/relics/ice-cream — full Energy retention; cross-check on pools.
- https://slaythespire.wiki.gg/wiki/Orobas and https://slaythespire.wiki.gg/wiki/Touch_of_Orobas — starter upgrades and boon pools.
- https://github.com/ptrlrd/spire-codex/blob/main/data/eng/relics.json — extracted English factual catalog snapshot, 296 entries excluding internal Vakuu card selector. The versioned relic-coverage.json enumerates the implemented baseline and the shipped independent descriptions.

The wiki additionally lists two public-beta-only entries, Dowsing Rod and Neow's Sacrifice. They are outside this stable-game baseline; this is not a claim to include future or beta-only relics. References: https://sts.wiki/cards/dowsing and https://sts.wiki/relics/neows-sacrifice.

Names and factual mechanics were cross-checked; implementation and short UI descriptions were written for Rift. Orb, summon, multiplayer, enchantment and missing-card effects use explicitly labelled adaptations to Rift's existing cards, Poison, Vigor, Block and solo combat. Cosmetic souvenirs intentionally have no combat proc. Shop, event, ordinary and Ancient pools remain distinct. Duplicate copies are supported by custom runs, with independently saved counters and per-copy numeric scaling; boolean rules such as Energy retention do not compound into duplicate turns.

Act 1 uses opening boons, including meaningful deck changes and risk/reward trades. Later acts offer a rule-changing option, a build option and an Ancient wildcard. Stronger starter relics replace the starter. Ice Cream retains all unused Energy up to the engine's global 99-Energy cap; combat resets clear it.
