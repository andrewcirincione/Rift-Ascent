# Rift Ascent for PSP

**[Download the latest GitHub release](https://github.com/andrewcirincione/Rift-Ascent/releases/latest)**

Current version: **1.8.4** (release tag **v1.8.4-r4**). Increases enemy attack
damage by 30% over r3, balances paired enemies, gives Stone Giant a more active
rotation, and adds four distinctive enemy moves. Debris no longer appears as
an obtainable card reward. Retains the distinct map branches, custom-card art,
sorted catalogs and Afterimage fixes from r3.
Continues the fresh 1.8-r2 rebuild from 1.71-r2.
Releases include the complete EBOOT and soundtrack, an installable PSP ZIP,
source, a combat screenshot and SHA-256 checksums.

Extract the PSP ZIP to the Memory Stick root. For the standalone download, rename
it to `EBOOT.PBP` and place it in `PSP/GAME/RIFTASCENT/`. Preserve `SAVE.DAT`,
`PROFILE.DAT`, and all `ASCENT18_*` files. A valid stable single-save run imports into slot 1 on first launch. The older save stays untouched.

## Source and validation

- [Current patch notes](versions/1.8.4-r4/PATCH_NOTES.md)
- [Build instructions and validation details](qa/v184r4/README.md)
- Canonical source: `qa/v184r4/source`; runtime copies: `GAME/src` and `portable_build`
- Exact tested binary: `versions/1.8.4-r4/EBOOT.PBP` (release tag `v1.8.4-r4`)
- Tested in PPSSPP 1.20.4 with native sanitizer and gameplay regressions.
  Uses 32 MiB PSP mode. The user confirmed 1.6-r2 on physical PSP; the current build has emulator and native validation; physical PSP testing remains unverified.

## Publishing

Every future release must include a source push and a GitHub Release. Updating
`release/current.json` on `main` starts the [publishing workflow](.github/workflows/publish-release.yml).
It validates the exact tested EBOOT and source, packages the downloads, verifies
the uploaded checksums, and publishes the release. See [AGENTS.md](AGENTS.md) for
the standing release requirements.

The older NO MUSIC releases remain available in release history.
