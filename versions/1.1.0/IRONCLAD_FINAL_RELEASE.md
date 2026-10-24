# Rift Ascent 1.1.0 — Ironclad 50-card final candidate

- **Delivered full EBOOT:** `Rift-Ascent-v1.1.0-Ironclad-Final/EBOOT.PBP`, available from the ChatGPT conversation.
- **Full EBOOT SHA-256:** `9ef2efe84469b3edde15dcf61a4a2f6d2b7ee2290576f3a6c864e4adb39ea8e0`
- **EBOOT size:** 14,442,674 bytes
- **All-in-one soundtrack:** user-provided 10-track IMA ADPCM package (11,533,436 bytes), unchanged from 1.1.0; previously verified CRC32 `568353e0`.
- **PSP home-screen art:** same illustrated PIC1 artwork as 1.1.0; PBP offsets regenerated and executable data verified byte-for-byte against CI base.
- **Compiled PSP base:** `de21a3a24f078dfc85b5ca7a44591c6b9b6088c6b29436c38b94002afc17c7a0` (SHA-256); successful PSPDEV/native/PPSSPP run [40113311838](https://github.com/andrewcirincione/Rift-Ascent/actions/runs/40113311838), commit `f5971259`.
- **Tests:** native gameplay regression suite; Brand/Armaments/Burning Pact/Headbutt choices; save/load in the middle of choice; True Grit; Rupture, Rage, Inferno, Flame Barrier, Exhaust, current Conflagration; Pact's End condition; updated Bloodletting+ and Demon Form; full three-act assisted campaign; PPSSPP boot/render and synthetic embedded index.
- **Card roster:** all 50 existing playable card IDs now use documented Ironclad card names and mechanics. Details: [IRONCLAD_FINAL_50.md](IRONCLAD_FINAL_50.md).
- **Important limitation:** **37** documented Ironclad cards are still absent. The engine has not been expanded to all 87; no false 1:1 enemy/relic/gameplay parity claim. Some corner-case interactions require physical run verification.
- **Hardware:** the complete audio-containing EBOOT has not been physically tested on a PSP-3000 running ARK-5; keep a known-good EBOOT and backup `SAVE.DAT`.
- **Media privacy:** the 10 original user WAVs have not been committed to GitHub. The GitHub repository retains source, deterministic packing tools, test artifacts and engine-only base; the complete music-bearing EBOOT is distributed through ChatGPT.

Installation: replace only `PSP/GAME/RIFTASCENT/EBOOT.PBP`. No separate audio file required.
