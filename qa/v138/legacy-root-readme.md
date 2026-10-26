> **HARDWARE SAFETY / RELEASE STATUS — 2026-10-22**
>
> **NOT VERIFIED ON REAL PSP; DO NOT INSTALL THE EXPERIMENTAL EBOOT BUILDS.** A PSP-3000 running system software 6.60 and ARK-5 has repeatedly frozen on launch (black screen; HOME unresponsive; hard reset required), including with the tiny test. The 30-second auto-exit in that test only runs if its event loop executes, so it is *not* a hardware watchdog. Cave Story and other homebrew work normally on the same PSP. Compiling with PSPDEV and displaying a title screen in PPSSPP did not establish real-hardware compatibility. Repackaging as single-segment legacy ELF and compiling a firmware-6.60 PRX did not resolve the problem. Do not request further on-device testing without an evidence-based revision and explicit risk acknowledgement.

# Rift Ascent — PSP build repository

## Download compiled PSP EBOOTs

**[Browse Rift Ascent GitHub Releases](https://github.com/andrewcirincione/Rift-Ascent/releases)**

Every **successful** build from the maintained `v1.1.3` branch is archived as a separately versioned, permanent GitHub Release. Releases provide a download of the exact compiled `.PBP` file and a matching SHA-256 checksum. The **Actions** tab additionally retains emulator test screenshots and build logs for a limited period.

**Music and copyright:** This is a **public** repository. Publicly downloadable CI builds are labeled **NO MUSIC** and intentionally exclude the ten privately supplied soundtrack recordings. The game does not synthesize background music. The original-soundtrack EBOOT is stored separately in the owner's ChatGPT Library, not on the public GitHub repository.

**Install note:** A downloaded filename like `Rift-Ascent-v1.1.3-build-123-NO-MUSIC-EBOOT.PBP` must be renamed to `EBOOT.PBP` for the PSP homebrew folder at `PSP/GAME/RIFTASCENT/`. Hardware compatibility has not been confirmed on PSP-3000/ARK-5, so keep a known-working backup.

## Source and testing

- Project sources: `Rift_Ascent_GitHub_OneClick_Setup.zip` plus sequential updates in `overhaul/`
- Maintained release branch: [`v1.1.3`](https://github.com/andrewcirincione/Rift-Ascent/tree/v1.1.3)
- Automated workflow: [PSP gameplay overhaul](https://github.com/andrewcirincione/Rift-Ascent/actions/workflows/overhaul-psp.yml)
- Earlier EBOOT snapshots also appear where available under `versions/`.

