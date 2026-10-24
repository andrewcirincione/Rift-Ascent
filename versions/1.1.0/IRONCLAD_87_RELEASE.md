# Rift Ascent — 87 Ironclad Cards, PSP Candidate

**Status:** Full 87-name Ironclad v0.107.1 main-release reference catalog implemented on `ironclad-all-87-110`. This is a substantial new gameplay candidate, **not** a physical PSP-certified or mechanically byte-identical copy of STS2.

## Downloadable, all-in-one release

- **Complete EBOOT:** `Rift-Ascent-Ironclad-87/EBOOT.PBP`, distributed through the conversation (user-provided ten-track audio included).
- **Full SHA-256:** `3dabed067e0a3984cc9e2ed7db216e5f86d73ea7d07d9a1d39c2da4d47d1162f`
- **File size:** **14,454,706 bytes**.
- **Music:** ten original user-supplied tracks packaged as 11,533,436 bytes indexed IMA ADPCM, CRC32 `568353e0`. No other game/music file is needed.
- **PBP art:** existing original PSP 480×272 XMB art retained and structurally checked; compiled PSP code section is byte-for-byte identical to the validated base.
- **Compiled engine source/build:** [GitHub Actions run 40116015265](https://github.com/andrewcirincione/Rift-Ascent/actions/runs/40116015265), success. The runnable CI archive is **engine-only**, lacking the private soundtrack until packaged with the music.
- **GitHub version folder:** `versions/1.1.0/` keeps the [full 87-card catalog](IRONCLAD_87_CATALOG.md), previous historical card audits, release checksum, and an independently downloadable **engine-only PSP EBOOT**. Do not confuse engine-only file with the music-containing EBOOT delivered in chat.

## New mechanics and persistence

- Original 50 playable card IDs are retained; 37 new official-card names and implemented effects are appended at IDs **54–90**, after status cards 50–53. Generated Giant Rock is token ID 91, not one of the 87.
- Reward pool, upgrade descriptions, on-card cost and card choice work with all cards. Ancient-only cards can be discovered through an event; four statuses, Giant Rock and two multiplayer-only cards do not appear in normal solo rewards.
- New `GAME_VERSION=17` saves retain the original v16 field prefix and add persistent card-Power state. Old checksum-protected v16 saves are read and upgraded on next save; no existing ID is renumbered.
- Two cards designed exclusively for allies—**Demonic Shield** and **Tank**—are represented in the catalog but explicitly not playable in the solo Rift Ascent campaign; they aren't falsely adapted to nonexistent multiplayer.
- **Mechanics are adaptations.** Specialized timing, targeting, relic relationships and other rare interactions can still differ from STS2 main v0.107.1. Enemy roster/balance is not a 1:1 reconstruction.

## Verification

- CI passes PSPDEV compile/import audit, native regressions for earlier releases, the 87-card effects test, **172 playable base/upgraded card scenarios**, **120 reward checks**, old-save migration, and PPSSPP screenshot/boot tests.
- Assisted three-act campaign passed in **175 steps** (95 battle actions, four shops, three events, eleven rests); it uses assistance/forced combat outcomes and is not evidence of tuned balance.
- Physical PSP-3000 / ARK-5 boot, the full soundtrack and all corner cases remain untested on handheld.
- Back up your old `EBOOT.PBP` and `SAVE.DAT` before installation.

## Installation

Copy the completed `EBOOT.PBP` to `PSP/GAME/RIFTASCENT/EBOOT.PBP`. Do not install GitHub's engine-only build if you expect embedded soundtracks.

Source reference: [87-card STS2 Ironclad v0.107.1](https://spiregenius.com/characters/ironclad/).
