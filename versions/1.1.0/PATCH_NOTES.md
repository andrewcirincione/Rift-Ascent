# Rift Ascent 1.1.0 — Visual clarity, Inferno accuracy, louder music

**Branch:** `release-1.1.0` — keeps 1.0.3 and older versions untouched.

## Changes based on physical-PSP feedback (all five requests)

1. **Actual claymore hero sprite.** Extracts the original hero's 164×127 RGB565 sprite; removes the disconnected short weapon; composites a new 210×127 sword-bearing sprite with a clearly shaped double-edged steel blade, crossguard, wrapped grip, pommel, and painted arm/hand. Removed the old renderer's diagonal line primitives. Source: `overhaul/v110/generate_art.py`.
2. **No collision between card energy, damage and descriptions.** Compact hand cards now have explicitly separated regions: name (top), energy and attack value (second strip), descriptive text (below a 3px gap). The selected card retains its wider 120px display, and the full description stays on the card, not in a separate floating tooltip. Source: `overhaul/v110/patch_render.py`.
3. **Readable map pictograms.** Regenerated all seven icons as 30×30 supersampled, antialiased art: crossed swords for fights, skull for elites, '?' for events, coin purse for shops, campfire, treasure chest, and boss crown. Their decorative node ring remains. No longer depends on aliased primitive lines.
4. **Inferno redesigned according to the *Slay the Spire 2* wiki:** 1 energy Power. Start of your turn: lose 1 HP **per copy**. Whenever you lose HP on your turn, deal **6 damage to ALL enemies (9 upgraded)**. HP loss from Hemokinesis, Bloodletting, Offering, Blood Wall, Brand, status Burn and Inferno itself triggers it, once per HP-loss event (not once per HP point). Enemy attacks/poison damage on the enemy's turn do not trigger it. This ability ignores Strength, Weak and Vulnerable modifiers but respects Block. Multiple copies stack, with a correct start-of-turn HP loss for each. Playing Inferno does **not** immediately lose HP. The on-card text now explicitly states the 6/9 values and trigger. Sources: [sts.wiki](https://sts.wiki/cards/inferno), [Untapped](https://sts2.untapped.gg/en/cards/ironclad/inferno).
5. **Louder soundtrack.** Keeps all ten supplied tracks embedded inside the single EBOOT, no original procedural background music. Before 11,025 Hz/4-bit mono ADPCM encoding, audio is preprocessed with **+4 dB** digital gain and a **0.95 peak limiter**; the runtime mix is raised from 80% to 100% soundtrack level, with battle SFX mixed at a lower gain to preserve clarity. Sampled 6-second ADPCM sections measured ~5–8 dB louder than the 1.0.3 in-game mix, with no hard clipping in those samples. The PSP's hardware speaker volume is not independently measured.

## Tests and limitations

- `overhaul/v110/test_inferno.c` covers base/upgrade text, zero immediate HP loss, HP-loss triggers, target-plus-AoE combination, stacking, ally-free AoE, and interaction with Block, Weak and Vulnerable.
- The existing `overhaul/v103/test_campaign.c`, PSPDEV build/import verification, scripted PPSSPP boot, gameplay and map screenshots, and synthetic music trailer checks also run.
- `tools/embed_soundtrack_110.py` packs the original user WAVs at the new boosted level; original media is not publicly redistributed in GitHub.
- The game binary preserves CRT-free PSP user-mode startup and GAME_VERSION 16 data layout. The actual changes have **not** been verified on a physical PSP-3000/ARK-5, and a complete fair 3-act playthrough is not claimed.
- Old saves may contain legacy Inferno state (damage only, one inferred copy); backup `SAVE.DAT` and the last physically working `EBOOT.PBP`.
- Other Ironclad cards and enemy moves are not claimed to match every current STS2 wiki version; this release specifically verifies Inferno.

## Release

The downloadable **all-in-one `EBOOT.PBP` with ten songs** is delivered in the ChatGPT conversation. Its binary/music cannot be reconstructed from GitHub Actions alone without the ten user-supplied WAVs; the repo retains all source changes, reproducible packer, CI evidence, release manifest, and checksums. The workflow's base artifact **does not contain the licensed/user-supplied soundtrack**. Please do not confuse a base PSPDEV EBOOT with the complete packaged EBOOT.

**PSP installation:** Copy only `EBOOT.PBP` to `PSP/GAME/RIFTASCENT/EBOOT.PBP`, preserving an earlier working binary and save backup.
