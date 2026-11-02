# Rift Ascent distribution branch instructions

The owner requested an independent, free distribution edition on a separate
GitHub branch. All work on this branch targets `distribution`; its workflow must
never update `main`. Preserve history and existing releases. Do not force-push.

- Keep the in-game version 1.8.6 unless the owner requests another version.
- The canonical source is `qa/v186/source`. The distribution catalog replaces
  the development edition's card/relic rules; never restore the old catalogs.
- The owner confirms artwork and music are distinct. Preserve these assets;
  provenance inspection or regeneration is outside this task's scope.
- Compile with the pinned PSPDEV toolchain, run native functional/balance tests,
  and playtest the exact complete EBOOT in PPSSPP. Inspect changed UI text/layout.
- Preserve five-win secret access, ten-win unlocks, once-only shared finale,
  inventory access and the corrected boss ending transitions.
- Every release includes the complete soundtrack, source, validation, notes,
  a combat screenshot, and SHA-256. Verify public download bytes after publication.
- Use a fresh immutable distribution release tag for a changed published binary.
  Distribution releases do not replace the development edition as latest stable.
- Only the allowlisted current distribution source and resources belong in its
  release archives. Exclude legacy executables, reference catalogs and QA archives.
- Card/name/stat changes are engineering differentiation, not legal clearance.
  Never promise immunity from claims or infer that free distribution is exempt.
