# Rift Ascent 1.0.2 — Embedded CHURN revision

The updated v1.0.2 EBOOT includes the complete user-supplied `CHURN` battle music **inside the EBOOT.PBP itself**; a separate `CHURN.PCM` is no longer necessary.

## Compression and format
- Input: 186.46-second, 44.1 kHz stereo float `CHURN.wav`.
- Embedded format: continuous IMA ADPCM, 11,025 Hz mono, 4 bits/sample.
- Size: approximately **1,027,870 bytes** (~1.0 MB) of compressed music; substantially smaller than the prior ~16 MB PCM file.
- Playback: the game opens its own `ms0:/PSP/GAME/RIFTASCENT/EBOOT.PBP` and reads the ADPCM trailer using user-mode `sceIoLseek32` and `sceIoRead`. It decompresses incrementally into the normal 512-frame PSP stereo audio buffer; no need to store the entire soundtrack in PSP RAM.
- The original in-game synthesizer remains available for noncombat scenes and as a fallback if the embedded music footer cannot be read.
- The footer stores `RIFTIMA1`, byte length, number of samples, and a CRC32 for offline integrity checks.

## Install
**Only copy `EBOOT.PBP`** to `PSP/GAME/RIFTASCENT/`. You may remove `CHURN.PCM` after confirming that this version works; it is not used by the embedded-music build. Back up the known-working version first.

## Reproducible packing

Compile the EBOOT through the modified `.github/workflows/overhaul-psp.yml` workflow (which intentionally produces an **unpacked** base EBOOT, as the original user-supplied WAV is not stored in this public repository). Then run:

```bash
python3 tools/embed_churn.py --wave CHURN.wav --eboot path/to/base/EBOOT.PBP --out path/to/release/EBOOT.PBP
python3 tools/embed_churn.py --check path/to/release/EBOOT.PBP
```

Requires FFmpeg for conversion. The resulting single EBOOT includes both the program and CHURN music.

## Testing limitations
Native PSPDEV compiler, user-mode import audit, standalone PBP footer test, and PPSSPP loading checks do not prove physical PSP-3000/6.60/ARK-5 compatibility. Because earlier builds hard-froze on the handheld, keep a verified EBOOT backup and try the new build only voluntarily. The music decoding and file-seeking implementation is new.

The archived original `versions/1.0.2/EBOOT.PBP` on the base release branch still expects the separate CHURN.PCM. Do not mistake that original archival build for the updated embedded-music binary until an updated binary is explicitly published there.

## Verified build and packaged revision (October 23, 2026)
- CI: https://github.com/andrewcirincione/Rift-Ascent/actions/runs/40006489890 — **success**, including PSPDEV, import audit, PPSSPP launch of a PBP with an ADPCM trailer, and existing gameplay smoke checks.
- Self-contained final EBOOT with the complete CHURN track: **3,891,081 bytes**.
- SHA256: `e60e4642f4b62322b0cc2e00912ba96946e1f3b4320e1b5eec9937410306d3b2`.
- Encoded music: 1,027,870 bytes, 2,055,739 samples (186.46 seconds).
- The final binary has been delivered as a conversation attachment. The older archived `versions/1.0.2/EBOOT.PBP` in GitHub has **not** been replaced; use the attached standalone build rather than confusing it with the older external-music archive.
