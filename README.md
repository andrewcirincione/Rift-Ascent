# Rift Ascent for PSP — 1.8.4 Beta 1

**[Download this beta](https://github.com/andrewcirincione/Rift-Ascent/releases/tag/v1.8.4-beta.1)**

This branch contains the experimental **v1.8.4-beta.1** build based on stable v1.8.4-r4. It adds deliberate elite/boss patterns, late-run forge rewards and a browsable run recap. Stable main remains unchanged.

Install the beta ZIP alongside stable in `PSP/GAME/RIFTASCENT_BETA`. It uses separate `BETA184_*` saves/account/settings and `BETA_PROFILE.DAT`. For the standalone binary, rename it `EBOOT.PBP` and use the same beta folder.

- [Beta patch notes](versions/1.8.4-beta.1/PATCH_NOTES.md)
- [Build and validation](qa/v184beta1/README.md)
- Canonical source: `qa/v184beta1/source`
- Complete tested binary: `versions/1.8.4-beta.1/EBOOT.PBP`
- Branch: `beta/1.8.4-b1`; GitHub prerelease, not latest stable.

## Publishing

The beta workflow validates the exact tested EBOOT, packages source and the PSP install ZIP, verifies uploaded digests, and publishes a prerelease with latest=false. Changes stay on this beta branch until explicitly approved for stable.
