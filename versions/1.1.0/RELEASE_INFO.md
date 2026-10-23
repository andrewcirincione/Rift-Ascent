# Rift Ascent 1.1.0 — verified packaged file

- **Full EBOOT SHA-256:** `c21884f6b1c17c9aa08abff6b9ece89e02f2dce027fa199de58fef5fc22a680e`
- **Full EBOOT size:** **14,435,122 bytes**
- **Ten soundtracks:** **11,533,436 bytes** of built-in 4-bit mono IMA ADPCM
- **Gameplay and graphics source:** [1.1.0 PSPDEV/PPSSPP CI success](https://github.com/andrewcirincione/Rift-Ascent/actions/runs/40029123085) (commit `4b2d7592`, includes Inferno-selected visual regression screenshot and new antialiased path icons)
- **PBP structure:** 480×272 PNG XMB preview, exact PSP executable bytes verified identical to the CI-compiled EBOOT, monotonic PBP section offsets
- **Soundtrack:** ten track descriptors, frame bounds, indexed footer, and CRC32 verified after embedding the user-provided WAVs; sampled audio gain measured higher than 1.0.3
- **Native tests:** ASAN/UBSAN engine tests, 1.0.1/1.0.2/1.0.3 regressions and 1.1.0 Inferno edge cases passed
- **Not hardware tested:** Boot and music playback have not yet been verified on a physical PSP-3000 with ARK-5

The **complete 1.1.0 EBOOT with the user's soundtrack** has been created and is downloadable from the ChatGPT conversation. GitHub's `BASE_EBOOT.PBP` (if present) is the *compiled source-only binary*, **without the user's ten songs**; it is archived for reproducibility and is **not the same EBOOT** as the full 14.4 MB one. Source audio was never placed in the public GitHub repository.

To rebuild the full binary, use the validated Github CI artifact, `tools/set_pbp_pic1.py` with the 480×272 XMB art, and `tools/embed_soundtrack_110.py` with the ten user-provided WAVs. Do not just rename or append raw WAV bytes. Full soundtracks are embedded as read-on-demand ADPCM and do not require external files during play.

Install as `PSP/GAME/RIFTASCENT/EBOOT.PBP`, after backing up the previous version and `SAVE.DAT`.
