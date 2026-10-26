# Rift Ascent project instructions

The owner requires the source and compiled EBOOT for version 1.3.11 and every
future release to be pushed to this GitHub repository and published as a GitHub
Release. Delivering files only in chat does not complete a release.

- Keep the requested game version. Do not increment it without instruction.
- Implement the changes, run appropriate native checks, and playtest the exact
  packaged EBOOT in PPSSPP. Inspect screenshots when visuals change.
- Aesthetic redesigns require generated artwork. Do not substitute arbitrary
  decorative lines. Integrate and inspect the art in the actual game.
- Keep source, generated assets, build scripts and validation evidence together.
  The current canonical source is identified by `release/current.json`.
- Put the exact tested complete EBOOT at `versions/<version>/EBOOT.PBP`, with
  patch notes, a combat screenshot and SHA-256. Include the intended soundtrack;
  do not silently publish an engine-only or NO MUSIC substitute.
- Update `release/current.json`, commit the corresponding source and push to
  `main`. The Publish PSP release workflow validates and publishes the assets.
- Use a new release tag for a changed, already-published binary. A correction
  can keep the in-game version and use a tag such as `v1.3.11-r2`. Never move an
  existing published tag or silently overwrite its assets.
- Wait for publishing to succeed. Verify that the public EBOOT asset digest
  matches the tested local binary and provide the GitHub release link.
- Preserve existing history and user work. Do not force-push.

The owner's October 26, 2026 instruction authorizes publishing the current
complete EBOOT and future requested releases. The previous engine-only public
release convention is superseded by this instruction.
