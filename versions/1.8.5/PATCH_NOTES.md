# Rift Ascent 1.8.5

- Easier combat, tuned to 30.9% wins over 900 fully unlocked automated campaigns. Lower enemy damage overall, with an additional reduction to normal/elite Crush spikes. Boss and elite recovery patterns from Beta 1 remain.
- Ordinary fights are 50% single, 25% double and 25% triple enemies. Elites and bosses retain their designed single encounters. Pairs use 70% health/damage per foe; trios use 50% health and 45% damage. These reductions remain after other enemies die.
- Three-enemy fights support the existing controls, all three characters, individual targeting, AoE/random attacks, previews, inspection, statuses, death animations and saving. All three health bars and intent descriptions remain visible.
- Removed Forge from combat card rewards. After a random story encounter, choose its gift or use Forge to upgrade a card, modify one, or spend 4 maximum HP to Echo an Attack. Cancel freely before confirming. Forge can appear through these stories from Act 1 onward; existing mystery modification events remain.
- Random stories now have a 5% chance after each completed non-act-boss room, after its rewards. No extra roll after closing the story or using Forge.
- Keeps the beta's run recap and the complete existing artwork and twelve-track soundtrack.
- Migrates existing stable run slots and preserves account unlocks. Updating within a beta-only folder imports its account and surviving runs into stable save files, preserving the original beta files. Stable saves take priority if both sets exist. An interrupted old reward-screen Forge returns to its unclaimed spoils.

Install: replace `EBOOT.PBP` in `PSP/GAME/RIFTASCENT` and retain all save/account files. The PSP ZIP contains the correct folder structure.

Validation: sanitizer-backed regressions, 2,340 three-enemy card cases, 20,000 encounter samples, 100,000 story rolls, 1,200 final campaign runs and controller-driven tests of the complete EBOOT in PPSSPP 1.20.4. Fully unlocked results: Warden 40.3%, Vesper 36.7%, Proxy 15.7%; overall 30.9%. The bot is comparatively weak with Proxy; these are not human win-rate estimates. Physical PSP validation remains unverified.
