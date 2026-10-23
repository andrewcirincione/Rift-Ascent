# Rift Ascent 1.0.1 — feedback-driven development candidate

This file tracks the full user-reported scope. A binary named `EBOOT.PBP` is written to this folder only after **the 1.0.1 PSPDEV/PPSSPP/native-test workflow passes**. Do not mark it hardware-proven until a voluntary PSP-3000/6.60/ARK-5 validation.

## Addressed in 1.0.1 candidate (must pass CI)
- [x] Card rewards exclude the three starter basics (Cut, Brace, Sunder); starters remain in the starting deck.
- [x] Upgrade selection now refuses already-upgraded cards, prevents spending the selection, and shows the upgraded description.
- [x] The forge event gives two explicit card choices rather than upgrading randomly, with visible confirmation.
- [x] Elites award one previously unowned relic on defeat; a map-accessible relic inventory and combat relic inspector tab are added.
- [x] Shared enemy attack calculations show total damage after Weak/Vulnerable modifiers in combat and the inspector. Nonattack buffs/heals show no damage.
- [x] Card descriptions get more vertical space, with taller combat cards overlapping the battlefield and a partial next card to indicate hidden hand cards.
- [x] Large combat damage values, an enemy attack animation instead of a timer meter, and a delayed victory/defeat-of-enemy visual before reward.
- [x] Selected and upgraded cards displayed in a card-image grid with a current/after-upgrade sidebar.
- [x] Reward skip supports D-pad Down; map room letter icons are replaced with symbols and boss room is shown above the path.
- [x] Combat inspector includes the currently selected foe's effects and a fifth relic tab, with map relic list via START.
- [x] A stage change now gets an act-title transition and the control help screen has custom face-button glyphs.
- [x] Automated regressions for shop entry persistence, elite rewards, upgrades, card rewards, preview changes and relic browsing.

## Further work requested — not yet complete
- [ ] Rebuild the entire card pool around well-balanced, varied health-loss / Strength / Exhaust / retaliation archetypes inspired by Ironclad in Slay the Spire 2; rethink repetitive variants and upgrade costs.
- [ ] Full enemy rebalance (particularly low-HP enemy types), with numeric playtest benchmarks.
- [ ] Complete expressive hit reactions and punchier layered impact audio.
- [ ] Rebuild map into a more organic connected-route silhouette with nonuniform lane positions and a visually distinct boss approach.
- [ ] Original, contextual illustrations and short narrative prose for all mystery events.
- [ ] Bespoke PSP home-menu background illustration: lone Warden in foreground facing a vast rift.
- [ ] Root-cause investigation of the reported shop auto-skip on handheld: an engine-level shop entry regression is included, but input timing on real hardware must be checked.
- [ ] Deep dynamic enemy intent glossary and action-by-action HP/Block preview for multi-enemy fights, poison ticks, action-triggered effects and buffs.
- [ ] Audit every relic/reward description for fitting within the available card and menu dimensions.

## References
Design/balance reference only: https://slaythespire.wiki.gg/wiki/Slay_the_Spire_2:Ironclad and https://slaythespire.wiki.gg/wiki/Slay_the_Spire_2:Map_Locations. Original Rift Ascent names, event stories, UI and artwork; no copied game assets.

## Testing and save compatibility
- Preserve `GAME_VERSION=16` and `Game` struct binary layout to support earlier saves.
- PSP firmware target 6.60; no kernel or network imports, original CRT-free NEXTVSYNC frame lifecycle untouched.
- Build EBOOT and screenshots with `.github/workflows/overhaul-psp.yml`, run `overhaul/v101/test_101.c` with ASAN/UBSAN, confirm actual PPSSPP render output.
- Hardware playability and FPS of this build are unverified; never replace the last-known-working handheld binary without backing it up.
