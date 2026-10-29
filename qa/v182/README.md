# Rift Ascent 1.8.2

This revision starts from the published 1.8.1 tree and complete EBOOT. The
1.8.1 lineage is the 1.8-r2 rebuild from 1.71-r2, not the abandoned 1.8 prerelease.
`BASE_PROVENANCE.json` records the immutable starting point.

## Changes

- Menu unlock totals live only in the Unlocks browser. Save-slot text is inset
  from its selection frame. Unselected character panels are shaded uniformly.
- Screen changes use a full-frame fade instead of the lingering partial-screen
  black veil. Map-to-normal/elite transitions take 16 display frames, with music
  paused for the first 12 (about 0.20 seconds at 60 Hz). Boss transitions keep
  their existing music behavior. Victory presentation timing is preserved.
- RiftFight4 joins normal combat music; RiftFight5 joins elite music.
- The XMB RiftBoss preview is re-encoded from the original supplied WAV with a
  180 ms silent lead, gentle entrance and silent tail. All ten full in-game
  soundtrack streams are preserved byte-for-byte.
- 27 enemy illustrations replace repeated or low-contrast designs, including
  Ash Crawler, Crypt Archer, Glimmer Moth, Chain Keeper and Sable Lurker.
  Acts 1–3 each have ten regular species and four distinct elite species;
  Crown Eater supplies Act 3's fourth unique elite. Later acts mix these pools.
- Every player pose and enemy silhouette receives a small, feathered shadow
  derived from its existing alpha. This uses renderer code, no new shadow art.
- The multi-enemy target marker is smaller and quieter. Mystery card previews
  have portrait proportions; other preview dimensions are unchanged. The
  Nameless Exchange now shares the raised/highlighted card-choice feedback.
- Selected elites/bosses can fracture defenses, sear, rally allies, purge
  debuffs, inflict gloom or blight. Intents and the combat log state the effects.
- Twenty-two new mystery rooms (39 total) add one-offer and three-offer layouts, potion trades,
  permanent Strength, multiple upgrades, modifications and a coin gamble.
  Costs are checked on commitment; canceling a card picker spends nothing.
  The extra 15 include four modification rooms, potion distillation, card
  transformation, and next-combat bargains visible in the status browser.
  A 64-bit event bag preserves coverage above 32 rooms.
- Character stories roll once, at 1%, after a completed non-boss map room and
  its rewards. Inventory visits do not create new rolls. The pending room and
  rare-card offset persist in saves; 1.8 and 1.8.1 runs migrate without changing
  account unlocks or the four-slot format.

## Rarity audit

The STS2 comparison uses the published game-data analysis at
https://spire-codex.com/mechanics/card-rarity, cross-checked against
https://sts.wiki/guides/card-rewards and the STS2 Cards reference. These are
community game-code analyses, not an official Mega Crit specification.

| Source | Common | Uncommon | Rare |
|---|---:|---:|---:|
| Normal (base) | 60% | 37% | 3% |
| Elite (base) | 50% | 40% | 10% |
| Boss | 0% | 0% | 100% |
| Shop (base) | 54% | 37% | 9% |

Combat rolls share a persisted rare offset: start/reset −5, increase one per
non-rare, cap +40, evaluate slots left-to-right. Shops read but do not advance
it. Negative rare thresholds also reduce the uncommon band. Non-rare automatic
upgrade chances are 0/25/50% by act. Ancient cards remain available through
explicit special events and character stories. Existing character/unlock
eligibility, duplicate suppression and relic-specific upgrades remain intact;
Rift Ascent does not have STS2's ascension difficulty modifiers.

## Reproduce

```sh
export PSPDEV=/path/to/pspdev
bash qa/v182/build.sh
bash qa/v182/test.sh
python3 qa/v182/check_media.py
RIFT_FAST_PRESENTATION=1 python3 qa/v182/qa_ppsspp.py
RIFT_COLD=1 python3 qa/v182/qa_ppsspp.py
```

`art-specs.json`, `art/prompts`, and `art/generation-*.json` retain asset prompts
and generation provenance. `pack_art.py` appends new art to the exact previous
bank and emits checked texture descriptors. `--draft` exists solely for early
compile checks; release packaging and media checks reject missing assets.

The emulator tests use the complete packaged executable, real button input,
and explicitly named state fixtures for targeted late-game scenarios. Native
sanitizer tests cover game logic, save migration and all rendered content.
