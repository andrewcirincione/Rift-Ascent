# Rift Ascent v1.0.3 — downloadable single EBOOT

- **Build SHA-256:** `cf3c067d0a0937cdfaa998a0d29496ba216a9f13ab659f8ad8a21e69d6078793`
- **EBOOT size:** 14,410,846 bytes
- **Full in-file soundtrack:** 10 tracks, 11,533,436 bytes, IMA ADPCM mono at 11,025 Hz
- **Gameplay base:** compiled by successful GitHub Actions run [40019202643](https://github.com/andrewcirincione/Rift-Ascent/actions/runs/40019202643) and checked with PSPDEV, ASAN/UBSAN native gameplay scenarios, three-act assisted progression, PPSSPP screenshots, and runtime confirmation `Rift 1.0.3 embedded music loaded: 10` for a synthetic 10-track test package.
- **Illustrated PSP XMB preview:** custom 480x272 Rift Ascent key art replaces the old menu screenshot.
- **No other files required:** the entire score and original art are in `EBOOT.PBP`.

The full **music-containing** `EBOOT.PBP` was packaged locally from the user's provided `Soundtrack 1.0.3 Compressed.zip` and delivered as a downloadable file in the ChatGPT conversation. It is **not** checked into this public repository because the user-supplied audio files are not present in GitHub Actions; the compiled base EBOOT in CI artifacts does not contain the actual ten user tracks. Do not confuse the base build with the fully packaged release.

For a reproducible build, run `tools/set_pbp_pic1.py` with the supplied splash art on the CI EBOOT, then `tools/embed_soundtrack_103.py` with the original 10 WAV files. Both tools are committed here.

Hardware PSP-3000 / ARK-5 boot and sound playback of the packaged file remain unverified; keep your previous working EBOOT and SAVE.DAT backed up.
