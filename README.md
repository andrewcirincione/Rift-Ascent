# Rift Ascent for PSP

**[Download the latest GitHub release](https://github.com/andrewcirincione/Rift-Ascent/releases/latest)**

Current version: **1.5**. Releases include the complete EBOOT with its intended
soundtrack, an installable PSP ZIP, source, a combat screenshot and SHA-256 checksums.

Extract the PSP ZIP to the Memory Stick root. For the standalone download, rename
it to `EBOOT.PBP` and place it in `PSP/GAME/RIFTASCENT/`. Preserve `SAVE.DAT` and
`PROFILE.DAT`. Version 1.5 migrates older runs to 19-floor acts while preserving cards, items and active combats.

## Source and validation

- [Current patch notes](versions/1.5/PATCH_NOTES.md)
- [Build instructions and validation details](qa/v150/README.md)
- Canonical source: `qa/v150/source`; runtime copies: `GAME/src` and `portable_build`
- Exact tested binary: `versions/1.5/EBOOT.PBP`
- Tested in PPSSPP 1.20.4 with native sanitizer and gameplay regressions.
  Physical PSP playback was not independently tested for this build.

## Publishing

Every future release must include a source push and a GitHub Release. Updating
`release/current.json` on `main` starts the [publishing workflow](.github/workflows/publish-release.yml).
It validates the exact tested EBOOT and source, packages the downloads, verifies
the uploaded checksums, and publishes the release. See [AGENTS.md](AGENTS.md) for
the standing release requirements.

The older NO MUSIC releases remain available in release history.
