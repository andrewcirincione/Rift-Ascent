# Battle music for Rift Ascent 1.0.2

The user-provided source track is `CHURN.wav`, a 3:06.462, 44.1 kHz, stereo, 32-bit float WAV.

The PSP implementation requires a **separate audio file** named `CHURN.PCM` in the same memory-stick folder as `EBOOT.PBP`:

`ms0:/PSP/GAME/RIFTASCENT/CHURN.PCM`

This is **not a standard WAV file**: it contains little-endian signed 16-bit samples at 22,050 samples/second, two channels, with no header. Convert your supplied WAV using:

```bash
ffmpeg -y -i CHURN.wav -ar 22050 -ac 2 -c:a pcm_s16le -f s16le CHURN.PCM
```

The converted `CHURN.PCM` is about 16 MB. Do not rename the original WAV to PCM. The model-provided complete ZIP for 1.0.2 includes the converted asset; due to its size the music is not stored in this repository.

`portable/psp_start.c` is patched by `overhaul/v102/21_music_stream.py`. On the PSP, battle audio is streamed in 16 KiB blocks through user-mode IoFileMgrForUser APIs and mixed with synth SFX in a 512-frame blocking stereo output worker. It resets at the start of each battle and loops at EOF; menus retain the procedural score. Missing or unreadable `CHURN.PCM` falls back to the procedural soundtrack.

**Testing:** The native PSPDEV and PPSSPP build smoke tests cannot verify that this specific memory-stick audio file plays with no glitches on the physical PSP-3000, 6.60 ARK-5. Preserve your older EBOOT and test the 1.0.2 candidate voluntarily before replacing the backup.
