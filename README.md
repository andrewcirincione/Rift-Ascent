# Rift Ascent for PSP

**[Download the latest GitHub release](https://github.com/andrewcirincione/Rift-Ascent/releases/latest)**

Current version: **1.8** (release tag **v1.8-r2**). Rebuilt directly from **1.71-r2**, with no code, artwork or dialogue reused from the previous 1.8 prerelease. Adds four run slots, shared unlock progress, 60 character cards, 20 potions, 10 relics, character endings and a hidden fourth-act boss. Releases include the complete EBOOT with its intended
soundtrack, an installable PSP ZIP, source, a combat screenshot and SHA-256 checksums.

Extract the PSP ZIP to the Memory Stick root. For the standalone download, rename
it to `EBOOT.PBP` and place it in `PSP/GAME/RIFTASCENT/`. Preserve `SAVE.DAT`,
`PROFILE.DAT`, and all `ASCENT18_ACCOUNT.*` / `ASCENT18_RUN_*` files. A valid stable single-save run imports into slot 1 on first launch. The older save stays untouched.

## Source and validation

- [Current patch notes](versions/1.8-r2/PATCH_NOTES.md)
- [Build instructions and validation details](qa/rebuild180/README.md)
- Canonical source: `qa/rebuild180/source`; runtime copies: `GAME/src` and `portable_build`
- Exact tested binary: `versions/1.8-r2/EBOOT.PBP` (release tag `v1.8-r2`)
- Tested in PPSSPP 1.20.4 with native sanitizer and gameplay regressions.
  Uses 32 MiB PSP mode. The user confirmed 1.6-r2 on physical PSP; hardware confirmation of 1.8 remains pending.

## Publishing

Every future release must include a source push and a GitHub Release. Updating
`release/current.json` on `main` starts the [publishing workflow](.github/workflows/publish-release.yml).
It validates the exact tested EBOOT and source, packages the downloads, verifies
the uploaded checksums, and publishes the release. See [AGENTS.md](AGENTS.md) for
the standing release requirements.

The older NO MUSIC releases remain available in release history.
