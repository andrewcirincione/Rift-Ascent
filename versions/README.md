# Versioned Rift Ascent EBOOTs

Every explicitly named Rift Ascent version has its own `versions/<version>/` directory containing `PATCH_NOTES.md`, `EBOOT.PBP` and `SHA256.txt` after successful compilation. Large optional media assets may be distributed in the version ZIP instead of embedded in EBOOT.

- [1.0.0 baseline](1.0.0/PATCH_NOTES.md) — source commit and CI artifact from October 23, 2026.
- [1.0.1 candidate](1.0.1/PATCH_NOTES.md) — user feedback, tracked implementation, regression checks and remaining work.
- [1.0.2 candidate](1.0.2/PATCH_NOTES.md) — save/quit, clearer battle/deck/shop UI, previews, route inspection, CHURN battle music.

The released binaries are checked into the repository by the isolated successful GitHub Actions publishing job. Always preserve an independent copy of the last verified physical-PSP EBOOT.
