# Rift Ascent 1.3.11

The complete tested PSP build, including all five music tracks and the RiftBoss
home-screen preview. This release brings the repository up to date with the
delivered source and artwork through 1.3.11.

- Endless Mode continues beyond Act III with scaling enemies and new act blessings.
- Normal fights choose RiftFight or the final replacement RiftFight3 recording;
  elite encounters use RiftFight2.
- Final blows play in slow motion, pause music at contact, show a short victory
  banner and then open rewards. The normal final boss ends the campaign without rewards.
- Victory text now holds for one-third as long and fades twice as fast.
- Power animation frames keep the player planted. Hand arrows are brighter and
  the Triangle button matches the other controller symbols.
- Restored and corrected RiftBoss splash audio; matching title icon; act entrance art.
- Improved sprite grounding, attack/block audio, Plating presentation, shop UI
  and Violet Orb descriptions.
- Includes the earlier gothic UI, card previews, status inventory, dynamic card
  damage/block descriptions, encounter/map and balance updates.

Install the PSP ZIP at the Memory Stick root, or rename the standalone file to
`EBOOT.PBP` and place it in `PSP/GAME/RIFTASCENT/`. Keep `SAVE.DAT` and
`PROFILE.DAT`. Older v16–v18 saves migrate to v19.

Validated with native sanitizers, arithmetic/preview regressions and PPSSPP 1.20.4.
The final binary was checked for new music playback and the shorter victory
transition. No physical PSP was available for this build's validation.

EBOOT SHA-256: `44bdf11de869d7355c2f14af2d5d99b649dae3734c6aba44a57b6e406417bfc6`

Full notes and build instructions: [qa/v1311/README.md](https://github.com/andrewcirincione/Rift-Ascent/blob/v1.3.11/qa/v1311/README.md).
