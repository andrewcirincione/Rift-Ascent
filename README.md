# Rift Ascent for PSP

**[Download the latest GitHub release](https://github.com/andrewcirincione/Rift-Ascent/releases/latest)**

Current version: **1.8.4** (release tag **v1.8.4-r3**). Makes alternate map
branches offer distinct room sequences, fixes upgraded-card artwork in custom
loadouts, and sorts character card catalogs by Attack, Skill and Power.
Also fixes mixed-deck Afterimage, slightly increases enemy damage and adds
two distinctive elite moves.
Continues the fresh 1.8-r2 rebuild from 1.71-r2.
Releases include the complete EBOOT and soundtrack, an installable PSP ZIP,
source, a combat screenshot and SHA-256 checksums.

Extract the PSP ZIP to the Memory Stick root. For the standalone download, rename
it to `EBOOT.PBP` and place it in `PSP/GAME/RIFTASCENT/`. Preserve `SAVE.DAT`,
`PROFILE.DAT`, and all `ASCENT18_*` files. A valid stable single-save run imports into slot 1 on first launch. The older save stays untouched.

## Source and validation

- [Current patch notes](versions/1.8.4-r3/PATCH_NOTES.md)
- [Build instructions and validation details](qa/v184r3/README.md)
- Canonical source: `qa/v184r3/source`; runtime copies: `GAME/src` and `portable_build`
- Exact tested binary: `versions/1.8.4-r3/EBOOT.PBP` (release tag `v1.8.4-r3`)
- Tested in PPSSPP 1.20.4 with native sanitizer and gameplay regressions.
  Uses 32 MiB PSP mode. The user confirmed 1.6-r2 on physical PSP; the current build has emulator and native validation; physical PSP testing remains unverified.

## Publishing

Every future release must include a source push and a GitHub Release. Updating
`release/current.json` on `main` starts the [publishing workflow](.github/workflows/publish-release.yml).
It validates the exact tested EBOOT and source, packages the downloads, verifies
the uploaded checksums, and publishes the release. See [AGENTS.md](AGENTS.md) for
the standing release requirements.

The older NO MUSIC releases remain available in release history.
