# Rift Ascent for PSP

**[Download the latest GitHub release](https://github.com/andrewcirincione/Rift-Ascent/releases/latest)**

Current version: **1.6.2**. This patch unifies the rift world art, improves cards, rewards and inventory, diversifies routes and relics, and adds artwork recovery while retaining the hardware-confirmed startup correction. Releases include the complete EBOOT with its intended
soundtrack, an installable PSP ZIP, source, a combat screenshot and SHA-256 checksums.

Extract the PSP ZIP to the Memory Stick root. For the standalone download, rename
it to `EBOOT.PBP` and place it in `PSP/GAME/RIFTASCENT/`. Preserve `SAVE.DAT` and
`PROFILE.DAT`. Version 1.6 adds Proxy and the redesigned Vesper, and migrates prior runs while preserving cards, items and active combat choices.

## Source and validation

- [Current patch notes](versions/1.6.2/PATCH_NOTES.md)
- [Build instructions and validation details](qa/v162/README.md)
- Canonical source: `qa/v162/source`; runtime copies: `GAME/src` and `portable_build`
- Exact tested binary: `versions/1.6.2/EBOOT.PBP` (release tag `v1.6.2`)
- Tested in PPSSPP 1.20.4 with native sanitizer and gameplay regressions.
  Version 1.6.2 is tested in 32 MiB PSP mode. The user confirmed 1.6-r2 on physical PSP; hardware confirmation of this patch remains pending.

## Publishing

Every future release must include a source push and a GitHub Release. Updating
`release/current.json` on `main` starts the [publishing workflow](.github/workflows/publish-release.yml).
It validates the exact tested EBOOT and source, packages the downloads, verifies
the uploaded checksums, and publishes the release. See [AGENTS.md](AGENTS.md) for
the standing release requirements.

The older NO MUSIC releases remain available in release history.
